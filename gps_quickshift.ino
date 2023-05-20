#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>


long LoggerLastTimestamp = 0L;  // ms
long LoggerFlushTimestamp = 0L;  // ms
int LoggerFlushInterval = 1000;  // ms;

volatile unsigned long InjectionPreviousTime = 0;
volatile unsigned long InjectionTimeDifference = 0;
const int InjectionPulsesPerRevolution = 1;  // Change this value according to your motorcycle's injector configuration
const unsigned long overflowLimit = 4294967295UL;  // Micros() overflow limit
float rpm = 0;

File logfile;
Adafruit_ADS1115 ads1115;

void setup() {
  Serial.begin(115200);
  ads1115.begin();

  pinMode(13, OUTPUT); // LED
  pinMode(8, OUTPUT);

  // Injector Capture
  pinMode(5, INPUT_PULLUP); 
  attachInterrupt(digitalPinToInterrupt(5), injectStart, FALLING);


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

  // prevents multiple datapoints in a milisecond
  if (LoggerLastTimestamp <= currentMillis) {

    // green led on
    digitalWrite(8, HIGH);

    // read ads1115 adc
    int16_t adc0 = ads1115.readADC_SingleEnded(0);

    // convert to volt
    float volts0 = ads1115.computeVolts(adc0);
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

void injectStart() {
  volatile unsigned long currentTime = micros();
  Serial.println("inject event");
  
  if (currentTime >= InjectionPreviousTime) {
    InjectionTimeDifference = currentTime - InjectionPreviousTime;
  } else {
    // Handle overflow
    InjectionTimeDifference = (overflowLimit - InjectionPreviousTime) + currentTime;
  }
  
  InjectionPreviousTime = currentTime;

  // Calculate RPM
  float InjectionTimeDifferenceInSeconds = InjectionTimeDifference / 1000000.0;  // Convert to seconds
  rpm = 60.0 / (InjectionTimeDifferenceInSeconds * InjectionPulsesPerRevolution);
  Serial.println(rpm);
}