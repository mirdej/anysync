#pragma once

#define LOOP_TASK_CORE 1

// +-----------------------------------------------------------------------+
//                                                      UI Task
extern TaskHandle_t ui_task_handle;
#define UI_TASK_PRIORITY 0
#define UI_TASK_CORE LOOP_TASK_CORE
#define UI_TASK_DELAY   20

// +-----------------------------------------------------------------------+
//                                                      Audio Task
extern TaskHandle_t audio_task_handle;
#define AUDIO_TASK_PRIORITY 2
#define AUDIO_TASK_CORE LOOP_TASK_CORE

// +-----------------------------------------------------------------------+
//                                                      GPS Task
extern TaskHandle_t gps_task_handle;
#define GPS_TASK_PRIORITY 2
#define GPS_TASK_CORE 0
#define GPS_TASK_DELAY   1
