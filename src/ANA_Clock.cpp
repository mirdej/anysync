#include "ANA_Pins.h"
#include "ANA_Clock.h"

TinyGPSPlus gps;
uint32_t clock_pps_timestamp;
uint32_t clock_seconds;
uint8_t gps_satellite_count;

volatile uint32_t clock_millis;
uint32_t stats_tick_duration;

TaskHandle_t tick_task_handle;

uint32_t next_event_ms = 12000;
hw_timer_t *ms_timer = NULL;

//----------------------------------------------------------------------------------------
void IRAM_ATTR gps_pulse_interrupt(void)
{
    clock_pps_timestamp = millis();
    clock_seconds = 3600 * gps.time.hour() + 60 * gps.time.minute() + gps.time.second();
    clock_millis = clock_seconds * 1000 + gps.time.age();
}
//----------------------------------------------------------------------------------------

static void IRAM_ATTR ms_timer_isr()
{
    xTaskResumeFromISR(tick_task_handle);
}

//----------------------------------------------------------------------------------------
void gps_task(void *p)
{
    Serial2.begin(9600, SERIAL_8N1,
                  PIN_RX1,
                  PIN_TX1);

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

            vTaskDelay(50 / portTICK_RATE_MS);
        }
    }
}

//----------------------------------------------------------------------------------------
void tick_task(void *p)
{
    static long start_micros;
            log_v("Alarm Task Started");

    while (1)
    {
        vTaskSuspend(NULL);
        // start_micros = micros();
        clock_millis++;
        if (clock_millis >= next_event_ms)
        {
            log_v("Alarm triggered");
            next_event_ms = -1;
        }
    }
}

//----------------------------------------------------------------------------------------

bool clock_init(void)
{
    int8_t irq = digitalPinToInterrupt(PIN_1PPS);

    if (irq != NOT_AN_INTERRUPT)
    {
        pinMode(PIN_1PPS, INPUT);
        attachInterrupt(irq, gps_pulse_interrupt, CHANGE);
    }

    xTaskCreatePinnedToCore(
        gps_task,   /* Function to implement the task */
        "GPS Task", /* Name of the task */
        10000,      /* Stack size in words */
        NULL,       /* Task input parameter */
        0,          /* Priority of the task */
        NULL,       /* Task handle. */
        0);         /* Core where the task should run */

    xTaskCreatePinnedToCore(
        tick_task,         /* Function to implement the task */
        "Tick Task",       /* Name of the task */
        10000,             /* Stack size in words */
        NULL,              /* Task input parameter */
        0,                 /* Priority of the task */
        &tick_task_handle, /* Task handle. */
        0);                /* Core where the task should run */

    ms_timer = timerBegin(0, 80, true);
    timerAttachInterrupt(ms_timer, ms_timer_isr, true);
    timerAlarmWrite(ms_timer, 1000, true);
    timerAlarmEnable(ms_timer);

    return (irq != NOT_AN_INTERRUPT);
}



void stopClock(void)
{
    detachInterrupt(digitalPinToInterrupt(PIN_1PPS));
}