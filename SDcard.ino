#include <Wire.h>
#include <Adafruit_BME280.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define SEALEVELPRESSURE_HPA (1013.25)
#define SD_CS 5  // پایه CS

Adafruit_BME280 bme;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // مقداردهی به سنسور BME280
  if (!bme.begin(0x76)) {
    Serial.println("❌ BME280 not found!");
    while (1);
  }

  // مقداردهی به SD کارت
  SPI.begin(18, 19, 23, SD_CS);
  if (!SD.begin(SD_CS, SPI, 8000000)) {
    Serial.println("❌ SD Card Mount Failed!");
    while (1);
  }

  // نوشتن عنوان ستون‌ها در فایل CSV (فقط یک بار اگر فایل نبود)
  if (!SD.exists("/data.csv")) {
    File file = SD.open("/data.csv", FILE_WRITE);
    if (file) {
      file.println("Time(ms),Temperature(C),Humidity(%),Pressure(hPa),Altitude(m)");
      file.close();
      Serial.println("✅ CSV header written.");
    }
  }

  Serial.println("✅ Setup complete.");
}

void loop() {
  float temp = bme.readTemperature();
  float humidity = bme.readHumidity();
  float pressure = bme.readPressure() / 100.0F;
  float altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);

  // نوشتن در فایل CSV
  File file = SD.open("/data.csv", FILE_APPEND);
  if (file) {
    file.printf("%lu,%.2f,%.2f,%.2f,%.2f\n", millis(), temp, humidity, pressure, altitude);
    file.close();
    Serial.println("✅ Data written to SD:");
    Serial.printf("Temp: %.2f°C | Humidity: %.2f%% | Pressure: %.2f hPa | Alt: %.2f m\n", temp, humidity, pressure, altitude);
  } else {
    Serial.println("❌ Failed to open data.csv");
  }

  delay(5000);  // هر ۵ ثانیه یک بار ذخیره شود
}
