#ifndef __AS_SYNCFILE_INCLUDED__
#define __AS_SYNCFILE_INCLUDED__

#include "anysync.h"
#include "anysync_audio.h"
#include <SdFat.h>

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

    SDFILE _file;
    int64_t _next_trigger;
    int64_t _last_trigger;
    int64_t _start_time;
    midiEvent _next_event;
    boolean _isEOF;

private:
};

SyncFile::SyncFile() { ; }

void SyncFile::begin(void)
{
    Serial1.begin(31250, SERIAL_8N1, -1, 10);

    if (!_file.open("show.sync", O_READ))
    {
        log_e("Cannot open Sync file");
        return;
    }
    _file.seekSet(_file.size() - 7);
    uint32_t t;
    _file.read(&t, 4);
    log_v("Read: %d", t);
    _last_trigger = t;
    _isEOF = true;

    init_audio();
}

void SyncFile::start(void)
{
    log_v("START");
    _file.rewind();
    _start_time = NTP.millis(); // rtc.getEpoch() * 1000 + rtc.getMillis();
    _isEOF = false;
    getNext();
}

uint32_t SyncFile::getLength()
{
    return _last_trigger;
}

boolean SyncFile::getNext()
{

    uint32_t t;
    if (_file.read(&t, 4) < 4)
    {
        _isEOF = true;
        log_v("Finished Tune");
        return false;
    }
    else
    {
        _next_trigger = t;
        _next_event.cmd = _file.read();
        _next_event.note = _file.read();
        _next_event.velocity = _file.read();
        return true;
    }
}

uint32_t SyncFile::run(void)
{
    //  int64_t last_millis;
    if (_isEOF)
        return 100;

    /*   if (rtc.getMillis() == last_millis)
          return;

      last_millis = rtc.getMillis(); */
    /*  if (NTP.millis() == last_millis)
         return;
  */
    //   last_millis = NTP.millis();
    if (_start_time > NTP.millis())
    {
        // this should never happen....
        log_e("starttime bigger than millis()");
        return 200;
    }

    int64_t the_time = NTP.millis() - _start_time;
  //  log_v("%d", the_time);

    while (the_time >= _next_trigger)
    {
        // spit out MIDI
        Serial1.write(_next_event.cmd | midi_channel);
        Serial1.write(_next_event.note);
        Serial1.write(_next_event.velocity);

 //      log_v("cmd %02x %02x", _next_event.cmd, (_next_event.cmd & 0xF0) == 0x90);

        if ((_next_event.cmd & 0xF0) == 0x90)
        { // note_on
            uint8_t n = _next_event.note;
            char buf[16];
            sprintf(buf, "samples/%03d.wav", n);
            play(buf);
        }

        // log_v("Data: %02x %02x %02x", _next_event.cmd, _next_event.note, _next_event.velocity);
        // check_audio();
        if (!getNext())
        {
            return 1000;
        }
    }
    the_time = NTP.millis() - _start_time;
    if (_next_trigger > the_time)
    {
        return (_next_trigger - the_time);
    }
    else
    {
        return 1;
    }
}

#endif