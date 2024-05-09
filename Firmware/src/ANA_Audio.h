#pragma once

#include "Arduino.h"
#include "AudioFileSourceSD.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2SNoDAC.h"
#include "WiFi.h"

#include "SPI.h"
#include "SD.h"
#include "FS.h"
#include "ANA_Pins.h"

extern  int sample_to_play;

struct audioMessage
{
  uint8_t cmd;
  const char *txt;
  uint32_t value;
  uint32_t ret;
};

enum : uint8_t
{
  SET_VOLUME,
  GET_VOLUME,
  CONNECTTOHOST,
  CONNECTTOSD
};



void audioTask(void *parameter);

void audioInit();