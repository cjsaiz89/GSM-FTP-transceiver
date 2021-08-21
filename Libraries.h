// libraries - check changes in libraries GSM.cpp and endian.h
#include <MKRGSM.h>
#include <GSMFTP.h> //** library created by https://github.com/tryhus/MKRGSM/tree/feature/ftp
#include <GSMFileSystem.h>
#include <RTCZero.h> //** download this library from the library manager
#include <SPI.h>
#include <SD.h>
#include "ArduinoLowPower.h"

// initialize
GSMFTP ftp;
GPRS gprs;
GSM gsmAccess;
RTCZero rtc;
GSMScanner scannerNetworks;
File myfile; // 8.3 Format -> 8 char name and 3 char extension

// NOTE: GSMFTP.cpp -> increase timeouts in streaming functions to assure ftp connection
// C:\Users\YOURUSERNAME\Documents\Arduino\libraries\MKRGSM-feature-ftp\src

// Header and line structure
//MMDDYYhhmmss PSP_AVG PSP_MIN PSP_MAX PIR_AVG PIR_MIN PIR_MAX [W.m-2] -> 70 bytes
//011520100130 1171 1171 1173 1280 1010 1336 -> 44 bytes
// sample 16 hs/day & 1 sample minute -> 16 x 60 = 960 lines/day = 960 x 44 bytes + 70 bytes / day = 42310 bytes/day = 41.32 kB/day
// 41.32 kB/day x 30 day = 1239.6 kB/month = 1.21 MB/month
// 1.21 MB/month x 12 month = 14.53 MB/year
