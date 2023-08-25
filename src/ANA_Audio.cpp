#include "ANA_Audio.h"
#include "ANA_Tasks.h"

TaskHandle_t audio_task_handle;

AudioGeneratorWAV *wav;
AudioFileSourceSD *file = NULL;
AudioOutputI2S *out;

int sample_to_play;

void audioTask(void *parameter)
{
  long last_play_millis;
  log_v("Set Audio GPIOS");
  pinMode(PIN_MUTE_PCM, OUTPUT);
  digitalWrite(PIN_MUTE_PCM, HIGH);
  pinMode(PIN_HP_GAIN_0, OUTPUT);
  pinMode(PIN_HP_GAIN_1, OUTPUT);
  pinMode(PIN_HP_EN, OUTPUT);
  digitalWrite(PIN_HP_EN, HIGH);
  digitalWrite(PIN_HP_GAIN_0, LOW);
  digitalWrite(PIN_HP_GAIN_1, LOW);

  wav = new AudioGeneratorWAV();
  // wav = new AudioGeneratorMP3();
  out = new AudioOutputI2S();
  out->SetGain(0.5);
  out->SetPinout(PIN_BCLK, PIN_LRCLK, PIN_DATA);
#define MIN_AUDIO_DELAY 20

  while (true)
  {
    if (sample_to_play > 0)
    {

      //                                                          ????????? still needed?
      /*    long t = millis() - last_play_millis;
         if (t < MIN_AUDIO_DELAY)
         {
           vTaskDelay(MIN_AUDIO_DELAY - t / portTICK_PERIOD_MS);
         }
         last_play_millis = millis(); */

      char buf[17];
      sprintf(buf, "/samples/%03d.wav", sample_to_play);
           digitalWrite(PIN_BTN_2, LOW);

      //  log_v("File loaded");
      if (wav->isRunning())
      {
        wav->stop();
      }
      delete file;
      file = new AudioFileSourceSD(buf);
       digitalWrite(PIN_BTN_1, LOW);

      wav->begin(file, out);
      //      wav->loop();*/
      sample_to_play = 0;
    }

    if (wav->isRunning())
    {
      if (!wav->loop())
      {
        wav->stop();
      }
       vTaskDelay(1 / portTICK_PERIOD_MS);  // give some time for other tasks while playing audio
    }
    else
    {
      vTaskSuspend(NULL);
    }
    // vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void audioInit()
{
  xTaskCreatePinnedToCore(
      audioTask,                               /* Function to implement the task */
      "audioplay",                             /* Name of the task */
      AUDIO_TASK_STACK_SIZE,                   /* Stack size in words */
      NULL,                                    /* Task input parameter */
      AUDIO_TASK_PRIORITY | portPRIVILEGE_BIT, /* Priority of the task */
      &audio_task_handle,                      /* Task handle. */
      AUDIO_TASK_CORE                          /* Core where the task should run */
  );
}
