#include <Arduino.h>
#include <driver/i2c_master.h>
#include <driver/i2s_std.h>

#include "clogger/clogger.h"
#include "es8311.h"

 
#ifndef ARDUINO_ESP32S3_DEV
#error "This example only supports ESP32S3-Dev board."
#endif

#ifndef CONFIG_SPIRAM_MODE_OCT
#error "This example requires PSRAM to OPI PSRAM. Please enable it in Arduino IDE."
#endif

namespace {

constexpr uint32_t kRxTypeEs8311SampleRate = 16000;
constexpr uint32_t kRxTypeEs8311MclkMultiple = 384;  // If not using 24-bit data width, 256 should be enough
constexpr uint32_t kRxTypeEs8311MclkFreqHz = kRxTypeEs8311SampleRate * kRxTypeEs8311MclkMultiple;

constexpr auto kRxTypeEs8311I2sMclk = GPIO_NUM_11;
constexpr auto kRxTypeEs8311I2sSclk = GPIO_NUM_10;
constexpr auto kRxTypeEs8311I2sWs = GPIO_NUM_8;
constexpr auto kRxTypeEs8311I2sDsdin = GPIO_NUM_7;
constexpr auto kRxTypeEs8311I2sAsdout = GPIO_NUM_9;
constexpr auto kRxTypeEs8311I2cScl = GPIO_NUM_12;
constexpr auto kRxTypeEs8311I2cSda = GPIO_NUM_13;

i2s_chan_handle_t g_rx_handle = nullptr;
i2s_chan_handle_t g_tx_handle = nullptr;

void InitEs8311I2s() {
  CLOGI();

  i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_1, I2S_ROLE_MASTER);
  chan_cfg.auto_clear = true;  // Auto clear the legacy data in the DMA buffer
  ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &g_tx_handle, &g_rx_handle));
  // ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &g_tx_handle, nullptr));
  i2s_std_config_t std_cfg = {
      .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(kRxTypeEs8311SampleRate),
      .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
      .gpio_cfg =
          {
              .mclk = kRxTypeEs8311I2sMclk,
              .bclk = kRxTypeEs8311I2sSclk,
              .ws = kRxTypeEs8311I2sWs,
              .dout = kRxTypeEs8311I2sDsdin,
              .din = kRxTypeEs8311I2sAsdout,
              // .din = GPIO_NUM_NC,
              .invert_flags =
                  {
                      .mclk_inv = 0,
                      .bclk_inv = 0,
                      .ws_inv = 0,
                  },
          },
  };

  std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT;
  std_cfg.clk_cfg.mclk_multiple = static_cast<i2s_mclk_multiple_t>(kRxTypeEs8311MclkMultiple);

  ESP_ERROR_CHECK(i2s_channel_init_std_mode(g_tx_handle, &std_cfg));
  ESP_ERROR_CHECK(i2s_channel_enable(g_tx_handle));

  ESP_ERROR_CHECK(i2s_channel_init_std_mode(g_rx_handle, &std_cfg));
  ESP_ERROR_CHECK(i2s_channel_enable(g_rx_handle));
}

void InitEs8311() {
  CLOGI();
  const i2c_master_bus_config_t i2c_master_bus_config = {
      .i2c_port = I2C_NUM_1,
      .sda_io_num = kRxTypeEs8311I2cSda,
      .scl_io_num = kRxTypeEs8311I2cScl,
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .glitch_ignore_cnt = 7,
      .intr_priority = 0,
      .trans_queue_depth = 0,
      .flags =
          {
              .enable_internal_pullup = 1,
              .allow_pd = 0,
          },
  };

  i2c_master_bus_handle_t i2c_master_bus = nullptr;
  ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_master_bus_config, &i2c_master_bus));
  es8311_handle_t es_handle = es8311_create(i2c_master_bus, ES8311_ADDRRES_0);
  const es8311_clock_config_t es_clk = {.mclk_inverted = false,
                                        .sclk_inverted = false,
                                        .mclk_from_mclk_pin = true,
                                        .mclk_frequency = kRxTypeEs8311MclkFreqHz,
                                        .sample_frequency = kRxTypeEs8311SampleRate};
  ESP_ERROR_CHECK(es8311_init(es_handle, &es_clk, ES8311_RESOLUTION_16, ES8311_RESOLUTION_16));
  ESP_ERROR_CHECK(es8311_sample_frequency_config(es_handle, kRxTypeEs8311SampleRate * kRxTypeEs8311MclkMultiple, kRxTypeEs8311SampleRate));
  ESP_ERROR_CHECK(es8311_voice_volume_set(es_handle, 70, nullptr));
  ESP_ERROR_CHECK(es8311_microphone_config(es_handle, false));
  ESP_ERROR_CHECK(es8311_microphone_gain_set(es_handle, ES8311_MIC_GAIN_12DB));
}

}  // namespace

void setup() {
  CLOGI();
  InitEs8311I2s();
  InitEs8311();

}

void loop() {
  int16_t buffer[16000 / 1000 * 20];
  i2s_channel_read(g_rx_handle, buffer, sizeof(buffer), nullptr, UINT32_MAX);
  i2s_channel_write(g_tx_handle, buffer, sizeof(buffer), nullptr, UINT32_MAX);
}