/*
  Smart Vehicle Monitoring System
  Created by Mohammed Ghandour.

  AI tools were used for guidance, code review, debugging assistance,
  and explanations. The project was tested, understood, and adapted
  by the author.
*/
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>

const byte BUTTON_PIN = 2;
const byte GREEN_LED_PIN = 5;
const byte YELLOW_LED_PIN = 6;
const byte RED_LED_PIN = 7;
const byte BUZZER_PIN = 8;
const byte ULTRASONIC_TRIG_PIN = 9;
const byte ULTRASONIC_ECHO_PIN = 10;
const byte NTC_PIN = A0;
const byte FUEL_PIN = A1;

const unsigned long SENSOR_READ_INTERVAL_MS = 500;
const unsigned long LCD_REFRESH_INTERVAL_MS = 500;
const unsigned long SERIAL_DEBUG_INTERVAL_MS = 500;
const unsigned long DEBOUNCE_INTERVAL_MS = 35;
const unsigned long BUZZER_PULSE_INTERVAL_MS = 250;

LiquidCrystal_I2C lcd(0x27, 16, 2);

enum StatusLevel {
  STATUS_NORMAL = 0,
  STATUS_WARNING = 1,
  STATUS_DANGER = 2
};

void initializeSystem();
float readEngineTemperatureC();
int readFuelLevelPercent();
float readParkingDistanceCm();
void readSensorsIfDue();
StatusLevel evaluateEngineStatus(float temperatureC);
StatusLevel evaluateFuelStatus(int fuelPercent);
StatusLevel evaluateParkingStatus(float distanceCm);
StatusLevel evaluateOverallStatus();
void updateOutputs();
void handleBuzzerPattern();
void handleButton();
const char *statusLabel(StatusLevel status);
void printLcdLine(byte row, const char *text);
void updateLcdIfDue();
void printSerialDebugIfDue();

float engineTemperatureC = 0.0;
int fuelLevelPercent = 0;
float parkingDistanceCm = 0.0;

StatusLevel engineStatus = STATUS_NORMAL;
StatusLevel fuelStatus = STATUS_NORMAL;
StatusLevel parkingStatus = STATUS_NORMAL;
StatusLevel overallStatus = STATUS_NORMAL;

byte currentPage = 0;
bool lcdNeedsRefresh = true;

bool lastButtonReading = HIGH;
bool debouncedButtonState = HIGH;
unsigned long lastButtonChangeMs = 0;

bool buzzerToneActive = false;
unsigned long lastBuzzerToggleMs = 0;

unsigned long lastSensorReadMs = 0;
unsigned long lastLcdRefreshMs = 0;
unsigned long lastSerialDebugMs = 0;

// Configures pins, Serial, and LCD startup state.
void initializeSystem() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);

  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(YELLOW_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);

  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Smart Vehicle");
  lcd.setCursor(0, 1);
  lcd.print("Monitor Ready");
  delay(1200);
  lcd.clear();
}

// Converts the NTC analog value to Celsius using the Beta equation.
float readEngineTemperatureC() {
  int analogValue = analogRead(NTC_PIN);
  analogValue = constrain(analogValue, 1, 1022);
  float celsius = 1.0 / (log(1.0 / (1023.0 / analogValue - 1.0)) / 3950.0 + 1.0 / 298.15) - 273.15;
  return celsius;
}

// Reads the fuel potentiometer as a 0-100 percent level.
int readFuelLevelPercent() {
  int analogValue = analogRead(FUEL_PIN);
  return map(analogValue, 0, 1023, 0, 100);
}

// Measures parking distance from the HC-SR04 in centimeters.
float readParkingDistanceCm() {
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);

  unsigned long durationUs = pulseIn(ULTRASONIC_ECHO_PIN, HIGH, 25000UL);
  if (durationUs == 0) {
    return 999.0;
  }

  return durationUs / 58.0;
}

// Reads all sensors on a fixed millis() interval.
void readSensorsIfDue() {
  unsigned long now = millis();
  if (now - lastSensorReadMs < SENSOR_READ_INTERVAL_MS) {
    return;
  }

  lastSensorReadMs = now;
  engineTemperatureC = readEngineTemperatureC();
  fuelLevelPercent = readFuelLevelPercent();
  parkingDistanceCm = readParkingDistanceCm();

  engineStatus = evaluateEngineStatus(engineTemperatureC);
  fuelStatus = evaluateFuelStatus(fuelLevelPercent);
  parkingStatus = evaluateParkingStatus(parkingDistanceCm);
  overallStatus = evaluateOverallStatus();

  lcdNeedsRefresh = true;
  printSerialDebugIfDue();
}

// Evaluates engine temperature status thresholds.
StatusLevel evaluateEngineStatus(float temperatureC) {
  if (temperatureC >= 75.0) {
    return STATUS_DANGER;
  }
  if (temperatureC >= 60.0) {
    return STATUS_WARNING;
  }
  return STATUS_NORMAL;
}

// Evaluates fuel level status thresholds.
StatusLevel evaluateFuelStatus(int fuelPercent) {
  if (fuelPercent <= 15) {
    return STATUS_DANGER;
  }
  if (fuelPercent <= 30) {
    return STATUS_WARNING;
  }
  return STATUS_NORMAL;
}

// Evaluates parking distance status thresholds.
StatusLevel evaluateParkingStatus(float distanceCm) {
  if (distanceCm <= 15.0) {
    return STATUS_DANGER;
  }
  if (distanceCm <= 25.0) {
    return STATUS_WARNING;
  }
  return STATUS_NORMAL;
}

// Chooses the worst status from temperature, fuel, and parking.
StatusLevel evaluateOverallStatus() {
  StatusLevel worstStatus = engineStatus;
  if (fuelStatus > worstStatus) {
    worstStatus = fuelStatus;
  }
  if (parkingStatus > worstStatus) {
    worstStatus = parkingStatus;
  }
  return worstStatus;
}

// Updates status LEDs from the overall system status.
void updateOutputs() {
  digitalWrite(GREEN_LED_PIN, overallStatus == STATUS_NORMAL ? HIGH : LOW);
  digitalWrite(YELLOW_LED_PIN, overallStatus == STATUS_WARNING ? HIGH : LOW);
  digitalWrite(RED_LED_PIN, overallStatus == STATUS_DANGER ? HIGH : LOW);
}

// Runs a non-blocking pulsed buzzer pattern during danger state.
void handleBuzzerPattern() {
  unsigned long now = millis();

  if (overallStatus != STATUS_DANGER) {
    if (buzzerToneActive) {
      noTone(BUZZER_PIN);
      buzzerToneActive = false;
    }
    lastBuzzerToggleMs = now;
    return;
  }

  if (now - lastBuzzerToggleMs >= BUZZER_PULSE_INTERVAL_MS) {
    lastBuzzerToggleMs = now;
    buzzerToneActive = !buzzerToneActive;
    if (buzzerToneActive) {
      tone(BUZZER_PIN, 2200);
    } else {
      noTone(BUZZER_PIN);
    }
  }
}

// Debounces the button and advances the LCD page on each short press.
void handleButton() {
  bool reading = digitalRead(BUTTON_PIN);
  unsigned long now = millis();

  if (reading != lastButtonReading) {
    lastButtonChangeMs = now;
    lastButtonReading = reading;
  }

  if ((now - lastButtonChangeMs) >= DEBOUNCE_INTERVAL_MS && reading != debouncedButtonState) {
    debouncedButtonState = reading;
    if (debouncedButtonState == LOW) {
      currentPage = (currentPage + 1) % 4;
      lcdNeedsRefresh = true;
    }
  }
}

// Returns a readable label for a status value.
const char *statusLabel(StatusLevel status) {
  switch (status) {
    case STATUS_DANGER:
      return "DANGER";
    case STATUS_WARNING:
      return "WARNING";
    default:
      return "NORMAL";
  }
}

// Prints a fixed-width line to the LCD to avoid stale characters.
void printLcdLine(byte row, const char *text) {
  lcd.setCursor(0, row);
  byte index = 0;
  while (index < 16 && text[index] != '\0') {
    lcd.print(text[index]);
    index++;
  }
  while (index < 16) {
    lcd.print(' ');
    index++;
  }
}

// Updates the active LCD page when values or page selection change.
void updateLcdIfDue() {
  unsigned long now = millis();
  if (!lcdNeedsRefresh && now - lastLcdRefreshMs < LCD_REFRESH_INTERVAL_MS) {
    return;
  }

  lastLcdRefreshMs = now;
  lcdNeedsRefresh = false;

  char line[17];
  char valueText[8];

  switch (currentPage) {
    case 0:
      printLcdLine(0, "Engine Temp");
      dtostrf(engineTemperatureC, 5, 1, valueText);
      snprintf(line, sizeof(line), "%sC %-7s", valueText, statusLabel(engineStatus));
      printLcdLine(1, line);
      break;
    case 1:
      printLcdLine(0, "Fuel Level");
      snprintf(line, sizeof(line), "%3d%% %-9s", fuelLevelPercent, statusLabel(fuelStatus));
      printLcdLine(1, line);
      break;
    case 2:
      printLcdLine(0, "Park Distance");
      dtostrf(parkingDistanceCm, 5, 1, valueText);
      snprintf(line, sizeof(line), "%scm %-5s", valueText, statusLabel(parkingStatus));
      printLcdLine(1, line);
      break;
    default:
      printLcdLine(0, "System Status");
      snprintf(line, sizeof(line), "%-16s", statusLabel(overallStatus));
      printLcdLine(1, line);
      break;
  }
}

// Prints diagnostic sensor and status values to Serial Monitor.
void printSerialDebugIfDue() {
  unsigned long now = millis();
  if (now - lastSerialDebugMs < SERIAL_DEBUG_INTERVAL_MS) {
    return;
  }

  lastSerialDebugMs = now;
  Serial.print("TempC=");
  Serial.print(engineTemperatureC, 1);
  Serial.print(" (");
  Serial.print(statusLabel(engineStatus));
  Serial.print("), Fuel=");
  Serial.print(fuelLevelPercent);
  Serial.print("% (");
  Serial.print(statusLabel(fuelStatus));
  Serial.print("), Distance=");
  Serial.print(parkingDistanceCm, 1);
  Serial.print("cm (");
  Serial.print(statusLabel(parkingStatus));
  Serial.print("), Overall=");
  Serial.println(statusLabel(overallStatus));
}

void setup() {
  initializeSystem();
  readSensorsIfDue();
  lcdNeedsRefresh = true;
}

void loop() {
  handleButton();
  readSensorsIfDue();
  updateOutputs();
  handleBuzzerPattern();
  updateLcdIfDue();
}
