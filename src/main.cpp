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

ESPLogger logger("/log.txt", SD);

//****************************************************************************************
//                                   S E T U P                                           *
//****************************************************************************************

void setup()
{
  Serial.begin(115200);

  delay(5000); // prevent upload errors if program crashes esp

  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI);
  SPI.setFrequency(1000000);
  Serial.begin(115200);
  SD.begin(PIN_SD_CS);
  logger.begin();

  audioInit();
  audioSetVolume(15);

  clock_init();
  ui_begin();
}

//****************************************************************************************
//                                   L O O P                                             *
//****************************************************************************************

void loop()
{
  static long last;
  if (millis() - last > 3000)
  {
    last = millis();

    print_satellites();
    print_stats();

    // audioConnecttoSD("/samples/007.wav");
  }
}
