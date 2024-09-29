#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// Replace with your network credentials
const char* ssid = "UNREAL";
const char* password = "12345678"; // Your Wi-Fi Password

WiFiClient wifiClient; // Create a WiFiClient instance
const char* serverUrl = "http://192.168.7.80:8080/api/sensor-data";

void setup() {
  Serial.begin(115200);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Optionally read from potentiometer at startup
  postSensorData();
}

void loop() {
  delay(1000); // Send data every 10 seconds
  postSensorData();
}

void getSensorData() {
  if (WiFi.status() == WL_CONNECTED) { // Check WiFi connection
    HTTPClient http;
    String url = String(serverUrl) + "/1"; // Use String for concatenation
    http.begin(wifiClient, url); // Specify the URL with WiFiClient

    int httpCode = http.GET(); // Make the request
    if (httpCode > 0) {
      String payload = http.getString(); // Get the response payload
      Serial.println("GET Response: ");
      Serial.println(payload); // Print the response
    } else {
      Serial.printf("GET request failed, error: %s, HTTP Code: %d\n", http.errorToString(httpCode).c_str(), httpCode);
      Serial.printf("Connection Status: %s\n", WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
    }
    http.end(); // Free resources
  } else {
    Serial.println("WiFi not connected. Cannot make GET request.");
  }
}

void postSensorData() {
  
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client; // Create a WiFiClient object
    HTTPClient http;    // Create an HTTPClient object

    // Specify the URL
    http.begin(client, serverUrl); // Use the WiFiClient with the begin method
    
    // Specify content-type header
    http.addHeader("Content-Type", "application/json");

    // Read potentiometer value from A0
    int potValue = analogRead(A0); // Read value from potentiometer
    float voltage = potValue * (3.3 / 1023.0); // Convert to voltage (0-3.3V)
    
    // Print the raw potentiometer value and the calculated voltage
    Serial.print("Potentiometer Value (Raw): ");
    Serial.println(potValue);
    Serial.print("Potentiometer Voltage: ");
    Serial.println(voltage, 3); // Print voltage with 3 decimal places

    // Prepare your JSON data with the potentiometer value
    String jsonData = String("{\"device_id\":\"1\",\"sensor_data\":[{\"sensor_id\":\"501\",\"value\":") + String(voltage, 3) + String("}],\"sent_at\":\"") + getCurrentTimestamp() + String("\"}");

    // Send the request
    int httpResponseCode = http.POST(jsonData);

    // Check for the response
    if (httpResponseCode > 0) {
      String response = http.getString(); // Get the response
      Serial.println(httpResponseCode);   // Print HTTP return code
      Serial.println(response);            // Print the response

      // Check for redirect
      if (httpResponseCode == 307) {
          String redirectUrl = http.getLocation();
          Serial.print("Redirected to: ");
          Serial.println(redirectUrl);
          
          // Follow the redirect URL
          http.begin(client, redirectUrl);
          http.addHeader("Content-Type", "application/json");
          httpResponseCode = http.POST(jsonData); // Send to redirected URL
      }
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }

    // Close connection
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}

// Function to get current timestamp in ISO 8601 format
String getCurrentTimestamp() {
  String timestamp = "";
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  char buffer[30];
  strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", timeinfo);
  timestamp = String(buffer) + ".841Z"; // Adjust for your desired format
  return timestamp;
}
