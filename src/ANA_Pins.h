
#pragma once
// Digital I/O used
#define __VERSION_20230109__ 1
#define SPI_SPEED SD_SCK_MHZ(40)


// pin declarations for hardware version 2023-01-09

#ifdef __VERSION_20230109__
#define PIN_BTN_DE 6
#define PIN_BTN_FR 5
#define PIN_MUTE_AMP 1
#define PIN_MUTE_PCM 2
#define PIN_BCLK 42
#define PIN_LRCLK 41
#define PIN_DATA 40
#define PIN_HP_EN 39
#define PIN_HP_GAIN_0 38
#define PIN_HP_GAIN_1 37
#define PIN_MISO 15
#define PIN_SCK 16
#define PIN_MOSI 17
#define PIN_SD_CS 18
#define PIN_SDA 14
#define PIN_SCL 13
#define PIN_RX1 21
#define PIN_TX1 47
#define PIN_1PPS 48
#define PIN_PIX 7
#define PIN_POT 4

#else
// pin declarations for hardware version 2023-05-02
#define PIN_POT 4
#define PIN_BTN_2 5
#define PIN_BTN_1 6
#define PIN_PIX 7

#define PIN_LRCLK 15
#define PIN_DATA 16
#define PIN_BCLK 17
#define PIN_MUTE_PCM 18

#define PIN_BATT_MON 8

#define PIN_MIDI_TX 10
#define PIN_RTC_1PPS 11
#define PIN_GPS_RST 12
#define PIN_SCL 13
#define PIN_SDA 14

#define PIN_GPS_TX 21
#define PIN_GPS_RX 47
#define PIN_GPS_1PPS 48

#define PIN_MUTE_AMP 1
#define PIN_HP_EN 2
#define PIN_SD_CS 42
#define PIN_MOSI 41
#define PIN_SCK 40
#define PIN_MISO 39

#define PIN_HP_GAIN_0 38
#define PIN_HP_GAIN_1 37
#endif
