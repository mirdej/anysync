#pragma once
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

extern WiFiMulti wifiMulti;

void parse_show_file()
{
            rtc.offset = 7200;

return;


    log_v("------PARSE SHOW-------");
    File f = SD.open("/show.json", FILE_READ);
    if (!f)
    {
        display_messages.push("FAIL SHOW FILE");
        rtc.offset = 7200;

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

    JsonArray array = doc["devices"].as<JsonArray>();
    for (JsonVariant v : array)
    {
        const char *macadd = v["mac"];
        log_v("---->MAC: %s, %s", macadd, WiFi.macAddress().c_str());
        if (!strcmp(macadd, WiFi.macAddress().c_str()))
        {
            const char *temp2 = v["name"];
            hostname = temp2;
        }
    }
    display_messages.push("READ");
}
//----------------------------------------------------------------------------------------

void download_show_file()
{
    String url = "http://synkie.net/anysync/show.json";
    HTTPClient http;
    log_i("Downloading config file");
    display_messages.push("DOWNLOAD");

    // file_name = test2.htm
    Serial.println(url);
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0)
    {
        if (httpCode == HTTP_CODE_OK)
        {
            File f;
            // ifstream    sdout(stream;
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
}

void check_wifi_task(void *)
{
    WiFi.mode(WIFI_STA);

    while (1)
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            wifiMulti.run();
            if (WiFi.status() == WL_CONNECTED)
            {

                log_i("IP address: %s", WiFi.localIP().toString());
                //       download_show_file();
            }
        }
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}
