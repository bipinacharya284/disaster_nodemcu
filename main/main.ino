#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// Replace with your network credentials
const char* ssid = "UNREAL";
const char* password = "12345678"; // Your Wi-Fi Password

WiFiClient wifiClient; // Create a WiFiClient instance
const char* serverUrl = "http://192.168.7.80:8080/api/sensor-data";

// Define pins for ultrasonic sensor
const int trigPin = D7; // Trigger pin
const int echoPin = D8; // Echo pin

void setup() {
  Serial.begin(115200);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Setup ultrasonic sensor pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Optionally read from potentiometer at startup
  postSensorData();
}

void loop() {
  delay(10); // Send data every 1 second
  postSensorData();
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

    // Read ultrasonic sensor value
    float distance = getUltrasonicDistance();

    // Print values for debugging
    Serial.print("Potentiometer Value (Raw): ");
    Serial.println(potValue);
    Serial.print("Potentiometer Voltage: ");
    Serial.println(voltage, 3);
    Serial.print("Ultrasonic Distance: ");
    Serial.println(distance, 2);

    // Prepare your JSON data with the sensor values
    String jsonData = String("{\"device_id\":\"1\",\"sensor_data\":[") +
                      String("{\"sensor_id\":\"552\",\"value\":") + String(voltage, 3) + String("},") +
                      String("{\"sensor_id\":\"551\",\"value\":") + String(distance, 2) + String("}") +
                      String("],\"sent_at\":\"") + getCurrentTimestamp() + String("\"}");

    // Print the JSON data being sent
    Serial.print("Sending JSON Data: ");
    Serial.println(jsonData);

    // Send the request
    int httpResponseCode = http.POST(jsonData);

    // Check for the response
    if (httpResponseCode > 0) {
      String response = http.getString(); // Get the response
      Serial.print("HTTP Response Code: ");
      Serial.println(httpResponseCode);   // Print HTTP return code
      Serial.print("Response: ");
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
          Serial.print("Redirected HTTP Response Code: ");
          Serial.println(httpResponseCode);
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

// Function to get the distance from the ultrasonic sensor
float getUltrasonicDistance() {
  long duration, distance;
  
  // Clear the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  // Trigger the ultrasonic pulse
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Read the echoPin, return the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance (in cm)
  distance = (duration * 0.034) / 2; // Sound travels at 0.034 cm/µs
  return (float)distance;
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
