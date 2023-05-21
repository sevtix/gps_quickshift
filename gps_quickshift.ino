#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>


long LoggerLastTimestamp = 0L;  // ms
long LoggerFlushTimestamp = 0L;  // ms
int LoggerFlushInterval = 1000;  // ms;

// Define the number ranges
int range1_min = 1;
int range1_max = 10;

int rangeN_min = 90;
int rangeN_max = 100;

int range2_min = 15;
int range2_max = 25;

int range3_min = 30;
int range3_max = 40;

int range4_min = 45;
int range4_max = 55;

int range5_min = 60;
int range5_max = 70;

int range6_min = 75;
int range6_max = 85;

bool CUT = false;

File logfile;
Adafruit_ADS1115 ads1115;

void setup() {
  Serial.begin(115200);
  ads1115.begin();

  pinMode(13, OUTPUT); // LED
  pinMode(8, OUTPUT);

  // MOSFET Output
  pinMode(5, OUTPUT); 

  // see if the card is present and can be initialized:
  if (!SD.begin(4)) {
    Serial.println("Card init. failed!");
    error(2);
  }
  char filename[15];
  strcpy(filename, "/ANALOG00.CSV");
  for (uint8_t i = 0; i < 100; i++) {
    filename[7] = '0' + i/10;
    filename[8] = '0' + i%10;
    // create if does not exist, do not open existing, write, sync after write
    if (! SD.exists(filename)) {
      break;
    }
  }

  // open file in write mode
  logfile = SD.open(filename, FILE_WRITE);
  if (!logfile) {
    error(3);
  }

  // header line for csv files
  logfile.println("Time, RPM, ADC, Volts");
}

void loop() {
  long currentMillis = millis();
  
  // read ads1115 adc
  int16_t adc0 = ads1115.readADC_SingleEnded(0);

  // convert to volt
  float volts0 = ads1115.computeVolts(adc0);

  // Check if the ADC value is within any of the ranges
  if (adc0 >= range1_min && adc0 <= range1_max) {
    NoCut();
  } else if (adc0 >= range2_min && adc0 <= range2_max) {
    NoCut();
  } else if (adc0 >= range3_min && adc0 <= range3_max) {
    NoCut();
  } else if (adc0 >= range4_min && adc0 <= range4_max) {
    NoCut();
  } else if (adc0 >= range5_min && adc0 <= range5_max) {
    NoCut();
  } else if (adc0 >= range6_min && adc0 <= range6_max) {
    NoCut();
  } else if (adc0 >= range7_min && adc0 <= range7_max) {
    NoCut();
  } else {
    Cut();
  }

  // prevents multiple datapoints in a milisecond
  if (LoggerLastTimestamp <= currentMillis) {

    // green led on
    digitalWrite(8, HIGH);

    //float volts0 = 0;

    // log data
    logfile.print(currentMillis);
    logfile.print(",");
    logfile.print(rpm);
    logfile.print(",");
    logfile.print(adc0);
    logfile.print(",");
    logfile.println(volts0);

    // flush if next timestamp reached
    if (LoggerFlushTimestamp <= currentMillis) {
      // save log to file
      logfile.flush();
      // set next flush timestamp
      LoggerFlushTimestamp = millis() + LoggerFlushInterval;
    }

    // green led off
    digitalWrite(8, LOW);

    LoggerLastTimestamp = millis();
  }
}

void NoCut() {
  if(CUT) {
    digitalWrite(5, HIGH);
    CUT = false;
  }
}

void Cut() {
  if(!CUT) {
    digitalWrite(5, LOW);
    CUT = true;
  }
}

// blink out an error code
void error(uint8_t c) {
  while (true) {
    uint8_t i;
    for (i = 0; i < c; i++) {
      digitalWrite(13, HIGH);
      delay(100);
      digitalWrite(13, LOW);
      delay(100);
    }
    for (i = c; i < 10; i++) {
      delay(200);
    }
  }
}