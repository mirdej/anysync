#pragma once
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>


#define FILE_CHECK_INTERVAL 60000
extern WiFiMulti wifiMulti;
bool wifi_allowed = true;
 long last_file_check;

void parse_show_file()
{
    // Set defaults
    rtc.offset = 7200;
    hostname = "unknown";
    show_name = "unknown";
    device_delay = 0;

    log_v("------PARSE SHOW-------");
    File f = SD.open("/show.json", FILE_READ);
    if (!f)
    {
        display_messages.push("FAIL SHOW FILE");

        log_e("Failed to open config file");
        return;
    }
    DynamicJsonDocument doc(2048);
    log_i("READ");
    DeserializationError error = deserializeJson(doc, f);
    if (error)
    {
        log_e("Failed to read file");
        f.close();

        return;
    }
    f.close();
    log_v("Deserialized");
    const char *temp = doc["name"];
    show_name = temp;

    long gmt_offset = doc["gmt_offset"];

    log_v("GMT-offset %d", gmt_offset);
    rtc.offset = gmt_offset;

    long t = doc["start"];
    t += gmt_offset;
    log_v("Start: %d, Epoch: %d", t, rtc.getEpoch());

    if (t < rtc.getEpoch())
    {
        t = 0;
    }

    set_show_start(t);

    JsonArray array = doc["devices"].as<JsonArray>();
    for (JsonVariant v : array)
    {
        const char *macadd = v["mac"];
        log_v("---->MAC: %s, %s", macadd, WiFi.macAddress().c_str());
        if (!strcmp(macadd, WiFi.macAddress().c_str()))
        {
            const char *temp2 = v["name"];
            hostname = temp2;

            device_delay = v["delay"];
        }
    }
    display_messages.push("READ");
}
//----------------------------------------------------------------------------------------

void download_show_file()
{
    String url = "http://synkie.net/anysync/hello.php?mac=" + WiFi.macAddress();

    HTTPClient http;
    log_i("Downloading config file");
    display_messages.push("DOWNLOAD");
    Serial.println(url);
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0)
    {
        if (httpCode == HTTP_CODE_OK)
        {
            File f;
            SD.remove("/show.json");
            f = SD.open("/show.json", FILE_WRITE);

            if (f)
            {
                String payload = http.getString();
                f.print(payload);
                f.close();
                parse_show_file();
            }
            else
            {
                log_e("Failed to open file");
            }
        }
    }
    else
    {
        log_e("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
    last_file_check = millis();
}

void check_wifi_task(void *)
{

    while (1)
    {
        if (clock_status != unset)
        {
            wifi_allowed = false;
            long time_now = rtc.getEpoch();
            if (!show_start || time_now > show_end)
            {
                wifi_allowed = true;
            }
            if (time_now < show_start - 120)
            {
                wifi_allowed = true;
            }
        }

        if (wifi_allowed)
        {
            if (WiFi.status() != WL_CONNECTED)
            {
                WiFi.mode(WIFI_STA);
                wifiMulti.run();
                if (WiFi.status() == WL_CONNECTED)
                {
                    log_i("IP address: %s", WiFi.localIP().toString());
                    download_show_file();
                }
            }
            else
            {
                if (millis()-last_file_check > FILE_CHECK_INTERVAL) {
                    download_show_file();
                }
            }
        }
        else
        {
            WiFi.mode(WIFI_OFF);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
