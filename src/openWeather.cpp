#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJSON.h>

#include "openWeather.h"
//#include "settings.h"
#include "_functions.h"




// Return the URL for Current Weather 
String owCurrentWeatherURL(){
    String url = "http://api.openweathermap.org/data/2.5/weather?lat=" + (String)LATITUDE +
                 "&lon=" +  (String)LONGITUDE + 
                 "&appid=" + (String)owAPIkey +
                 "&lang=" +  (String)owLang;
    return url;
}
// Return the URL for Current Weather 
String ow3DaytWeatherURL(){
    String url = "https://api.openweathermap.org/data/2.5/forecast?lat=" + (String)LATITUDE +
                 "&lon=" +  (String)LONGITUDE + 
                 "&appid=" + (String)owAPIkey +
                 "&lang=" +  (String)owLang;
    return url;
}

// Make the OpenWeather API Call and return JSON respose
void getWeather(OpenWeatherCurrentWeather *data, String URL){
    logEntry("Getting Weather.");
    logEntry("Requesting URL: " + URL);

    HTTPClient http;
 
    http.begin(URL);    // Call the URL
    int httpCode = http.GET();  //Make the request
 
    if (httpCode > 0) { //Check for the returning code
 
        String payload = http.getString();
        logEntry("HTTP Return code: " + (String)httpCode);
        logEntry("Respose: " + payload);

        // Allocate the JSON document
        JsonDocument doc;

        // Parse JSON object
        DeserializationError error = deserializeJson(doc, payload);
        if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            return;
        }
    

        data->observationTime = doc["dt"].as<unsigned long>();
        if (TEMPERATURE_IN_CELSIUS) {
            data->temp = cvtKtoC(doc["main"]["temp"].as<float>());
            data->feelsLike = cvtKtoC(doc["main"]["feels_like"].as<float>());
            data->min_temp = cvtKtoC(doc["main"]["temp_min"].as<float>());
            data->max_temp = cvtKtoC(doc["main"]["temp_max"].as<float>());
        } else {
            data->temp = cvtKtoF(doc["main"]["temp"].as<float>());
            data->feelsLike = cvtKtoF(doc["main"]["feels_like"].as<float>());
            data->min_temp = cvtKtoF(doc["main"]["mtemp_min"].as<float>());
            data->max_temp = cvtKtoF(doc["main"]["temp_max"].as<float>());
        }

        data->pressure = doc["main"]["pressure"].as<uint16_t>();
        data->humidity = doc["main"]["humidity"].as<uint8_t>();
        data->visibility = doc["visibility"].as<uint16_t>();
        
        data->clouds = doc["clouds"]["all"].as<uint8_t>();
        if (RAIN_IN_INCHES) {
            data->rain = cvtMMtoInches(doc["rain"]["1h"].as<float>());
        } else {
            data->rain = doc["rain"]["1h"].as<float>(),2;
        }

        if (WIND_IN_MPH) {
            data->windSpeed = cvtMPStoMPH(doc["wind"]["speed"].as<float>());
            data->windGust = cvtMPStoMPH(doc["wind"]["gust"].as<float>());
        } else {
            data->windSpeed = doc["wind"]["speed"].as<float>();
            data->windGust = doc["wind"]["gust"].as<float>(),1;
        }
        data->windDeg = doc["wind"]["deg"].as<float>(),1;
        data->main = doc["weather"][0]["main"].as<const char*>();
        data->description = doc["weather"][0]["description"].as<const char*>();
        data->icon = doc["weather"][0]["icon"].as<const char*>()+ doc["timezone"].as<unsigned long>();      // Add Time Zone 
        data->sunrise = doc["sys"]["sunrise"].as<unsigned long>() + doc["timezone"].as<unsigned long>();
        data->sunset = doc["sys"]["sunset"].as<unsigned long>() + doc["timezone"].as<unsigned long>();
        data->location = doc["name"].as<const char*>();
        data->timeZone = doc["timezone"].as<unsigned long>();
        
    } else {
      logEntry("Error on HTTP request");
    }
 
    http.end(); //Free the resources

}
