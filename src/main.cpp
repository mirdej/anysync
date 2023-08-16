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
#include <ArduinoJson.h>

ESPLogger logger("/log.txt", SD);
SyncFile sync_file;
String hostname;
bool clock_was_set = false;
void main_task(void *p);

void parse_config()
{
  File f = SD.open("/config.json", FILE_READ);
  if (!f)
  {
    display_messages.push("FAIL CONFIG");

    log_e("Failed to open config file");
    return;
  }

  DynamicJsonDocument doc(2048);
  log_i("READ PASSWORDS");
  DeserializationError error = deserializeJson(doc, f);
  if (error)
  {
    log_e("Failed to read file");
    f.close();

    return;
  }
  f.close();

  const char *temp = doc["name"];
  hostname = temp;
  midi_channel = doc["midi_channel"];
  midi_channel--;
  midi_channel %= 16;
  log_v("Set MIDI channel to %d", midi_channel);
  /*   JsonArray array = doc["passwords"].as<JsonArray>();
    for (JsonVariant v : array)
    {
        const char *ssid = v["ssid"];
        const char *pass = v["pass"];
        log_v("---->WIFI: %s, %s", ssid, pass);
        wifiMulti.addAP(ssid, pass);
    } */

  /*  const char *temp1 = doc["start"];
   String s = temp1;
   log_v("found Show start to be %s ", s); */
  int n = doc["start"]; // s.toInt();

  struct tm tm; // check epoch time at https://www.epochconverter.com/
  tm.tm_year = rtc.getYear() - 1900;
  tm.tm_mon = rtc.getMonth();
  tm.tm_mday = rtc.getDay();
  /*   tm.tm_hour = n / 100;
    tm.tm_min = n % 100; */
  tm.tm_hour = rtc.getHour(true);
  tm.tm_min = rtc.getMinute() + 1;
  tm.tm_sec = 0;
  tm.tm_isdst = -1; // disable summer time
  time_t t = mktime(&tm);

  show_start = t;
  log_v("Set Show start to %d from n=%d", show_start, n);

  display_messages.push("READ");

  /*  S
    log_v("------PARSE SHOW-------");

   f.open("/show.json", O_READ);
   if (!f)
   {
     display_messages.push("FAIL CONFIG FILE");

     log_e("Failed to open config file");
     return;
   }
   DynamicJsonDocument doc(2048);
   log_i("READ");
   DeserializationError error = deserializeJson(doc, f);
   if (error)
   {
     log_e("Failed to read file");
     f.close();

     return;
   }
   f.close();
   log_v("Deserialized");
   const char *temp = doc["name"];
   show_name = temp;

   gmt_offset = doc["gmt_offset"];
   show_start = doc["start"];
   show_start += gmt_offset;
   show_end = show_start + SYF.getLength() / 1000 + 1;
   // rtc.offset = gmt_offset;

   log_v("GMT-offset %d", gmt_offset);
   JsonArray array = doc["devices"].as<JsonArray>();
   for (JsonVariant v : array)
   {
     const char *macadd = v["mac"];
     log_v("---->MAC: %s, %s", macadd, WiFi.macAddress().c_str());
     if (!strcmp(macadd, WiFi.macAddress().c_str()))
     {
       const char *temp2 = v["name"];
       hostname = temp2;
     }
   }
   display_messages.push("READ"); */
}

//========================================================================================
//----------------------------------------------------------------------------------------
//                                        SETUP

void setup()
{
  makeversion(__DATE__, __TIME__, version);
  pinMode(PIN_BTN_1, OUTPUT);
  Serial.begin(115200);

  delay(1000); // prevent upload errors if program crashes esp
  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI);
  SPI.setFrequency(1000000);
  sd_card_present = SD.begin(PIN_SD_CS);

  init_display();
  delay(2000); // prevent upload errors if program crashes esp
  Serial.begin(115200);
  logger.begin();

  audioInit();
  audioSetVolume(15);

  clock_init();
  ui_begin();
  sync_file.begin();

  xTaskCreatePinnedToCore(
      main_task,                 /* Function to implement the task */
      "MAIN Task",               /* Name of the task */
      SYNC_FILE_TASK_STACK_SIZE, /* Stack size in words */
      NULL,                      /* Task input parameter */
      SYNC_FILE_TASK_PRIORITY,   /* Priority of the task */
      &display_task_handle,      /* Task handle. */
      SYNC_FILE_TASK_CORE);      /* Core where the task should run */
                                 // print_task_stats();
}

//========================================================================================
//----------------------------------------------------------------------------------------
//                                        LOOP

void loop()
{
}

void main_task(void *p)
{
  while (1)
  {
    digitalWrite(PIN_BTN_1, HIGH);
    gps_task(NULL);
    digitalWrite(PIN_BTN_1, LOW);

    if (clock_is_set)
    {
      if (!clock_was_set)
      {
        parse_config();
        clock_was_set = true;
        vTaskDelay(10 / portTICK_PERIOD_MS);
      }

      if (!sync_file._isEOF)
      {
        sync_file_task(NULL);
      }
      else
      {
        uint32_t now = rtc.getEpoch();

        if (now > show_start)
        {
          sync_file.start();
        }
      }
    }
  }
}
