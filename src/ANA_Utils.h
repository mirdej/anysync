#pragma once
#include <Arduino.h>


char version[16];

void makeversion(char const *date, char const *time, char *buff)
{
  int month, day, year, hour, min, sec;
  static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
  sscanf(date, "%s %d %d", buff, &day, &year);
  month = (strstr(month_names, buff) - month_names) / 3 + 1;
  sscanf(time, "%d:%d:%d", &hour, &min, &sec);
  sprintf(buff, "%d.%02d%02d.%02d", year, month, day, hour);
}



