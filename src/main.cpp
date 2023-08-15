/*========================================================================================
                                        ANYSYNC
                             GPS Synchronized MIDI/Sample Player

                                    © 2023 Michael Egger


----------------------------------------------------------------------------------------*/

#include "Arduino.h"
#include <ESP32Time.h>

#include "WiFi.h"
#include "ANA_Pins.h"
#include "ANA_Audio.h"
#include "ANA_Clock.h"
#include "ANA_Display.h"
#include "ANA_UI.h"
#include "ANA_Print.h"
#include "ANA_MIDIFile.h"
#include "ANA_Syncfile.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"
#include <ESPLogger.h>
#include "ANA_Utils.h"

ESPLogger logger("/log.txt", SD);
SyncFile sync_file;

//========================================================================================
//----------------------------------------------------------------------------------------
//                                        SETUP

void setup()
{
  makeversion(__DATE__, __TIME__, version);

  Serial.begin(115200);

  delay(1000); // prevent upload errors if program crashes esp
  sd_card_present = SD.begin(PIN_SD_CS);
  init_display();
  delay(2000); // prevent upload errors if program crashes esp

  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI);
  SPI.setFrequency(1000000);
  Serial.begin(115200);
  logger.begin();

  audioInit();
  audioSetVolume(15);

  clock_init();
  ui_begin();
  sync_file.begin();
}

//========================================================================================
//----------------------------------------------------------------------------------------
//                                        LOOP

void loop()
{
  static long last;
  if (millis() - last > 5000)
  {
    /*     last = millis();

        print_satellites();
        print_time();
        log_v("Millis since last pulse %d", millis() - gpsPulseTimeMillis); */
    // print_stats();
  }
  delay(20);
}
