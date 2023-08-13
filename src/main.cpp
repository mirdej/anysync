#include "Arduino.h"
#include "WiFi.h"
#include "ANA_Pins.h"
#include "ANA_Audio.h"
#include "ANA_Clock.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"
#include <ESPLogger.h>


String ssid = "Anymair";
String password = "Mot de passe pas complique";

ESPLogger logger("/log.txt", SD);   

//****************************************************************************************
//                                   S E T U P                                           *
//****************************************************************************************

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid.c_str(), password.c_str());
  while (WiFi.status() != WL_CONNECTED)
    delay(1500);

  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI);
  SPI.setFrequency(1000000);
  Serial.begin(115200);
  SD.begin(PIN_SD_CS);
  logger.begin();
  
  audioInit();
  audioSetVolume(15);
  log_i("current volume is: %d", audioGetVolume());

  clock_init();
}


// Variable to be logged
static int eventCounter = 0;

void event() {  
  eventCounter++;

  Serial.printf("Hey, event ->%d<- is just happened\n", eventCounter);
  char record[15];
  snprintf(record, 15, "value: %d", eventCounter);
  // the second parameter allows to prepend to the record the current timestamp
  bool success = logger.append(record, true);
  if (success) {
    Serial.println("Record stored!");
  } else {
    if (logger.isFull()) {
      Serial.println("Record NOT stored! You had filled the available space: flush or reset the "
                     "log before appending another record");
    } else {
      Serial.println("Something goes wrong, record NOT stored!");
    }
  }
}

//****************************************************************************************
//                                   L O O P                                             *
//****************************************************************************************

void loop()
{
 static long last;
  if (millis()-last > 4000) {
    last = millis();

    log_v("millis: %d, mymillis %d", millis(), clock_millis);
 /*    audioConnecttoSD("/samples/007.wav");
     event(); */
  } 
}
