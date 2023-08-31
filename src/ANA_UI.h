#pragma once

#include "ANA_Pins.h"
#include <Arduino.h>
#include "ANA_Tasks.h"
#include "ANA_Clock.h"
#include "ANA_Display.h"
#include "ANA_Syncfile.h"
#include "ANA_Audio.h"

int pot_value;
void set_show_start(uint32_t t);
TaskHandle_t ui_task_handle;
extern void parse_config();
long old_show_start;
extern AudioOutputI2S *out;

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

        //      ------------------------------------------------------------------------ Volume Pot
        int val = analogRead(PIN_POT);
        val = val / 16;
        val = (7 * pot_value + val) / 8;
        if (val != pot_value)
        {
            pot_value = val;
            float f = val / 255.;
            if (f < .1)
                f = .1;
            out->SetGain(f);
        }

        //      ------------------------------------------------------------------------ SD Card
        temp = digitalRead(PIN_SD_DETECT);
        if (temp != sd_state)
        {
            sd_state = temp;
            if (sd_state)
            {
                log_v("SD Card Removed");
                sd_card_present = 0;
                sync_file.end();
                SD.end();
            }
            else
            {
                log_v("SD Card Inserted");
                vTaskDelay(500);
                sd_card_present = SD.begin(PIN_SD_CS);
                if (sd_card_present)
                {
                    parse_config();
                }
            }
        }

        //      ------------------------------------------------------------------------ Play Button
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
                if (sync_file._file)
                {
                    if (sync_file._isEOF)
                    {
                        old_show_start = show_start;
                        set_show_start(rtc.getEpoch());
                        log_v("START from Button");
                    }
                    else
                    {
                        set_show_start(old_show_start);
                        old_show_start = 0;
                        log_v("STOP from Button");
                        sync_file.rewind();
                    }
                }
            }
        }
        vTaskDelay(UI_TASK_DELAY / portTICK_PERIOD_MS);
    }
}

void ui_begin()
{

    xTaskCreatePinnedToCore(
        ui_task,            /* Function to implement the task */
        "UI Task",          /* Name of the task */
        UI_TASK_STACK_SIZE, /* Stack size in words */
        NULL,               /* Task input parameter */
        UI_TASK_PRIORITY,   /* Priority of the task */
        &ui_task_handle,    /* Task handle. */
        UI_TASK_CORE);      /* Core where the task should run */
}