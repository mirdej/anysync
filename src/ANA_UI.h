#pragma once
#include "ANA_Pins.h"
#include <Arduino.h>
#include "ANA_Tasks.h"
#include "ANA_Clock.h"

TaskHandle_t ui_task_handle;

void ui_task(void *p)
{
    log_v("Starting UI Task");

    pinMode(PIN_SD_DETECT, INPUT_PULLUP);
    uint8_t button_state = digitalRead(PIN_SD_DETECT);
    uint8_t temp;

    while (1)
    {
        temp = digitalRead(PIN_SD_DETECT);
        if (temp != button_state)
        {
            button_state = temp;
            if (button_state)
            {
                log_v("SD Card Removed");
                SD.end();
            }
            else
            {
                log_v("SD Card Inserted");
                SD.begin(PIN_SD_CS);
            }
        }

        vTaskDelay(UI_TASK_DELAY / portTICK_PERIOD_MS);
    }
}

void ui_begin()
{

    xTaskCreatePinnedToCore(
        ui_task,          /* Function to implement the task */
        "UI Task",        /* Name of the task */
        10000,            /* Stack size in words */
        NULL,             /* Task input parameter */
        UI_TASK_PRIORITY, /* Priority of the task */
        &ui_task_handle,  /* Task handle. */
        UI_TASK_CORE);    /* Core where the task should run */
}