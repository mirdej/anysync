#pragma once

#include "Arduino.h"

#include "WiFi.h"
#include "Audio.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"
#include "ANA_Pins.h"

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


void CreateQueues();

void audioTask(void *parameter);

void audioInit();

audioMessage transmitReceive(audioMessage msg);

void audioSetVolume(uint8_t vol);

uint8_t audioGetVolume();

bool audioConnecttohost(const char *host);

bool audioConnecttoSD(const char *filename);

void audio_info(const char *info);
