#include <Arduino.h>

int16_t getWiFiQuality(int16_t value);

void logEntry(String msg);

String leadZeros(int16_t value, int8_t len);

float cvtCtoF(float Celsius);
float cvtKtoC(float Kevin);
float cvtKtoF(float kelvin);
float cvtMPStoMPH(float MS);
float cvtMPStoKPH(float MS);
float cvtMMtoInches(float mm);
float cvtInchestoMM(float inches);

String degtoDir(int d);

int16_t roundup(float value);

String cvtEPOCtoDateTime(unsigned long epoc, bool timeonly);
