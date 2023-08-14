#include "ANA_Pins.h"
#include "ANA_Clock.h"
#include "ANA_Tasks.h"
#include <time.h>
#include <sys/time.h>

#define GPSTIMESET 1 // how frequently synchronized with GPS PPS

long GPS_PulseCount = 0;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED; //http://marchan.e5.valueserver.jp/cabin/comp/jbox/arc202/doc21105.html
// see interrupt processing for ESP32 at https://techtutorialsx.com/2017/09/30/esp32-arduino-external-interrupts/
#define GPS_BAUD 9600 //9600bps NEO6M by default


TinyGPSPlus gps;
uint32_t clock_pps_timestamp;
uint32_t clock_seconds;
uint8_t gps_satellite_count;

volatile uint32_t clock_millis;
uint32_t stats_tick_duration;

TaskHandle_t gps_task_handle;

uint32_t next_event_ms = 12000;

unsigned long gpsEpochTime = 0; //in sec UNIX epoch time , second is counted up since 00:00:00 Jan 1 1970
unsigned long gpsEpochTimeMillis = 0; //in msec millis() when NMEA sentence gives UTC
unsigned long gpsPulseTimeMillis = 0; // millis() when 1PPS from GPS rises
unsigned long gpsOffsetMillis = 0; // msec between 1PPS pulse (rising edge ) to when NMEA sentence gives UTC
long gpsLasttime = 0; long gpsLoop = 0;
unsigned long gpsPulseTimeMicros = 0;
unsigned long gpsPulseTimeMicrosLast = 0;
// interrupt service for 1PPS from NEO6M GPS
// https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/

//----------------------------------------------------------------------------------------
void IRAM_ATTR gps_pulse_interrupt(void)
{
    clock_pps_timestamp = millis();
}
//----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
void gps_task(void *p)
{

    pinMode(PIN_GPS_1PPS, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_GPS_1PPS), gps_pulse_interrupt, RISING);


    Serial2.begin(9600, SERIAL_8N1,
                  PIN_GPS_TX,
                  PIN_GPS_TX);

    while (1)
    {
        while (Serial2.available() > 0)
        {
            gps.encode(Serial2.read());
        }

        if (gps_satellite_count < gps.satellites.value())
        {
            gps_satellite_count = gps.satellites.value();
            log_v("Found a satellite, now there are %d", gps_satellite_count);
        }
        else if (gps_satellite_count > gps.satellites.value())
        {
            gps_satellite_count = gps.satellites.value();
            log_v("Lost a satellite, now there are %d", gps_satellite_count);
        }
        vTaskDelay(GPS_TASK_DELAY / portTICK_RATE_MS);
    }
}

//----------------------------------------------------------------------------------------

bool clock_init(void)
{

 
    xTaskCreatePinnedToCore(
        gps_task,          /* Function to implement the task */
        "GPS Task",        /* Name of the task */
        10000,             /* Stack size in words */
        NULL,              /* Task input parameter */
        GPS_TASK_PRIORITY, /* Priority of the task */
        &gps_task_handle,  /* Task handle. */
        GPS_TASK_CORE);    /* Core where the task should run */

    /*  ms_timer = timerBegin(0, 80, true);
     timerAttachInterrupt(ms_timer, ms_timer_isr, true);
     timerAlarmWrite(ms_timer, 1000, true);
     timerAlarmEnable(ms_timer); */

    return 1;
}

void stopClock(void)
{
  // detachInterrupt(digitalPinToInterrupt(PIN_GPS_1PPS));
}
