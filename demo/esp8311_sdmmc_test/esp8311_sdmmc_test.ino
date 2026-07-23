#include "Arduino.h"
#include "Audio.h"
#include "WiFi.h"
#include "es8311.h"
#include "Wire.h"

#ifndef ARDUINO_ESP32S3_DEV
#error "This example only supports ESP32S3-Dev board."
#endif

#ifndef CONFIG_SPIRAM_MODE_OCT
#error "This example requires PSRAM to OPI PSRAM. Please enable it in Arduino IDE."
#endif

#define SD_MMC_D0   40
#define SD_MMC_CLK  39
#define SD_MMC_CMD  38

#define I2S_DOUT       7  // DSDIN pin(es8311)
#define I2S_BCLK      10
#define I2S_MCLK      11
#define I2S_LRC       8
#define I2C_SCL       12
#define I2C_SDA       13
#define PA_ENABLE     53

Audio audio;
ES8311 es;

//String ssid =     "*****";
//String password = "*****";


void setup() {
    Serial.begin(115200);
    Serial.print("\n\n");
    Serial.println("----------------------------------");
    Serial.printf("ESP32 Chip: %s\n", ESP.getChipModel());
    Serial.printf("Arduino Version: %d.%d.%d\n", ESP_ARDUINO_VERSION_MAJOR, ESP_ARDUINO_VERSION_MINOR, ESP_ARDUINO_VERSION_PATCH);
    Serial.printf("ESP-IDF Version: %d.%d.%d\n", ESP_IDF_VERSION_MAJOR, ESP_IDF_VERSION_MINOR, ESP_IDF_VERSION_PATCH);
    Serial.printf("ARDUINO_LOOP_STACK_SIZE %d words (32 bit)\n", CONFIG_ARDUINO_LOOP_STACK_SIZE);
    Serial.println("----------------------------------");
    Serial.print("\n\n");

    pinMode(SD_MMC_D0, INPUT_PULLUP);
    SD_MMC.setPins(SD_MMC_CLK,SD_MMC_CMD, SD_MMC_D0);
    if (!SD_MMC.begin("/sdcard", true)) {
       Serial.println("Card Mount Failed");
    }

    // WiFi.begin(ssid.c_str(), password.c_str());
    // while (WiFi.status() != WL_CONNECTED) {delay(1500); Serial.print(".");}

    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT, I2S_MCLK);
    audio.setVolume(21); // default 0...21

    pinMode(PA_ENABLE, OUTPUT);
    digitalWrite(PA_ENABLE, HIGH);

    if(!es.begin(I2C_SDA, I2C_SCL, 400000)) log_e("ES8311 begin failed");
    es.setVolume(50);
    es.setBitsPerSample(16);

    // es.setSampleRate(22050);
    // es.read_all();
    // audio.connecttohost("http://www.wdr.de/wdrlive/media/einslive.m3u");
    // audio.connecttohost("http://stream.antennethueringen.de/live/aac-64/stream.antennethueringen.de/"); // aac
    audio.connecttoFS(SD_MMC, "/test.mp3");

}

void loop() {
    audio.loop();
    vTaskDelay(1);
}

// optional
void audio_info(const char *info){
    Serial.print("info        "); Serial.println(info);
}

