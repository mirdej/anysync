#include "ANA_Pins.h"
#include "ANA_Clock.h"
#include "ANA_Tasks.h"
#include <time.h>
#include <sys/time.h>

ESP32Time rtc(0); // offset in seconds GMT+1

#define GPSTIMESET 1 // how frequently synchronized with GPS PPS

long GPS_PulseCount = 0;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED; // http://marchan.e5.valueserver.jp/cabin/comp/jbox/arc202/doc21105.html
// see interrupt processing for ESP32 at https://techtutorialsx.com/2017/09/30/esp32-arduino-external-interrupts/
#define GPS_BAUD 9600 // 9600bps NEO6M by default

TinyGPSPlus gps;
uint32_t clock_pps_timestamp;
uint32_t clock_seconds;
uint8_t gps_satellite_count;

volatile uint32_t clock_millis;
uint32_t stats_tick_duration;

TaskHandle_t gps_task_handle;

uint32_t next_event_ms = 12000;

unsigned long gpsEpochTime = 0;       // in sec UNIX epoch time , second is counted up since 00:00:00 Jan 1 1970
unsigned long gpsEpochTimeMillis = 0; // in msec millis() when NMEA sentence gives UTC
unsigned long gpsPulseTimeMillis = 0; // millis() when 1PPS from GPS rises
unsigned long gpsOffsetMillis = 0;    // msec between 1PPS pulse (rising edge ) to when NMEA sentence gives UTC
long gpsLasttime = 0;
long gpsLoop = 0;
unsigned long gpsPulseTimeMicros = 0;
unsigned long gpsPulseTimeMicrosLast = 0;
// interrupt service for 1PPS from NEO6M GPS
// https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/


//----------------------------------------------------------------------------------------
void IRAM_ATTR gps_pulse_interrupt(void)
{
    portENTER_CRITICAL_ISR(&mux);
    gpsPulseTimeMicros = micros();
    gpsPulseTimeMillis = millis();
    GPS_PulseCount++;
    portEXIT_CRITICAL_ISR(&mux);
}
//----------------------------------------------------------------------------------------

String int2digit3(uint32_t msec)
{
    if (msec >= 100)
        return (String(msec));
    if (msec >= 10)
        return ("0" + String(msec));
    return ("00" + String(msec));
}

// UTC by TinyGPS++ points to the rising edge of PPS according to NEO6M reference manual
int i = 0;
void getGPSInfo()
{
    int Year = gps.date.year();
    if (!(i % GPSTIMESET))
    {
        if (gps.location.isValid() && gps.time.isValid() && (Year > 2017))
        {

            byte Month = gps.date.month();
            byte Day = gps.date.day();
            byte Hour = gps.time.hour();
            byte Minute = gps.time.minute();
            byte Second = gps.time.second();

            struct tm tm; // check epoch time at https://www.epochconverter.com/
            tm.tm_year = Year - 1900;
            tm.tm_mon = Month - 1;
            tm.tm_mday = Day;
            tm.tm_hour = Hour;
            tm.tm_min = Minute;
            tm.tm_sec = Second;
            tm.tm_isdst = -1; // disable summer time
            time_t t = mktime(&tm);
            gpsEpochTime = t;              // in  sec
            gpsEpochTimeMillis = millis(); // in msec
            gpsOffsetMillis = gpsEpochTimeMillis - gpsPulseTimeMillis;
            struct timeval now = {.tv_sec = t};
            settimeofday(&now, NULL); // set rtc
            struct tm *ptm;
            t = time(NULL);
            ptm = localtime(&t);

            clock_seconds = gpsEpochTime + gpsOffsetMillis / 1000;
        }
        else
        {
            Serial.println(F("GPS INVALID, wait for a second"));
        }
    }
    i++;
}
//----------------------------------------------------------------------------------------
void gps_task(void *p)
{
    uint32_t t_millis;

    pinMode(PIN_GPS_1PPS, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_GPS_1PPS), gps_pulse_interrupt, RISING);

    Serial2.begin(GPS_BAUD, SERIAL_8N1,
                  PIN_GPS_TX,
                  PIN_GPS_TX);

    while (1)
    {
        if (GPS_PulseCount > 0)
        {
            //    gpsPulseTimeMicros = micros();
            //    gpsPulseTimeMillis = millis();
            // If ISR has been serviced at least once
            portENTER_CRITICAL(&mux);
            GPS_PulseCount--;
            portEXIT_CRITICAL(&mux);
            gpsPulseTimeMicrosLast = gpsPulseTimeMicros;
        }

        if (Serial2.available() > 0)
        {
            if (gps.encode(Serial2.read()))
            { // some NMEA sentense is parsed
                t_millis = millis();
                if (t_millis < (0xffffffff - 1000))
                { // avoid millis roll over
                    if ((t_millis - gpsPulseTimeMillis) < 500)
                    { // NMEA parsed is immediately done after PPS
                        // Serial.print(t_millis); Serial.print(" " ); Serial.print(gpsLasttime); Serial.print(":"); Serial.println( t_millis -gpsLasttime);
                        if (t_millis - gpsLasttime > 900)
                        { // but discard other consective NMEA sentenses after first NMEA after PPS
                            gpsLoop++;
                            //  digitalWrite(GPIO_NUM_4, gpsLoop % 2);
                            getGPSInfo();
                            gpsLasttime = millis();
                        }
                        else
                        {
                            //  Serial.println("discard this NMEA");
                        }
                    }
                }
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

    return 1;
}

