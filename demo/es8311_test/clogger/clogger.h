#pragma once

#ifndef __CLOG_H__
#define __CLOG_H__

#ifdef ARDUINO_ARCH_ESP32
#include "clogger_esp32.h"
#else
#include "clogger_common.h"
#endif

#endif