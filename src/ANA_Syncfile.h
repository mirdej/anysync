#pragma once
#include <Arduino.h>
#include <SD.h>
#include "ANA_Clock.h"
#include "ANA_Audio.h"



#define SHOW_MIDIFILE "/show.mid"
#define SHOW_SYNCFILE "/show.sync"

extern uint8_t midi_channel;

typedef struct
{
    char cmd;
    char note;
    char velocity;
} midiEvent;

class SyncFile
{
public:
    SyncFile(void);
    void begin(void);
    boolean getNext(void);
    void start(void);
    uint32_t run(void);
    uint32_t getLength();

    File _file;
    int64_t _next_trigger;
    int64_t _last_trigger;
    int64_t _start_time;
    midiEvent _next_event;
    boolean _isEOF;

private:
};

void sync_file_task (void * p);

extern SyncFile sync_file;
