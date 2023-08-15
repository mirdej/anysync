#pragma once

#include "ANA_Pins.h"
#include "ANA_Clock.h"
#include "ANA_Utils.h"
#include "ANA_Tasks.h"
#include "Logo.h"
#include <iostream>
#include <queue>
#include <U8g2lib.h>

boolean sd_card_present;

TaskHandle_t display_task_handle;
String show_name = "Undefined";
unsigned long show_start = -1;
std::queue<String> display_messages;

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0,
                                         /* reset=*/U8X8_PIN_NONE,
                                         /* clock=*/PIN_SCL,
                                         /* data=*/PIN_SDA); // ESP32 Thing, HW I2C with pin remapping

#define LCDWidth u8g2.getDisplayWidth()
#define ALIGN_CENTER(t) ((LCDWidth - (u8g2.getUTF8Width(t))) / 2)
#define ALIGN_RIGHT(t) (LCDWidth - u8g2.getUTF8Width(t))
#define ALIGN_LEFT 0

#define MIN_RSSI -100
#define MAX_RSSI -55

static int calculateSignalLevel(int rssi, int numLevels)
{
    if (rssi <= MIN_RSSI)
    {
        return 0;
    }
    else if (rssi >= MAX_RSSI)
    {
        return numLevels - 1;
    }
    else
    {
        float inputRange = (MAX_RSSI - MIN_RSSI);
        float outputRange = (numLevels - 1);
        return (int)((float)(rssi - MIN_RSSI) * outputRange / inputRange);
    }
}

void intro()
{

    u8g2.clearBuffer();
    u8g2.drawBitmap(0, 0, 16, 16, bitmap_anyma);
    u8g2.drawBitmap(32, 16, 8, 48, bitmap_osh);
    u8g2.sendBuffer();
    delay(500);

    u8g2.clearBuffer();
    u8g2.drawBitmap(0, 0, 16, 16, bitmap_anyma);
    u8g2.setCursor(12, 45);
    u8g2.print("ANYSYNC");
    u8g2.setFont(u8g2_font_tallpixelextended_te);
    u8g2.setCursor(4, 64);
    u8g2.print("Version ");
    u8g2.print(version);
    u8g2.sendBuffer();
    delay(500);
}

void display_task(void *p)
{

    // init OLED display
    u8g2.begin();
    u8g2.setPowerSave(0);
    u8g2.setFont(u8g2_font_logisoso22_tr);

    intro();

    while (1)
    {
        long start = millis();
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_tallpixelextended_te);
        u8g2.setCursor(0, 8);
        // u8g2.setFont(u8g2_font_t0_12_me);

        if (gps.satellites.value())
        {
            for (size_t i = 0; i < gps.satellites.value(); i++)
            {
                u8g2.print("|");
            }
        }
        else
        {
            u8g2.print("NO GPS");
        }

        String wifiString = "NO WIFI";

        /*  if (WiFi.status() == WL_CONNECTED)
         {
             wifiString = "";
             int level = calculateSignalLevel(WiFi.RSSI(), 5);
             for (size_t i = 0; i < level; i++)
             {
                 wifiString += "|";
             }

             // wifiString = WiFi.SSID();
         } */

        u8g2.setCursor(ALIGN_RIGHT(wifiString.c_str()), 8);
        u8g2.print(wifiString);
        u8g2.drawHLine(0, 10, 128);
        u8g2.drawHLine(0, 54, 128);
        u8g2.setCursor(0, 64);
        //   u8g2.print(hostname);

        u8g2.setCursor(ALIGN_RIGHT(show_name.c_str()), 64);
        u8g2.print(show_name);

        rtc.offset = 7200;
        String time = rtc.getTime();
        u8g2.setCursor(ALIGN_CENTER(time.c_str()), 9);
        u8g2.print(time);

        /*   u8g2.setCursor(0, 20);
          u8g2.print(show_start); */
        u8g2.setFont(u8g2_font_logisoso22_tr);

        if (display_messages.size())
        {
            time = display_messages.front();
            display_messages.pop();
        }
        else if (!sd_card_present) {
                time = "NO SDCARD";
        }
        else
        {
            long time_now = rtc.getEpoch();
            if (time_now < 1579494213UL)
            {
                time = "NO TIME";
            }
            else
            {

                struct tm tm; // check epoch time at https://www.epochconverter.com/
                tm.tm_year = rtc.getYear() - 1900;
                tm.tm_mon = rtc.getMonth();
                tm.tm_mday = rtc.getDay();
                tm.tm_hour = 15;
                tm.tm_min = 0;
                tm.tm_sec = 0;
                tm.tm_isdst = -1; // disable summer time
                time_t t = mktime(&tm);

                show_start = t;
                log_v("SHowstart = %d NOW = %d", show_start, time_now);
                long difftime;
                time = "";

                if (time_now < show_start)
                {
                    difftime = show_start - time_now;
                    time = "-";
                }
                else
                {
                    difftime = time_now - show_start;
                }
                char tempBuffer[10];
                if (difftime > 3600)
                {
                    sprintf(tempBuffer, "%02u:%02u:%02u", difftime / 3600, (difftime % 3600) / 60, difftime % 60);
                }
                else
                {
                    sprintf(tempBuffer, "%02u:%02u", (difftime % 3600) / 60, difftime % 60);
                }

                time += tempBuffer;
            }
        }
        u8g2.setCursor(ALIGN_CENTER(time.c_str()), 48);
        u8g2.print(time);
        u8g2.sendBuffer();
        log_v("Display udate in %dms", millis() - start);

        vTaskDelay(DISPLAY_TASK_DELAY / portTICK_RATE_MS);
    }
}

void init_display()
{

    xTaskCreatePinnedToCore(
        display_task,          /* Function to implement the task */
        "DISPLAY Task",        /* Name of the task */
        10000,                 /* Stack size in words */
        NULL,                  /* Task input parameter */
        DISPLAY_TASK_PRIORITY, /* Priority of the task */
        &display_task_handle,  /* Task handle. */
        DISPLAY_TASK_CORE);    /* Core where the task should run */
}
