#include <Arduino.h>
#include "_functions.h"

 
// converts the dBm to a range between 0 and 100%
int16_t getWiFiQuality(int16_t dbm){
    if(dbm <= -100) {
        return 0;
    } else if(dbm >= -50) {
        return 100;
    } else {
      return 2 * (dbm + 100);
    }
}

void logEntry(String msg){
  Serial.print((String)millis()); Serial.println(", " + msg);
}

String leadZeros(int16_t value, int8_t len){
    String v = (String)value;
    while (v.length() < len){ v = "0" + v; }
    return v;
}

// Convert Celsius to Fahrenheit
float cvtCtoF(float Celsius){
    return (Celsius * 1.8) + 32;
}

// Convert Kelvin to Celsius
 float cvtKtoC(float Kelvin){
    return Kelvin - 272.15;
 }

// Convert Kelvin to Fahrenheit
 float cvtKtoF(float Kelvin){
    // Convert to Celsius and then to Fahrenheit
    return cvtCtoF(cvtKtoC(Kelvin));   
 }

// Convert Meter per Second to Mile per Hour (m/s to mph)
float cvtMPStoMPH(float MS){
    return MS / 0.44704;
}

// Convert Meters per Second to Kilometers per Hour (m/s to km/h)
float cvtMPStoKPH(float MS){
    return MS * 3.6;
} 

// Convert Millimeters (mm) to Inches
float cvtMMtoInches(float mm){
    return mm / 25.4;
}

// Convert Inches to Millimeters(mm)
float cvtInchestoMM(float inches){
    return inches * 25.4;
}


// Return Textual Direction for Degrees (0 - 359);
String degtoDir(int d){
  // Note NORTH (N) = 348.75 - 11.25 Degrees
  String direction[16] = {"N","NNE", "NE", "ENE", "E", "ESE", "SE", "SSE", "S", "SSW", "SW","WSW", "W","WNW", "NW", "NWW"};
  int idx;
  // Ensure Degrees is less than 360
  while(d >= 359){ d = d - 360; }
  idx = int(((float)d + 11.25)/22.5);
  if (idx>15){idx = 0;}
  return direction[idx]; 
}


// Roundup float to nearest Integer
// e.g. 1.2 = 1  or 1.5 = 2
int16_t roundup(float value){
    return int(value + 0.5);
}

// Convert EPOC time to Date & Time or Time
String cvtEPOCtoDateTime( unsigned long epoc, bool timeonly) {
// Adapted from https://www.geeksforgeeks.org/convert-unix-timestamp-to-dd-mm-yyyy-hhmmss-format/

	// Number of days in month in normal year
	int daysOfMonth[] = { 31, 28, 31, 30, 31, 30,
						31, 31, 30, 31, 30, 31 };

	long int currYear, daysTillNow, extraTime, extraDays,
		index, date, month, hours, minutes, seconds,
		flag = 0;

	// Calculate total days unix time T
	daysTillNow = epoc / (24 * 60 * 60);
	extraTime = epoc % (24 * 60 * 60);
	currYear = 1970;

	// Calculating current year
	while (true) {
		if (currYear % 400 == 0
			|| (currYear % 4 == 0 && currYear % 100 != 0)) {
			if (daysTillNow < 366) {
				break;
			}
			daysTillNow -= 366;
		}
		else {
			if (daysTillNow < 365) {
				break;
			}
			daysTillNow -= 365;
		}
		currYear += 1;
	}
	// Updating extra days because it will give days till previous day
	// and we have include current day
	extraDays = daysTillNow + 1;

	if (currYear % 400 == 0
		|| (currYear % 4 == 0 && currYear % 100 != 0))
		flag = 1;

	// Calculating MONTH and DATE
	month = 0, index = 0;
	if (flag == 1) {
		while (true) {

			if (index == 1) {
				if (extraDays - 29 < 0)
					break;
				month += 1;
				extraDays -= 29;
			}
			else {
				if (extraDays - daysOfMonth[index] < 0) {
					break;
				}
				month += 1;
				extraDays -= daysOfMonth[index];
			}
			index += 1;
		}
	}
	else {
		while (true) {

			if (extraDays - daysOfMonth[index] < 0) {
				break;
			}
			month += 1;
			extraDays -= daysOfMonth[index];
			index += 1;
		}
	}

	// Current Month
	if (extraDays > 0) {
		month += 1;
		date = extraDays;
	}
	else {
		if (month == 2 && flag == 1)
			date = 29;
		else {
			date = daysOfMonth[month - 1];
		}
	}

	// Calculating HH:MM:YYYY
	hours = extraTime / 3600;
	minutes = (extraTime % 3600) / 60;
	seconds = (extraTime % 3600) % 60;

	String ans = "";
	if (!timeonly) {
        ans += leadZeros(date,2);
	    ans += "/";
	    ans += leadZeros(month,2);
	    ans += "/";
	    ans += (String)currYear;
	    ans += " ";
    }
    ans += leadZeros(hours,2);
	ans += ":";
	ans += leadZeros(minutes, 2);
	ans += ":";
	ans += leadZeros(seconds, 2);

	// Return the time
	return ans;
}

