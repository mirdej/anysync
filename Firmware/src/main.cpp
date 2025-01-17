/*========================================================================================
                                        ANYSYNC
                             GPS Synchronized MIDI/Sample Player

                                    © 2023 Michael Egger


----------------------------------------------------------------------------------------*/

#include "Arduino.h"
#include <ESP32Time.h>

#include "WiFi.h"
#include <WiFiMulti.h>
#include "ANA_Pins.h"
#include "ANA_Audio.h"
#include "ANA_Clock.h"
#include "ANA_Display.h"
#include "ANA_UI.h"
#include "ANA_Print.h"
#include "ANA_MIDIFile.h"
#include "ANA_Syncfile.h"
#include "ANA_Wifi.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"
#include <ESPLogger.h>
#include "ANA_Utils.h"
#include <ArduinoJson.h>

ESPLogger logger("/log.txt", SD);
SyncFile sync_file;

WiFiMulti wifiMulti;
uint32_t device_delay = 0;
ClockStatus clock_status = unset;
ClockStatus last_clock_status = unset;
TaskHandle_t wifi_task_handle;

void main_task(void *p);

void set_show_start(uint32_t t)
{
  sync_file.rewind();
  show_start = t;

  show_end = show_start + sync_file.getLength() / 1000 + 10;
  log_v("Set Show start to %d end to n=%d", show_start, show_end);
}

void parse_config()
{
  sync_file.begin(); // need to be open before setting show_end

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
  /*
    const char *temp = doc["name"];
    hostname = temp;
    device_delay = doc["delay"];
    device_delay %= 100000; */

  if (doc.containsKey("server"))
  {
    const char *server = doc["server"];
    command_server = server;
  }

  JsonArray array = doc["passwords"].as<JsonArray>();
  for (JsonVariant v : array)
  {
    const char *ssid = v["ssid"];
    const char *pass = v["pass"];
    log_v("---->WIFI: %s, %s", ssid, pass);
    wifiMulti.addAP(ssid, pass);
  }

  /* int n = doc["start"];

  show_start_hour = n / 100;
  show_start_minute = n % 100;
  struct tm tm; // check epoch time at https://www.epochconverter.com/
  tm.tm_year = rtc.getYear() - 1900;
  tm.tm_mon = rtc.getMonth();
  tm.tm_mday = rtc.getDay();
  tm.tm_hour = show_start_hour;
  tm.tm_min = show_start_minute;
  tm.tm_sec = 0;
  tm.tm_isdst = -1; // disable summer time
  time_t t = mktime(&tm);

  if (t < rtc.getEpoch())
  {

    /*    struct tm tm; // check epoch time at https://www.epochconverter.com/
       tm.tm_year = rtc.getYear() - 1900;
       tm.tm_mon = rtc.getMonth();
       tm.tm_mday = rtc.getDay();
       tm.tm_hour = rtc.getHour(true);
       tm.tm_min = rtc.getMinute() + 2;
       tm.tm_sec = 0;
       tm.tm_isdst = -1; // disable summer time
       time_t t = mktime(&tm); */
  /*   t = 0;
   }

   set_show_start(t); */

  display_messages.push("READ");
}

void debug_task(void *p)
{
  while (1)
  {
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    log_v("Heap: %d", ESP.getFreeHeap());
  }
}

//========================================================================================
//----------------------------------------------------------------------------------------
//                                        SETUP

void setup()
{
  WiFi.mode(WIFI_OFF);

  makeversion(__DATE__, __TIME__, version);
  pinMode(PIN_BTN_1, OUTPUT);
  pinMode(PIN_BTN_2, OUTPUT);
  Serial.begin(115200);

  delay(1000); // prevent upload errors if program crashes esp
  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI);
  SPI.setFrequency(1000000);
  pinMode(PIN_MISO,INPUT_PULLUP);
  sd_card_present = SD.begin(PIN_SD_CS);

  init_display();
  delay(2000); // prevent upload errors if program crashes esp

  // logger.begin();

  parse_config();
  parse_show_file();

  xTaskCreate(check_wifi_task, "check wifi", WIFI_TASK_STACK_SIZE, NULL, WIFI_TASK_PRIORITY, &wifi_task_handle);
  // xTaskCreate(debug_task, "check heap", 4000, NULL, 0, NULL);

  audioInit();

  clock_init();
  ui_begin();

  xTaskCreatePinnedToCore(
      main_task,                 /* Function to implement the task */
      "MAIN Task",               /* Name of the task */
      SYNC_FILE_TASK_STACK_SIZE, /* Stack size in words */
      NULL,                      /* Task input parameter */
      SYNC_FILE_TASK_PRIORITY,   /* Priority of the task */
      &sync_file_task_handle,    /* Task handle. */
      SYNC_FILE_TASK_CORE);      /* Core where the task should run */
                                 // print_task_stats();
}

//========================================================================================
//----------------------------------------------------------------------------------------
//                                        LOOP

void loop()
{
  static long last;
  if (millis() - last > 4000)
  {
    //    log_v("%04d %02d:%02d;%02d", gps.date.year(), gps.time.hour(), gps.time.minute(), gps.time.second());
    last = millis();
    if (clock_status == unset)
    {
      if (gps.date.year() > 2017)
      {
        // apparently GPS RTC is running, set clock temporarily with >1s precision
        clock_status = gps_rtc;
        set_clock_from_gps();
      }
    }
  }
}

void main_task(void *p)
{
  while (1)
  {
    gps_task(NULL);

    if (clock_status != unset)
    {
      if (last_clock_status == unset)
      {
        parse_config();
        parse_show_file();
        last_clock_status = clock_status;
        vTaskDelay(10 / portTICK_PERIOD_MS);
      }

      if (!sync_file._isEOF)
      {
        sync_file_task(NULL);
      }
      else
      {
        uint32_t now = rtc.getEpoch();
        if (now < show_end)
        {
          if (now > show_start)
          {
            if (show_start)
            {
              sync_file.start(device_delay);
            }
          }
        }
      }
    }
  }
}
