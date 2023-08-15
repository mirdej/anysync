#include "ANA_Audio.h"
#include "ANA_Tasks.h"

Audio audio;

TaskHandle_t audio_task_handle;

audioMessage audioTxMessage, audioRxMessage;

QueueHandle_t audioSetQueue = NULL;
QueueHandle_t audioGetQueue = NULL;

void CreateQueues()
{
  audioSetQueue = xQueueCreate(10, sizeof(struct audioMessage));
  audioGetQueue = xQueueCreate(10, sizeof(struct audioMessage));
}

void audioTask(void *parameter)
{
  CreateQueues();
  if (!audioSetQueue || !audioGetQueue)
  {
    log_e("queues are not initialized");
    while (true)
    {
      ;
    } // endless loop
  }

  log_v("Set Audio GPIOS");
  pinMode(PIN_MUTE_PCM, OUTPUT);
  digitalWrite(PIN_MUTE_PCM, HIGH);
  pinMode(PIN_HP_GAIN_0, OUTPUT);
  pinMode(PIN_HP_GAIN_1, OUTPUT);
  pinMode(PIN_HP_EN, OUTPUT);
  digitalWrite(PIN_HP_EN, HIGH);
  digitalWrite(PIN_HP_GAIN_0, LOW);
  digitalWrite(PIN_HP_GAIN_1, LOW);

  struct audioMessage audioRxTaskMessage;
  struct audioMessage audioTxTaskMessage;

  audio.setPinout(PIN_BCLK, PIN_LRCLK, PIN_DATA);
  audio.setVolume(15); // 0...21

  while (true)
  {
    if (xQueueReceive(audioSetQueue, &audioRxTaskMessage, 1) == pdPASS)
    {
      if (audioRxTaskMessage.cmd == SET_VOLUME)
      {
        audioTxTaskMessage.cmd = SET_VOLUME;
        audio.setVolume(audioRxTaskMessage.value);
        audioTxTaskMessage.ret = 1;
        xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
      }
      else if (audioRxTaskMessage.cmd == CONNECTTOHOST)
      {
        audioTxTaskMessage.cmd = CONNECTTOHOST;
        audioTxTaskMessage.ret = audio.connecttohost(audioRxTaskMessage.txt);
        xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
      }
      else if (audioRxTaskMessage.cmd == CONNECTTOSD)
      {
        audioTxTaskMessage.cmd = CONNECTTOSD;
        audioTxTaskMessage.ret = audio.connecttoSD(audioRxTaskMessage.txt);
        xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
      }
      else if (audioRxTaskMessage.cmd == GET_VOLUME)
      {
        audioTxTaskMessage.cmd = GET_VOLUME;
        audioTxTaskMessage.ret = audio.getVolume();
        xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
      }
      else
      {
        log_i("error");
      }
    }
    audio.loop();
    if (!audio.isRunning())
    {
      sleep(1);
    }
  }
}

void audioInit()
{
  xTaskCreatePinnedToCore(
      audioTask,             /* Function to implement the task */
      "audioplay",           /* Name of the task */
      AUDIO_TASK_STACK_SIZE,                  /* Stack size in words */
      NULL,                  /* Task input parameter */
      AUDIO_TASK_PRIORITY | portPRIVILEGE_BIT, /* Priority of the task */
      &audio_task_handle,                  /* Task handle. */
      AUDIO_TASK_CORE                      /* Core where the task should run */
  );
}

audioMessage transmitReceive(audioMessage msg)
{
  xQueueSend(audioSetQueue, &msg, portMAX_DELAY);
  if (xQueueReceive(audioGetQueue, &audioRxMessage, portMAX_DELAY) == pdPASS)
  {
    if (msg.cmd != audioRxMessage.cmd)
    {
      log_e("wrong reply from message queue");
    }
  }
  return audioRxMessage;
}

void audioSetVolume(uint8_t vol)
{
  audioTxMessage.cmd = SET_VOLUME;
  audioTxMessage.value = vol;
  audioMessage RX = transmitReceive(audioTxMessage);
}

uint8_t audioGetVolume()
{
  audioTxMessage.cmd = GET_VOLUME;
  audioMessage RX = transmitReceive(audioTxMessage);
  return RX.ret;
}

bool audioConnecttohost(const char *host)
{
  audioTxMessage.cmd = CONNECTTOHOST;
  audioTxMessage.txt = host;
  audioMessage RX = transmitReceive(audioTxMessage);
  return RX.ret;
}

bool audioConnecttoSD(const char *filename)
{
  audioTxMessage.cmd = CONNECTTOSD;
  audioTxMessage.txt = filename;
  audioMessage RX = transmitReceive(audioTxMessage);
  return RX.ret;
}
/* 
void audio_info(const char *info)
{
  Serial.print("info        ");
  Serial.println(info);
}
 */