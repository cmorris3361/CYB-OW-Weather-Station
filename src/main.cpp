

// Things to do
// 01 - open - E - Set time based on Longitude and Latitude or country
// 02 - open - B - EPOC error when returning date and Time  - due to String limited to 16 chars 

// B: Bug, E - Enhancements

// Load third Party Libraries
#include <Arduino.h>
#include <TFT_eSPI.h>           // Used for TFT Display
#include <WiFi.h>               // Used for WiFi connection 

// Load Local Libraries
#include "version.h"
#include "settings.h"
#include "_functions.h"
#include "openWeather.h"


int16_t screenCentre = SCREEN_WIDTH / 2;
time_t dstOffset = 0;
uint16_t dialRadius = 40;                 // Radius for Dials
uint16_t dialDiameter = dialRadius * 2;   // Dial Dimeter


// Dial struct
struct gaugeParameters {
    uint16_t radius = dialRadius;
    uint16_t width = dialRadius;
    uint16_t height = dialRadius * 2;
    uint16_t thickness = 7;
    uint16_t posX;
    uint16_t posY;
    uint16_t min = 0; 
    uint16_t max = 100;
    String units;
    String label;
    bool showThreshold = false;
    uint16_t color = TFT_YELLOW;
    u_int16_t threshold2Value = 25;
    u_int16_t threshold2Color = TFT_GREEN;
    u_int16_t threshold3Value = 50;
    u_int16_t threshold3Color = TFT_ORANGE;
    u_int16_t threshold4Value = 75;
    u_int16_t threshold4Color = TFT_RED;
};



TFT_eSPI tft = TFT_eSPI();  
TFT_eSprite spr_WindDirDial = TFT_eSprite(&tft);
TFT_eSprite spr_WindDirNeedle = TFT_eSprite(&tft);
TFT_eSprite spr_Gauge = TFT_eSprite(&tft);
TFT_eSprite spr_Bar = TFT_eSprite(&tft);

simpleDSTadjust dstAdjusted(StartRule, EndRule);

OpenWeatherCurrentWeather currentWeather;

gaugeParameters gaugeTemperature;
gaugeParameters gaugeWind;
gaugeParameters gaugePressure;
gaugeParameters gaugeHumidity;
gaugeParameters gaugeCloud;
gaugeParameters gaugeRain;
gaugeParameters gaugeVisibility;

/* ********************************************************* */
/* **** Defined Functions **** */
/* ********************************************************* */
void initSerialPort();
void initDisplay();

void connectWiFi();
void drawProgressBar(int16_t x, int16_t y, int16_t value);
void updateStatusBar(int16_t x, String msg);
void drawWiFiQuality(int16_t value);
void drawTime();
void getTime();
void drawTemperature(uint16_t x, uint16_t y);

void createDialScale(int16_t start_angle, int16_t end_angle, int16_t increment);
void drawEmptyDial(String label, int16_t val);
void plotDial(int16_t x, int16_t y, int16_t angle, String label, uint16_t val);
void createNeedle(void);

void setupGauges();
void drawGauge(uint16_t value, gaugeParameters p);

void drawBar(float value, gaugeParameters p);


void setTime(uint16_t timezone);
/* ********************************************************* */

void setup() {
  initSerialPort();
  initDisplay();  
  screenCentre = SCREEN_WIDTH / 2;

  tft.drawString(HOSTNAME, 0,0, sFONT_SIZE);
  tft.drawString(VERSION, 0, 16, sFONT_SIZE);
  connectWiFi();
  
  getTime();

  // Have a small delay before clearing screen
  logEntry("Waiting 2 seconds..");
  delay(2000);
  tft.fillScreen(TFT_BLACK);
  
  // Draw Status Bar
  tft.fillRect(0, SCREEN_HEIGHT - sbh, SCREEN_WIDTH, sbh, TFT_LIGHTGREY);
  updateStatusBar(2, VERSION_SHORT);
  updateStatusBar(55, WiFi.localIP().toString());

 // Create the dial Sprite and dial (this temporarily hijacks the use of the needle Sprite)
  createDialScale(0, 360, 360 / 8);   // create scale (start angle, end angle, increment angle)
  createNeedle();                // draw the needle graphic
 
  
}

unsigned long millisNow;
unsigned long nextTick;
int16_t weatherCountDown;

void loop() {
  millisNow = millis();
  
  // 1 Second Tick
  if (millisNow >= nextTick) {
    drawTime();
    drawWiFiQuality(getWiFiQuality(WiFi.RSSI()));

    // Get Weather
    if (weatherCountDown == 0){
      weatherCountDown = weatherInterval;   // Reset Weather Countdown
      // Get time from NTP sever
      getTime();

      // Update Weather, check if Wifi Still Connected - restart if not
      if (WiFi.status() != WL_CONNECTED) { ESP.restart(); }
      getWeather(&currentWeather, owCurrentWeatherURL());
      
      // Get Time - Need to cal get weather to get TimeZone
      //setTime(currentWeather.timeZone);
      
      updateStatusBar(140, cvtEPOCtoDateTime(currentWeather.observationTime, true)); // Display ObservationTime (NEED TO CONVERT TO REAL TIME....)
      
      // Display Location
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.drawString(currentWeather.location, 0, 0 , mFONT_SIZE);
      tft.drawString(currentWeather.description, 0, 15, mFONT_SIZE);
      tft.drawRightString(cvtEPOCtoDateTime(currentWeather.sunrise, true), SCREEN_WIDTH, 0, mFONT_SIZE);
      tft.drawRightString(cvtEPOCtoDateTime(currentWeather.sunset, true), SCREEN_WIDTH, 15, mFONT_SIZE);
      
      setupGauges();
      // **** First Row ****
      drawGauge(currentWeather.temp, gaugeTemperature);
      drawGauge(currentWeather.windSpeed, gaugeWind);
      plotDial(180, 40, currentWeather.windDeg - 180,  degtoDir(currentWeather.windDeg), currentWeather.windDeg);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.drawCentreString("Direction", 180 + dialRadius, 40 + dialDiameter, sFONT_SIZE);
      
      // Second Row
      drawGauge(currentWeather.pressure, gaugePressure);
      drawGauge(currentWeather.humidity, gaugeHumidity);
      drawGauge(currentWeather.clouds, gaugeCloud);

      drawBar(currentWeather.visibility, gaugeVisibility);
      drawBar(currentWeather.rain, gaugeRain);

    }

    weatherCountDown --;
    updateStatusBar(200, leadZeros(weatherCountDown, 3));

    nextTick = millisNow + 1000;
  }

}

/* ********************************************************* */

// Initialise Serial Port for debugging purposes
void initSerialPort(){
  Serial.begin(115200);  // Start the Serial Monitor
  while (!Serial) { delay(100); }
}

// Initialise CYD TFT Display
void initDisplay(){
  tft.init();                               // Initialise TFT display
  tft.setRotation(3);                       // Set the TFT display rotation to landscape mode
  tft.fillScreen(TFT_BLACK);                // Clear the screen before writing to it, Black with White Text
  tft.setTextColor(TFT_WHITE, TFT_BLACK);   // Set Inital Foreground and Background colors
}

// Connect to WiFi
void connectWiFi(){
  int8_t attempt = 1;
  String m;
  logEntry("Connecting to " + (String)WIFI_SSID + " WiFi.");
  if (WiFi.status() == WL_CONNECTED) return;
  
  logEntry("Connecting to WiFi (" + (String)WIFI_SSID + ").");
  tft.drawString("Connecting to WiFi ( " + (String) WIFI_SSID + " ).", 0, 32);

  WiFi.mode(WIFI_STA);
  //WiFi.hostname(WIFI_HOSTNAME);
  WiFi.begin(WIFI_SSID,WIFI_PASS);  
  while ((WiFi.status() != WL_CONNECTED) && (attempt <= wifiRetryCount)) {
    m = "Attempt " + (String)attempt + "/" + (String)wifiRetryCount;
    logEntry(m);
    drawProgressBar(0, 50, attempt * (100 / wifiRetryCount));
    tft.drawString(m, 0, 66);
    
    delay(500);
    attempt++;
  }

  // Are we connected or not?
  if (WiFi.status() != WL_CONNECTED) {
    // Not Connected to Wifi
    m = "Unable to Connect to " + (String)WIFI_SSID + " to WifI AP.";
    logEntry(m);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.drawString(m, 0, 66);
    tft.drawString("Rebooting in 5 seconds.", 0, 77, mFONT_SIZE);  
    // reboot
    for (int x = 0; x < 5000; x+=50) {
      drawProgressBar(0, 95, x / 50);
      delay(10);
    }
    ESP.restart();
  } else {
    // Connected to WiFi
    m = "Connection to " + (String)WIFI_SSID + " AP has been successful.";
    logEntry(m + " IPAddress: " + WiFi.localIP().toString());
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString(m, 0, 66);
  }

}

void drawProgressBar(int16_t x, int16_t y, int16_t value){
  int16_t w = 100;
  int16_t h = 10;
  // Check value is in range (0-100)
  if (value > 100) { value = 100; }

  // Draw Progress Bar
  tft.drawRect(x, y, w + 2, h + 2, TFT_DARKGREY);
  tft.fillRect(x + 1, y + 1, value, h, TFT_RED);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString((String)value + "%", x + 103, y + 3, sFONT_SIZE);
}

void updateStatusBar(int16_t x, String msg){
  tft.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  tft.drawString(msg, x, SCREEN_HEIGHT - sbh + 1, sFONT_SIZE);
}

void drawWiFiQuality(int16_t value){
  int16_t x = 275;
  int16_t barHeight;
  int16_t barColor;
  for (int8_t bar = 0; bar < 4; bar++){
    if (value > 75) { barColor = TFT_GREEN; }
    else if (value > 50) { barColor = TFT_GREENYELLOW; }
    else if (value > 25) { barColor = TFT_GREEN; }
    else if (value > 0 ) { barColor = TFT_RED; }
    else { barColor = TFT_BLACK; };
    barHeight = (bar + 1) * 2;
    tft.drawFastVLine(x + (bar * 2), SCREEN_HEIGHT - barHeight, barHeight, barColor);
  }   
  updateStatusBar( x + 12, (String)value + "% ");
 } 
 
 // *** TIME ***
 void drawTime() {
  char time_str[11];
  char *dstAbbrev;
  time_t now = dstAdjusted.time(&dstAbbrev);
  struct tm * timeinfo = localtime (&now);

  // Format Time
  if (IS_STYLE_12HR) {
    // 12 Hour Clock (hh:mm:ss am)
    int hour = (timeinfo->tm_hour+11)%12+1;  // take care of noon and midnight
    sprintf(time_str, "%2d:%02d:%02d %s",hour, timeinfo->tm_min, timeinfo->tm_sec, timeinfo->tm_hour>=12?"PM":"AM");
  } else {
    // 24 Hour Clock (hh:mm:ss)
    sprintf(time_str, "%02d:%02d:%02d",timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
  }
  
  // Format Date 
  String date = ctime(&now);
  date = date.substring(0,11) + String(1900 + timeinfo->tm_year);

  // Display time and Date
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString(time_str, screenCentre, 0, lFONT_SIZE);
  tft.drawCentreString(date, screenCentre, 20, mFONT_SIZE);
  updateStatusBar(250, dstAbbrev);
}

void getTime(){
  configTime(UTC_OFFSET * 3600, 0, NTP_SERVERS);
  while(!time(nullptr)) {
    Serial.print("#");
    delay(100);
  }
}
void setTime(uint16_t timezone){
  configTime(timezone, 0, NTP_SERVERS);
  while(!time(nullptr)) {
    Serial.print("#");
    delay(100);
  }
  // calculate for time calculation how much the dst class adds.
//  dstOffset = UTC_OFFSET * 3600 + dstAdjusted.time(nullptr) - time(nullptr);
//  logEntry("Time difference for DST: " + (String)dstOffset);
}



void drawTemperature(uint16_t x, uint16_t y){
  uint16_t min = int(currentWeather.min_temp);
  uint16_t max = int(currentWeather.feelsLike) + 1;
  uint16_t dailArcThickness = 7;
  float angle = 300 / (max - min);  // Angle per Degrees (temp)
  int temperature = roundup(currentWeather.temp); // Round to nearest

  tft.drawArc(x, y, dialRadius, dialRadius, - dailArcThickness, 30, 330, TFT_SILVER, TFT_BLACK );
  tft.setTextColor(TFT_BLUE, TFT_BLACK);  tft.drawString((String)min, x - dialRadius + 24, y + dialRadius - 10, sFONT_SIZE);
  tft.setTextColor(TFT_RED, TFT_BLACK);   tft.drawString((String)max, x - dialRadius + 40, y + dialRadius - 10, sFONT_SIZE);

  // Draw Temperature
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString((String)temperature, x , y - 12, lFONT_SIZE);
  tft.drawCentreString("C", x, y + 10, mFONT_SIZE);

  // Draw Arcs
  uint16_t endAngle = roundup(currentWeather.feelsLike - currentWeather.min_temp) * angle;
  tft.drawArc(x, y, dialRadius, dialRadius - dailArcThickness, 30, endAngle, TFT_RED, TFT_BLACK );
  endAngle = (temperature - currentWeather.min_temp) * angle;
  tft.drawArc(x, y, dialRadius, dialRadius - dailArcThickness, 30, endAngle, TFT_YELLOW, TFT_BLACK );

  Serial.print("   Current Temperature: "); Serial.print(currentWeather.temp); Serial.println(" C");
  Serial.print("Feels Like Temperature: "); Serial.print(currentWeather.feelsLike); Serial.println(" C");
}


// =======================================================================================
// Create the dial sprite, the dial outer and place scale markers
// =======================================================================================

void createDialScale(int16_t start_angle, int16_t end_angle, int16_t increment)
{
  // Create the dial Sprite
  spr_WindDirDial.setColorDepth(8);       
  spr_WindDirDial.createSprite(dialDiameter, dialDiameter);   
  spr_WindDirDial.setPivot(dialRadius, dialRadius);       // set pivot in middle of dial Sprite

  // Draw dial outline
  spr_WindDirDial.fillSprite(TFT_TRANSPARENT);           // Fill with transparent colour
  spr_WindDirDial.fillCircle(dialRadius, dialRadius, dialRadius-2, TFT_DARKGREY);  // Draw dial outer

  // Hijack the use of the needle Sprite since that has not been used yet!
  spr_WindDirNeedle.createSprite(3, 3);     // 3 pixels wide, 3 high
  spr_WindDirNeedle.fillSprite(TFT_WHITE);  // Fill with white
  spr_WindDirNeedle.setPivot(1, dialRadius-2);        //  Set pivot point x to the Sprite centre and y to marker radius

  for (int16_t angle = start_angle; angle <= end_angle; angle += increment) {
    spr_WindDirNeedle.pushRotated(&spr_WindDirDial, angle); // Sprite is used to make scale markers
    yield(); // Avoid a watchdog time-out
  }

  spr_WindDirNeedle.deleteSprite(); // Delete the hijacked Sprite
}


// =======================================================================================
// Add the empty dial face with label and value
// =======================================================================================

void drawEmptyDial(String label, int16_t val)
{
  // Draw black face
  spr_WindDirDial.fillCircle(dialRadius, dialRadius, dialRadius-5, TFT_BLACK);
  spr_WindDirDial.drawPixel(dialRadius, dialRadius, TFT_WHITE);        // For demo only, mark pivot point with a while pixel
  
  // Draw Compass Points
  spr_WindDirDial.drawCentreString("N", dialRadius, 10, sFONT_SIZE);
  spr_WindDirDial.drawRightString("E", dialDiameter-5, dialRadius-5, sFONT_SIZE);
  spr_WindDirDial.drawCentreString("S", dialRadius, dialDiameter-15, sFONT_SIZE);
  spr_WindDirDial.drawString("W", 10, dialRadius-5, sFONT_SIZE);

  spr_WindDirDial.setTextColor(TFT_YELLOW, TFT_BLACK);       // Set Text Color
  spr_WindDirDial.setTextDatum(TC_DATUM);                    // Draw dial text
  spr_WindDirDial.drawString(label, dialRadius, 20, mFONT_SIZE);  // Display Dial Label
  spr_WindDirDial.drawNumber(val, dialRadius, dialDiameter - 27, sFONT_SIZE);    // Display Dial Value
}

// =======================================================================================
// Update the dial and plot to screen with needle at defined angle
// =======================================================================================

void plotDial(int16_t x, int16_t y, int16_t angle, String label, uint16_t val)
{
  // Draw the blank dial in the Sprite, add label and number
  drawEmptyDial(label, val);

  // Push a rotated needle Sprite to the dial Sprite, with black as transparent colour
  spr_WindDirNeedle.pushRotated(&spr_WindDirDial, angle, TFT_BLACK); // dial is the destination Sprite

  // Push the resultant dial Sprite to the screen, with transparent colour
  spr_WindDirDial.pushSprite(x, y, TFT_TRANSPARENT);
}

// =======================================================================================
// Create the needle Sprite and the image of the needle
// =======================================================================================

void createNeedle(void)
{
  spr_WindDirNeedle.setColorDepth(8);
  spr_WindDirNeedle.createSprite(11, (dialRadius * 2)-8); // create the needle Sprite 11 pixels wide by 49 high
  spr_WindDirNeedle.fillSprite(TFT_BLACK);          // Fill with black
  // Define needle pivot point
  uint16_t piv_x = spr_WindDirNeedle.width() / 2;   // x pivot of Sprite (middle)
  uint16_t piv_y = spr_WindDirNeedle.height()/ 2;   // y pivot of Sprite (10 pixels from bottom)
  spr_WindDirNeedle.setPivot(piv_x, piv_y);         // Set pivot point in this Sprite
  uint16_t needleLength = spr_WindDirNeedle.height() - 10; 

  // Keep needle tip 1 pixel inside dial circle to avoid leaving stray pixels
  spr_WindDirNeedle.drawFastVLine(piv_x, 0, needleLength, TFT_RED);
  spr_WindDirNeedle.drawCircle(piv_x, needleLength+3, 3, TFT_RED);
  spr_WindDirNeedle.fillTriangle(piv_x, 0, piv_x-3, 6, piv_x+3 ,6, TFT_YELLOW);

  // Draw needle centre boss
  spr_WindDirNeedle.fillCircle(piv_x, piv_y, 5, TFT_MAROON);
  spr_WindDirNeedle.drawPixel( piv_x, piv_y, TFT_WHITE);     // Mark needle pivot point with a white pixel
}



void setupGauges(){
  // Temperature Gauge
  gaugeTemperature.label = "Temperature";
  gaugeTemperature.units = "C";
  gaugeTemperature.posX = 0;
  gaugeTemperature.posY = 40;
  gaugeTemperature.min = int(currentWeather.min_temp);
  gaugeTemperature.max = int(currentWeather.max_temp + .5);
  gaugeTemperature.showThreshold = true;
  gaugeTemperature.color = TFT_BLUE;
  gaugeTemperature.threshold2Value = 17;
  gaugeTemperature.threshold3Value = 25;
  gaugeTemperature.threshold4Value = 30;
  
  // Wind Speed Gauge
  gaugeWind.label = "Wind";
  gaugeWind.units = "mph";
  gaugeWind.posX = 90;
  gaugeWind.posY = 40;
  gaugeWind.min = 0;
  gaugeWind.max = roundup(currentWeather.windGust);
  gaugeWind.showThreshold = true;
  gaugeWind.color = TFT_GREEN;
  gaugeWind.threshold2Value = 7;
  gaugeWind.threshold2Color = TFT_YELLOW;
  gaugeWind.threshold3Value = 22;
  gaugeWind.threshold3Color = TFT_PURPLE;
  gaugeWind.threshold4Value = 39;
  gaugeWind.threshold4Color = TFT_RED;
     
  // Pressure Gauge
  gaugePressure.label = "Pressure";
  gaugePressure.units = "hPa";
  gaugePressure.posX = 0;
  gaugePressure.posY = 135;
  gaugePressure.min = 870;
  gaugePressure.max = 1085;
  gaugePressure.showThreshold = true;
  gaugePressure.color = TFT_BLUE;
  gaugePressure.threshold2Value = 970;
  gaugePressure.threshold2Color = TFT_GREEN;
  gaugePressure.threshold3Value = 1050;
  gaugePressure.threshold3Color = TFT_RED;
  gaugePressure.threshold4Value = 1050;
  gaugePressure.threshold4Color = TFT_RED;

  // Humidity Gauge
  gaugeHumidity.label = " Humidity";
  gaugeHumidity.units = "%";
  gaugeHumidity.posX = 90;
  gaugeHumidity.posY = 135;
  gaugeHumidity.min = 0;
  gaugeHumidity.max = 100;
  gaugeHumidity.showThreshold = true;
  gaugeHumidity.color = TFT_BLUE;
  gaugeHumidity.threshold2Value = 30;
  gaugeHumidity.threshold2Color = TFT_GREEN;
  gaugeHumidity.threshold3Value = 60;
  gaugeHumidity.threshold3Color = TFT_PURPLE;
  gaugeHumidity.threshold4Value = 90;
  gaugeHumidity.threshold4Color = TFT_RED;
 
  // Cloud Coverage Gauge
  gaugeCloud.label= "Clouds";
  gaugeCloud.units = "%";
  gaugeCloud.posX = 185;
  gaugeCloud.posY = 135;
  gaugeCloud.min = 0;
  gaugeCloud.max = 100;
  gaugeCloud.color = TFT_GREEN;
  gaugeCloud.showThreshold = true;
  gaugeCloud.threshold2Value = 20;
  gaugeCloud.threshold2Color = TFT_YELLOW;
  gaugeCloud.threshold2Value = 50;
  gaugeCloud.threshold3Color = TFT_ORANGE;
  gaugeCloud.threshold4Value = 90;
  gaugeCloud.threshold3Color = TFT_RED;

  // Visibility
  gaugeVisibility.label = "Vis";
  gaugeVisibility.units = "M";
  gaugeVisibility.posX = 285;
  gaugeVisibility.posY = 40;
  gaugeVisibility.max = 10000;
  gaugeVisibility.width = 30;
  gaugeVisibility.color = TFT_YELLOW;

  // Rain
  gaugeRain.label ="Rain";
  gaugeRain.units = "mm/h";
  gaugeRain.posX =  285;
  gaugeRain.posY = 135;
  gaugeRain.width = 30;
  gaugeRain.max = 5;
  gaugeRain.color = TFT_BLUE;
}

void drawGauge(uint16_t value, gaugeParameters p){

// A bit of House keeping
if (value < p.min) { p.min = value; }
else if (value > p.max) {p.max = value; }
if (p.min == p.max ) {p.max++; p.min--;}

float app = 300 / (p.max - p.min); // Angle per point
  int16_t startAngle = 30;
  int16_t endAngle = 330;

  spr_Gauge.createSprite(p.radius * 2, (p.radius * 2) + 10);
  spr_Gauge.fillScreen(TFT_BLACK);

//  Serial.printf("Units: %s, min: %i, max: %i, (%i), APP: %f, Deg: %f value: %i\n", p.units, p.min, p.max, p.max-p.min, app, value * app, value );
  // Draw Arc Background
  spr_Gauge.drawArc(p.radius, p.radius, p.radius, p.radius - p.thickness, startAngle, endAngle, TFT_SILVER, TFT_BLACK );

  // Set Bar Color
  uint16_t barColor = p.color;
   if (p.showThreshold) {
    if (value >= p.threshold4Value) {barColor = p.threshold4Color; }
    else if ( value >= p.threshold3Value) { barColor = p.threshold3Color; }
    else if ( value >= p.threshold2Value) { barColor = p.threshold2Color; }
  }

  spr_Gauge.drawArc(p.radius, p.radius, p.radius, p.radius - p.thickness, startAngle, (app * (value-p.min)) + startAngle , barColor, TFT_BLACK );

  spr_Gauge.setTextColor(TFT_WHITE, TFT_WHITE, false);
  spr_Gauge.setTextPadding(20);
  // Draw Value
  spr_Gauge.drawCentreString((String)value, p.radius, p.radius - 15, lFONT_SIZE);
  // Draw Units
  spr_Gauge.drawCentreString(p.units, p.radius, p.radius + 10, mFONT_SIZE);
  // Draw Label
  spr_Gauge.drawCentreString(p.label, p.radius, p.radius +  p.radius, sFONT_SIZE);

  // Max Value
  spr_Gauge.setTextColor(TFT_RED, TFT_BLACK);   
  spr_Gauge.drawCentreString((String)p.max, p.radius, (p.radius * 2 ) - 10, sFONT_SIZE);

  // Push Sprint to Display
  spr_Gauge.pushSprite(p.posX, p.posY);

}


// Draw Vertical Bar
void drawBar(float value, gaugeParameters p){
  // House keeping
  if (value > p.max) {p.max = int(value + 1); }

  uint16_t barW = p.width - 4;
  float scale = (float)(p.height-4) / (float)p.max;
  uint16_t barH = value * scale;
  
  Serial.println(scale);

  spr_Bar.createSprite(p.width, p.height + 10);
  spr_Bar.fillScreen(TFT_BLACK);

  spr_Bar.drawRoundRect(0, 0, p.width, p.height, 0, TFT_WHITE);
  spr_Bar.setTextColor(TFT_WHITE, TFT_BLACK);
  // Display Units
  spr_Bar.drawCentreString(p.label, p.width / 2, p.height + 1, sFONT_SIZE);
  // Draw Bar
  spr_Bar.fillRect(2, p.height - barH - 2, barW, barH, p.color);
  // Display Rain Value  
  spr_Bar.setTextColor(TFT_RED, TFT_BLACK);
  spr_Bar.drawCentreString((String)value, p.width / 2, 4, sFONT_SIZE);
  spr_Bar.drawCentreString(p.units, p.width / 2, 14, sFONT_SIZE);

  spr_Bar.pushSprite(p.posX, p.posY);
}

