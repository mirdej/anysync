#include "ANA_Audio.h"
#include "ANA_Tasks.h"

TaskHandle_t audio_task_handle;

AudioGeneratorWAV *wav;
AudioFileSourceSD *file = NULL;
AudioOutputI2S *out;

int sample_to_play;

void audioTask(void *parameter)
{
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

  while (true)
  {
    if (sample_to_play > 0)
    {

      uint8_t n = sample_to_play;
      char buf[17];
      sprintf(buf, "/samples/%03d.wav", n);
      file = new AudioFileSourceSD(buf);
      //  log_v("File loaded");
      if (wav->isRunning())
      {
        wav->stop();
      }
      wav->begin(file, out);
      wav->loop();
      sample_to_play = 0;
      digitalWrite(PIN_BTN_1, HIGH);
    }

    if (wav->isRunning())
    {
      if (!wav->loop())
      {
        wav->stop();
      }
    }
    else
    {
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }
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
