#include "ANA_Syncfile.h"
#include "ANA_Tasks.h"

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

    xTaskCreatePinnedToCore(
        sync_file_task,            /* Function to implement the task */
        "SYNCFILE Task",           /* Name of the task */
        SYNC_FILE_TASK_STACK_SIZE, /* Stack size in words */
        NULL,                      /* Task input parameter */
        SYNC_FILE_TASK_PRIORITY,   /* Priority of the task */
        &sync_file_task_handle,    /* Task handle. */
        SYNC_FILE_TASK_CORE);      /* Core where the task should run */
}

//----------------------------------------------------------------------------------------

void SyncFile::start(void)
{
    log_v("START");
    _file.seek(0);
    _start_time = get_clock_millis();
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
    if (!_file.available())
    {
        _isEOF = true;
        log_v("Finished Tune");
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

    if (_isEOF)
        return 100;

    if (_start_time > get_clock_millis())
    {
        // this should never happen....
        log_e("starttime bigger than millis()");
        return 200;
    }

    int32_t the_time = get_clock_millis() - _start_time;
    //  log_v("%d", the_time);

    while (the_time >= _next_trigger)
    {
        // spit out MIDI
        Serial1.write(_next_event.cmd | midi_channel);
        Serial1.write(_next_event.note);
        Serial1.write(_next_event.velocity);

    //    log_v("cmd %02x %02x", _next_event.cmd, (_next_event.cmd & 0xF0) == 0x90);

       if ((_next_event.cmd & 0xF0) == 0x90)
        { // note_on
            uint8_t n = _next_event.note;
            char buf[16];
            sprintf(buf, "samples/%03d.wav", n);
            audioConnecttoSD(buf);
        } 

        // log_v("Data: %02x %02x %02x", _next_event.cmd, _next_event.note, _next_event.velocity);
        if (!getNext())
        {
            return 1000;
        }
    }

    the_time = get_clock_millis() - _start_time;
    if (_next_trigger > the_time)
    {
        return (_next_trigger - the_time);
    }
    else
    {
        return 1;
    }
}

//----------------------------------------------------------------------------------------
//                                        SYNC FILE TASK
void sync_file_task(void *p)
{
    log_v("Started SYFLOOP");
    while (1)
    {
        uint32_t delaytime = sync_file.run();
        //  log_v("Delay time %d", delaytime);
        if (delaytime > SYNC_FILE_TASK_DELAY)
            delaytime = SYNC_FILE_TASK_DELAY;
        vTaskDelay(delaytime / portTICK_PERIOD_MS);
    }
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