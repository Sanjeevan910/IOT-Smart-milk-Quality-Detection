#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

const char *ssid = "Wokwi-GUEST";
const char *password = "";
const char *tagoToken = "ab31526a-2308-4809-8b64-45de2ebf963b";

// --- MILK QUALITY THRESHOLDS ---
const float MIN_PH = 6.4;
const float MAX_PH = 6.8;
const float MAX_TEMP = 4.0;
const float MIN_TDS = 4000.0;
const float MAX_TDS = 8000.0;
const int MIN_TURBIDITY = 80;

// --- TIMERS & STATE CONTROL ---
unsigned long lastTagoUpdate = 0;
unsigned long phaseStartTime = 0;
bool isNormalPhase = true; // This toggle controls the demo state

void setup()
{
  Serial.begin(9600);

  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  // Start the phase timer
  phaseStartTime = millis();
}

void loop()
{
  // Flip the phase every 30 seconds (30000 milliseconds)
  if (millis() - phaseStartTime > 30000)
  {
    isNormalPhase = !isNormalPhase; // Toggle the state
    phaseStartTime = millis();      // Reset the stopwatch
    Serial.println("\n=========================================");
    Serial.println(">>> AUTOMATIC SWITCHING DEMO PHASE <<<");
    Serial.println("=========================================\n");
  }

  // Variables to hold our simulated data
  float tempC, ph, tdsValue;
  int turbidity;
  String milkStatus, reason;

  // --- THE BRAINS: INJECTING DATA BASED ON PHASE ---
  if (isNormalPhase)
  {
    // PHASE 1: NORMAL MILK (Safe values)
    tempC = 2.5;
    ph = 6.6;
    tdsValue = 6000.0;
    turbidity = 90;
    milkStatus = "Good";
    reason = "All parameters normal";
  }
  else
  {
    // PHASE 2: ADULTERATED MILK (Unsafe values)
    tempC = 5.5;       // High temperature (Spoilage risk)
    ph = 5.8;          // High acidity (Sour milk)
    tdsValue = 2500.0; // Low TDS (Diluted with water)
    turbidity = 40;    // Low turbidity (Watered down)
    milkStatus = "Bad";
    reason = "Multiple anomalies (Dilution & Temp)";
  }

  // Print what is happening to the VS Code Serial Monitor
  Serial.print("Current Mode: ");
  Serial.println(isNormalPhase ? "NORMAL (Healthy Milk)" : "ANOMALY (Spoiled/Diluted)");
  Serial.print("Temp: ");
  Serial.print(tempC);
  Serial.println(" C");
  Serial.print("pH: ");
  Serial.println(ph);
  Serial.print("TDS: ");
  Serial.print(tdsValue);
  Serial.println(" ppm");
  Serial.print("Turbidity: ");
  Serial.println(turbidity);
  Serial.print("STATUS: ");
  Serial.print(milkStatus);
  Serial.print(" (");
  Serial.print(reason);
  Serial.println(")");
  Serial.println("---------------------------");

  // Send to TagoIO every 10 seconds
  if (millis() - lastTagoUpdate > 10000)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      HTTPClient http;
      http.begin("https://api.tago.io/data");
      http.addHeader("Content-Type", "application/json");
      http.addHeader("Device-Token", tagoToken);

      String payload = "[";
      payload += "{\"variable\":\"temperature\",\"value\":" + String(tempC) + "},";
      payload += "{\"variable\":\"ph\",\"value\":" + String(ph) + "},";
      payload += "{\"variable\":\"tds\",\"value\":" + String(tdsValue) + "},";
      payload += "{\"variable\":\"turbidity\",\"value\":" + String(turbidity) + "},";
      payload += "{\"variable\":\"status\",\"value\":\"" + milkStatus + "\"},";
      payload += "{\"variable\":\"reason\",\"value\":\"" + reason + "\"}";
      payload += "]";

      int httpResponseCode = http.POST(payload);
      if (httpResponseCode > 0)
      {
        Serial.println(">>> DATA UPLOADED TO TAGO IO <<<");
      }
      http.end();
    }
    lastTagoUpdate = millis();
  }

  delay(1000);
}