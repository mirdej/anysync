#include "Arduino.h"
#include "WiFi.h"
#include "ANA_Pins.h"
#include "ANA_Audio.h"
#include "ANA_Clock.h"
#include "ANA_UI.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"
#include <ESPLogger.h>

ESPLogger logger("/log.txt", SD);

uint32_t millis_start;
uint32_t clock_millis_start;

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

void display_time()
{
  if (gps.time.isValid())
    log_v("Time: %02d:%02d:%02d.%02d Age %d", gps.time.hour(), gps.time.minute(), gps.time.second(), gps.time.centisecond(), gps.time.age());
  else
    log_v("Time INVALID");
}
void display_satellites()
{
  log_v("SATELLITES Fix Age=%d, ms Value=%d", gps.satellites.age(), gps.satellites.value());
}

void display_stats()
{
  log_v("DIAGNOSTICS:      Chars=%d, Sentences-with-Fix=%d, Failed-checksum=%d, Passed-checksum=%d", gps.charsProcessed(), gps.sentencesWithFix(), gps.failedChecksum(), gps.passedChecksum());
  if (gps.charsProcessed() < 10)
    log_v("WARNING: No GPS data.  Check wiring.");
}

void display_task_stats()
{
  log_v("UI Task %d", uxTaskGetStackHighWaterMark(ui_task_handle));
  log_v("AUDIO Task %d", uxTaskGetStackHighWaterMark(audio_task_handle));
}

//****************************************************************************************
//                                   S E T U P                                           *
//****************************************************************************************

void setup()
{
  Serial.begin(115200);
  /*  WiFi.begin(ssid.c_str(), password.c_str());
   while (WiFi.status() != WL_CONNECTED)
     delay(1500);
  */
  delay(5000);    // prevent upload errors if program crashes esp

  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI);
  SPI.setFrequency(1000000);
  Serial.begin(115200);
  SD.begin(PIN_SD_CS);
  logger.begin();

  audioInit();
  audioSetVolume(15);
  //  log_i("current volume is: %d", audioGetVolume());

  clock_init();
  ui_begin();

  millis_start = millis();
  clock_millis_start = 0; // get_clock_millis();
}

// Variable to be logged
static int eventCounter = 0;

void event()
{
  eventCounter++;

  Serial.printf("Hey, event ->%d<- is just happened\n", eventCounter);
  char record[15];
  snprintf(record, 15, "value: %d", eventCounter);
  // the second parameter allows to prepend to the record the current timestamp
  bool success = logger.append(record, true);
  if (success)
  {
    Serial.println("Record stored!");
  }
  else
  {
    if (logger.isFull())
    {
      Serial.println("Record NOT stored! You had filled the available space: flush or reset the "
                     "log before appending another record");
    }
    else
    {
      Serial.println("Something goes wrong, record NOT stored!");
    }
  }
}

//****************************************************************************************
//                                   L O O P                                             *
//****************************************************************************************

void loop()
{
  static long last;
  if (millis() - last > 3000)
  {
    //  uint32_t start = micros();
    s32_t elapsed_millis = millis() - millis_start;
    // s32_t elapsed_c_millis = get_clock_millis() - clock_millis_start;
    last = millis();
    // log_v("%02d:%02d:%02d %d %d Diff %d Age %d ", gps.time.hour(), gps.time.minute(), gps.time.second(), elapsed_millis, elapsed_c_millis, elapsed_c_millis - elapsed_millis, gps.time.age());

    // display_task_stats();

    // log_v("%d",start-micros());
    display_satellites();
    display_stats();
    // display_time();
    log_v("Timestamp %d", clock_pps_timestamp);
    //     log_v("millis: %d, stellites %d", millis(), gps.satellites.value());
    /*    audioConnecttoSD("/samples/007.wav");
        event(); */
  }
}
