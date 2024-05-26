#include <Arduino.h>



typedef struct OpenWeatherCurrentWeather {
  String main;
  String description;
  String icon;
  String location;
  float temp;
  float feelsLike;
  float min_temp;
  float max_temp;
  uint16_t pressure;
  uint8_t humidity;
  float rain;
  uint16_t visibility;
  float windSpeed;
  float windGust;
  uint16_t windDeg;
  uint8_t clouds;
  uint32_t observationTime;
  uint16_t timeZone;
  uint32_t sunrise;
  uint32_t sunset;
} OpenWeatherCurrentWeather;




// Open Weather Settings

#define TEMPERATURE_IN_CELSIUS true // False = Fahrenheit
#define WIND_IN_MPH true            // False Meters per Second (M/S)
#define RAIN_IN_INCHES false         // False mm

#define LATITUDE 12.568452  //Hua Hin
#define LONGITUDE 99.957726
//#define LATITUDE    52.792951 // BOT 
//#define LONGITUDE   -1.662980 

#define owAPIkey    "2e5392393b1528fe5dd2e285aa83691c"
#define owLang      "en"
/*
Arabic -> ar, Bulgarian -> bg, Catalan -> ca, Czech -> cz, German -> de, Greek -> el,
English -> en, Persian (Farsi) -> fa, Finnish -> fi, French -> fr, Galician -> gl,
Croatian -> hr, Hungarian -> hu, Italian -> it, Japanese -> ja, Korean -> kr,
Latvian -> la, Lithuanian -> lt, Macedonian -> mk, Dutch -> nl, Polish -> pl,
Portuguese -> pt, Romanian -> ro, Russian -> ru, Swedish -> se, Slovak -> sk,
Slovenian -> sl, Spanish -> es, Turkish -> tr, Ukrainian -> ua, Vietnamese -> vi,
Chinese Simplified -> zh_cn, Chinese Traditional -> zh_tw. */




String owCurrentWeatherURL();
String ow3DayWeatherURL();

void getWeather(OpenWeatherCurrentWeather *data, String URL);