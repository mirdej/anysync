#pragma once
#include "Arduino.h"
#include "WiFi.h"
#include "ANA_Pins.h"
#include "ANA_Audio.h"
#include "ANA_Clock.h"
#include "ANA_UI.h"
#include "ANA_Print.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"
#include <ESPLogger.h>


void displayInfo()
{
  if (gps.location.isValid())
    log_v("Location: %f, %f", gps.location.lat(), gps.location.lng());
  else
    log_v("Location INVALID");

  if (gps.date.isValid())
    log_v("Date: %02d.%02d.%04d", gps.date.day(), gps.date.month(), gps.date.year());
  else
    log_v("Date INVALID");

  if (gps.time.isValid())
    log_v("Time: %02d:%02d:%02d.%02d", gps.time.hour(), gps.time.minute(), gps.time.second(), gps.time.centisecond());
  else
    log_v("Time INVALID");
}

void print_time()
{
  if (gps.time.isValid())
    log_v("Time: %02d:%02d:%02d.%02d Age %d", gps.time.hour(), gps.time.minute(), gps.time.second(), gps.time.centisecond(), gps.time.age());
  else
    log_v("Time INVALID");
}
void print_satellites()
{
  log_v("SATELLITES Fix Age=%d, ms Value=%d", gps.satellites.age(), gps.satellites.value());
}

void print_stats()
{
  log_v("DIAGNOSTICS:      Chars=%d, Sentences-with-Fix=%d, Failed-checksum=%d, Passed-checksum=%d", gps.charsProcessed(), gps.sentencesWithFix(), gps.failedChecksum(), gps.passedChecksum());
  if (gps.charsProcessed() < 10)
    log_v("WARNING: No GPS data.  Check wiring.");
}

void print_task_stats()
{
  log_v("UI Task %d", uxTaskGetStackHighWaterMark(ui_task_handle));
  log_v("AUDIO Task %d", uxTaskGetStackHighWaterMark(audio_task_handle));
}
