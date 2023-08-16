#pragma once

#define LOOP_TASK_CORE 1

// +-----------------------------------------------------------------------+
//                                                      UI Task
extern TaskHandle_t ui_task_handle;
#define UI_TASK_PRIORITY 0
#define UI_TASK_CORE LOOP_TASK_CORE
#define UI_TASK_DELAY   20
#define UI_TASK_STACK_SIZE 3200

// +-----------------------------------------------------------------------+
//                                                      Audio Task
extern TaskHandle_t audio_task_handle;
#define AUDIO_TASK_PRIORITY 4
#define AUDIO_TASK_CORE LOOP_TASK_CORE
#define AUDIO_TASK_STACK_SIZE 5000

// +-----------------------------------------------------------------------+
//                                                      GPS Task
extern TaskHandle_t gps_task_handle;
#define GPS_TASK_PRIORITY 2
#define GPS_TASK_CORE 0
#define GPS_TASK_DELAY   1
#define GPS_TASK_STACK_SIZE 3000

// +-----------------------------------------------------------------------+
//                                                      Display Task
extern TaskHandle_t display_task_handle;
#define DISPLAY_TASK_PRIORITY 1
#define DISPLAY_TASK_CORE 0
#define DISPLAY_TASK_DELAY   1000
#define DISPLAY_TASK_STACK_SIZE 4000

// +-----------------------------------------------------------------------+
//                                                      Sync File Task
extern TaskHandle_t sync_file_task_handle;
#define SYNC_FILE_TASK_PRIORITY 5
#define SYNC_FILE_TASK_CORE 0
#define SYNC_FILE_TASK_DELAY   15
#define SYNC_FILE_TASK_STACK_SIZE 3000
