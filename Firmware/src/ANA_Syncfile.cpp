#include "ANA_Syncfile.h"
#include "ANA_Tasks.h"

extern long old_show_start;
extern long show_start;
uint8_t midi_channel = 0;
TaskHandle_t sync_file_task_handle;

//----------------------------------------------------------------------------------------

SyncFile::SyncFile() { ; }

//----------------------------------------------------------------------------------------

void SyncFile::begin(void)
{
    Serial1.begin(31250, SERIAL_8N1, -1, 10);

    _file = SD.open(SHOW_SYNCFILE, FILE_READ);
    if (!_file)
    {
        log_e("Cannot open Sync file");
        return;
    }

    _file.seek(_file.size() - 7, SeekSet);
    uint32_t t = 0;

    for (int i = 0; i < 4; i++)
    {
        t |= (_file.read() << (i * 8));
    }
    log_v("Read: %d", t);
    _last_trigger = t;
    _isEOF = true;
}


//----------------------------------------------------------------------------------------

void SyncFile::end(void)
{
        _isEOF = true;
        if (_file) {
            _file.close();
        }
}

//----------------------------------------------------------------------------------------

void SyncFile::rewind(void)
{
    _isEOF = true;
}

//----------------------------------------------------------------------------------------

void SyncFile::start(long delay)
{
    if(!_file) return;
    log_v("START");
    _file.seek(0);
    _start_time = get_clock_millis();
    // round to exact beginning of the second
    _start_time = _start_time / 1000;
    _start_time = _start_time * 1000;
    _start_time += delay;

    _isEOF = false;
    getNext();
}

//----------------------------------------------------------------------------------------

uint32_t SyncFile::getLength()
{
    return _last_trigger;
}

//----------------------------------------------------------------------------------------

boolean SyncFile::getNext()
{
        if(!_file) return false ;

    if (!_file.available())
    {
        _isEOF = true;
        log_v("Finished Tune");
        if (old_show_start){
            log_v("Set back to original show start");
            set_show_start(old_show_start);
            old_show_start = 0;
        }
        return false;
    }

    uint32_t t = 0;
    for (int i = 0; i < 4; i++)
    {
        t |= (_file.read() << (i * 8));
    }

    _next_trigger = t;
    _next_event.cmd = _file.read();
    _next_event.note = _file.read();
    _next_event.velocity = _file.read();
    return true;
}

//----------------------------------------------------------------------------------------

uint32_t SyncFile::run(void)
{

   /*  if (_isEOF)
        return 0; */

    if (_start_time > get_clock_millis())
    {
        // this happens when delayed
        log_v("starttime bigger than millis(): %d",get_clock_millis());
        return 200;
    }

    int32_t the_time = get_clock_millis() - _start_time;
//log_v("%d", the_time);

    if (the_time >= _next_trigger)
    {
        // spit out MIDI
        Serial1.write(_next_event.cmd | midi_channel);
        Serial1.write(_next_event.note);
        Serial1.write(_next_event.velocity);

        // log_v("cmd %02x %02x", _next_event.cmd, (_next_event.cmd & 0xF0) == 0x90);

        if ((_next_event.cmd & 0xF0) == 0x80)
        { // note_off
        }

        if ((_next_event.cmd & 0xF0) == 0x90)
        { // note_on
            digitalWrite(PIN_BTN_2, HIGH);
            digitalWrite(PIN_BTN_1, HIGH);

            sample_to_play = _next_event.note;
            vTaskResume(audio_task_handle);
        }

        // log_v("Data: %02x %02x %02x", _next_event.cmd, _next_event.note, _next_event.velocity);
        if (!getNext())
        {
            return 0;
        }
    }
    return 0;
}

//----------------------------------------------------------------------------------------
//                                        SYNC FILE TASK
void sync_file_task(void *p)
{
    sync_file.run();
}
//----------------------------------------------------------------------------------------
//                                        Check Sync File
/*
void check_sync_file()
{
    File f;
    bool rebuild_file = false;
    ;

    // check if show file exists
    if (!SD.exists(SHOW_SYNCFILE))
        rebuild_file = true;

    // check if MIDI file exists

    f = SD.open(SHOW_MIDIFILE, FILE_READ);

    if (!f)
    {
            log_e("Cannot find MIDI file")
         u8g2.clearBuffer();
        u8g2.setCursor(text_x, text_y);
        u8g2.printf("No file called: %s", SHOW_MIDIFILE);
        u8g2.sendBuffer();
        while (1)
        {
            ;
        }
    }

    // check if midi file has changed
    uint16_t d, t;
    f.printFatDate()

    f.getModifyDateTime(&d, &t);
    uint32_t timestamp = d << 16 | t;
    uint32_t old_timestamp = preferences.getLong("file_timestamp", 0);
    if (old_timestamp != timestamp)
        rebuild_file = true;

    if (!rebuild_file)
    {
        u8g2.clearBuffer();
        u8g2.setCursor(text_x, text_y);
        u8g2.printf("Files did not change.");
        u8g2.sendBuffer();
        delay(300);
        return;
    }

    // create sync file
    int err;

    const int box_x = 14;
    const int box_y = 40;
    const int box_w = 100;
    const int box_h = 20;
    u8g2.setFont(u8g2_font_tallpixelextended_te);

    u8g2.clearBuffer();
    u8g2.setCursor(text_x, text_y);
    u8g2.print("Create Sync File");
    u8g2.sendBuffer();

    err = SMF.load(SHOW_MIDIFILE);
    if (err != ANA_MIDIFile::E_OK)
    {
        u8g2.clearBuffer();
        u8g2.setCursor(text_x, text_y);
        u8g2.print("SMF load Error");
        u8g2.setCursor(0, text_y + 14);

        log_e("SMF load Error :%d", err);

        switch (err)
        {
        case ANA_MIDIFile::E_NO_FILE:
            u8g2.print("Blank file name");
            break;
        case ANA_MIDIFile::E_NO_OPEN:
            u8g2.print("Can't open file specified");
            break;
        case ANA_MIDIFile::E_NOT_MIDI:
            u8g2.print("File is not MIDI format");
            break;
        case ANA_MIDIFile::E_HEADER:
            u8g2.print("MIDI header size incorrect");
            break;
        case ANA_MIDIFile::E_FORMAT:
            u8g2.print("File format type not 0 or 1");
            break;
        case ANA_MIDIFile::E_FORMAT0:
            u8g2.print("File format 0 but more than 1 track");
            break;
        case ANA_MIDIFile::E_TRACKS:
            u8g2.print(" More than MIDI_MAX_TRACKS required");
            break;
        default:
            u8g2.print(" Unknown Error");
            break;
        }
        u8g2.sendBuffer();
        delay(4000);
        return;
    }

    // SMF.dump();
    int progress;
    while (!SMF.isEOF())
    {
        progress = SMF.getNextEvent();
        if (progress % 2 == 0)
        {
            u8g2.clearBuffer();
            u8g2.setCursor(text_x, text_y);
            u8g2.print("Create Sync File");
            u8g2.drawFrame(box_x, box_y, box_w, box_h);
            u8g2.drawBox(box_x, box_y, progress, box_h);
            u8g2.sendBuffer();
        }
    }
    SMF.close();

    // copy temp file to show file
    if (SD.exists(SHOW_SYNCFILE))
    {
        if (!SD.remove(SHOW_SYNCFILE))
            log_e("Could not delete old syncfile");
    }
    if (!SD.rename("temp.sync", SHOW_SYNCFILE))
        log_e("Could not delete old syncfile");

    u8g2.clearBuffer();
    u8g2.setCursor(text_x, text_y);
    u8g2.print("Done.");
    u8g2.sendBuffer();
    preferences.putLong("file_timestamp", timestamp);
    delay(500);
}
 */