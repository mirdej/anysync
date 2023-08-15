#pragma once
#include "ANA_Pins.h"
#include <Arduino.h>
#include "ANA_Tasks.h"
#include "ANA_Clock.h"
#include "ANA_Display.h"
#include "ANA_Syncfile.h"

TaskHandle_t ui_task_handle;

void ui_task(void *p)
{
    log_v("Starting UI Task");

    pinMode(PIN_SD_DETECT, INPUT_PULLUP);
    pinMode(PIN_BTN, INPUT_PULLUP);
    uint8_t sd_state = digitalRead(PIN_SD_DETECT);
    uint8_t button_state = digitalRead(PIN_BTN);
    uint8_t temp;
    static portMUX_TYPE my_spinlock = portMUX_INITIALIZER_UNLOCKED;

    while (1)
    {
        temp = digitalRead(PIN_SD_DETECT);
        if (temp != sd_state)
        {
            sd_state = temp;
            if (sd_state)
            {
                log_v("SD Card Removed");
                sd_card_present = 0;
                SD.end();
            }
            else
            {
                log_v("SD Card Inserted");
                vTaskDelay(500);
                sd_card_present = SD.begin(PIN_SD_CS);
            }
        }
        temp = digitalRead(PIN_BTN);
        if (temp != button_state)
        {
            button_state = temp;
            if (button_state)
            {
                log_v("BTN RELEASED");
            }
            else
            {
                log_v("BTN PRESSED");

                if (sync_file._isEOF)
                {
                    //    vPortEnterCritical(&my_spinlock);
                    show_start = rtc.getEpoch();
                    show_end = show_start + sync_file.getLength() / 1000 + 10;
                } else {
                    show_start = 0;
                    sync_file._isEOF = true;
                }
                //          vPortExitCritical(&my_spinlock);
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
        UI_TASK_STACK_SIZE,  /* Stack size in words */
        NULL,             /* Task input parameter */
        UI_TASK_PRIORITY, /* Priority of the task */
        &ui_task_handle,  /* Task handle. */
        UI_TASK_CORE);    /* Core where the task should run */
}