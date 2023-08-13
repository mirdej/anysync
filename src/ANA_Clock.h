#include "Arduino.h"
#include <ESPLogger.h>
#include <TinyGPSPlus.h>

#pragma once

extern ESPLogger logger;

enum : uint8_t
{
    FREE_RUNNING,
    SYNCED
};



extern TinyGPSPlus gps;
extern uint32_t clock_pps_timestamp;
extern uint32_t clock_seconds;
extern uint8_t gps_satellite_count;

extern volatile uint32_t clock_millis;
extern uint32_t stats_tick_duration;

extern TaskHandle_t tick_task_handle;

extern uint32_t next_event_ms;
extern hw_timer_t *ms_timer ;

bool clock_init();