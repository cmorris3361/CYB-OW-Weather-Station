#pragma once

#include <simpleDSTadjust.h>

// **** Changeable Settings ****

// WiFi Settings
#define WIFI_SSID "SKYRXTSQ"
#define WIFI_PASS "wNHWDc2m7iYM"



/* ***************************************************************** */
// ***** Other Settings *****

#define HOSTNAME "ESP32-CYD-Weather-Station"

const unsigned long weatherInterval = 10 * 60;      // Time between getting Weather in seconds 
const int8_t wifiRetryCount = 10;                   // Maximum number of attempts to connect to the WiFi
const int16_t sbh = 9;                             // StatusBar Height

#define UTC_OFFSET +0
struct dstRule StartRule = {"BST", Second, Sun, Mar, 27, 3600};    // Daylight time = UTC/GMT -1 hours
struct dstRule EndRule = {"GMT", First, Sun, Oct, 30, 0};       // Standard time = UTC/GMT -0 hour
const bool IS_STYLE_12HR = false;

// Set to require NTP pool (time servers)
#define NTP_SERVERS "0.uk.pool.ntp.org", "1.uk.pool.ntp.org", "2.uk.pool.ntp.org"
// #define NTP_SERVERS "0.ch.pool.ntp.org", "1.ch.pool.ntp.org", "2.ch.pool.ntp.org"
// #define NTP_SERVERS "us.pool.ntp.org", "time.nist.gov", "pool.ntp.org"

// CYD Display Settings 
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define sFONT_SIZE 1
#define mFONT_SIZE 2
#define lFONT_SIZE 4
#define xlFONT_SIZE 6
#define xxlFONT_SIZE 8


