#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SD.h>
#include <SPI.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET     -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme;
WebServer server(80);

const char* ssid = "saba";
const char* password = "saba1385";

const int chipSelect = 5; // پایه CS برای SD کارت (بسته به بردت ممکنه متفاوت باشه)

void setup() {
  Serial.begin(115200);

  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP address: ");
  Serial.println(WiFi.localIP());

  if (!bme.begin(0x76)) {
    Serial.println("BME280 not found!");
    while (1);
  }

  if (!SD.begin(chipSelect)) {
    Serial.println("Card Mount Failed");
    while (1);
  }
  Serial.println("SD Card initialized.");

  server.on("/", handleRoot);
  server.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.display();
  delay(1000);
}

void loop() {
  float temperature = bme.readTemperature();
  float pressure = bme.readPressure() / 100.0F;
  float altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
  float humidity = bme.readHumidity();

  // ذخیره داده ها در SD کارت
  File dataFile = SD.open("/data.csv", FILE_APPEND);
  if (dataFile) {
    dataFile.printf("%lu,%.2f,%.2f,%.2f,%.2f\n", millis(), temperature, humidity, pressure, altitude);
    dataFile.close();
  } else {
    Serial.println("Error opening data.csv");
  }

  server.handleClient();


  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.print("T:");
  display.print(temperature, 1);
  display.print("C  H:");
  display.print(humidity, 0);
  display.println("%");

  display.setCursor(0, 10);
  display.print("P:");
  display.print(pressure, 0);
  display.print("hPa A:");
  display.print(altitude, 0);
  display.println("m");
  display.display();
  delay(1000);
}

void handleRoot() {
  float temp = bme.readTemperature();
  float pressure = bme.readPressure() / 100.0F;
  float humidity = bme.readHumidity();
  float altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);

  String page = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <meta charset="UTF-8">
      <title>CIR1pro</title>
      <style>
        body { font-family: sans-serif; text-align: center; background-color: #f5f5f5; padding: 20px; }
        .card { background-color: white; border-radius: 10px; padding: 20px; box-shadow: 0 2px 8px rgba(0,0,0,0.2); display: inline-block; }
        h2 { margin-bottom: 20px; }
        p { font-size: 18px; margin: 5px 0; }
        button { padding: 10px 20px; font-size: 16px; margin-top: 15px; }
      </style>
    </head>
    <body>
      <div class="card">
        <h2>اطلاعات محیطی</h2>
  )rawliteral";

  page += "<p>🌡 دما: " + String(temp) + " °C</p>";
  page += "<p>💧 رطوبت: " + String(humidity) + " %</p>";
  page += "<p>🌬 فشار: " + String(pressure) + " hPa</p>";
  page += "<p>⛰ ارتفاع: " + String(altitude) + " m</p>";

  page += R"rawliteral(
        <form method="GET" action="/">
          <button type="submit">به‌روزرسانی دستی</button>
        </form>
      </div>
    </body>
    </html>
  )rawliteral";

  server.send(200, "text/html", page);
}
