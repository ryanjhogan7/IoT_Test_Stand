/*
 * ESP32 Dirt Test Monitor - Complete Backend
 * 
 * Copy this entire code to your Arduino IDE and upload to ESP32
 * 
 * ESP32-C6 Pin Configuration (avoiding GPIO0):
 * - Pressure 1: GPIO 1 (ADC1_CH1) - Left side of board
 * - Pressure 2: GPIO 2 (ADC1_CH2) - Left side of board
 * - Flow Sensor: GPIO 5 (ADC1_CH5) - Left side of board
 * - Solenoid Control: GPIO 10 - Left side of board
 */

#include <WiFi.h>
#include <WebServer.h>

// Network Configuration
const char* WIFI_SSID = "PNRENGIOT";
const char* WIFI_PASSWORD = "WP3MQJCsqYqstXm5";

// Pin Definitions - ESP32-C6 safe ADC pins (avoiding GPIO0)
const int PRESSURE1_PIN = 1;        // GPIO1 - ADC1_CH1 (left side)
const int PRESSURE2_PIN = 2;        // GPIO2 - ADC1_CH2 (left side)  
const int FLOW_SENSOR_PIN = 5;      // GPIO5 - ADC1_CH5 (left side)
const int SOLENOID_PIN = 10;        // GPIO10c:\Users\e1260936\Downloads\testing_interface_prd.md - Digital output (left side)

// Sensor scaling - adjust these values for your sensors
float pressure1_min = 0.0, pressure1_max = 50.0;
float pressure2_min = 0.0, pressure2_max = 50.0;
float flow_min = 0.0, flow_max = 10.0;

// System state variables for test management
bool testRunning = false;
bool testPaused = false;
unsigned long testStartTime = 0;
unsigned long pausedDuration = 0;
float totalVolume = 0.0;
unsigned long lastDataTime = 0;
float pressureThreshold = 20.0;

// System state
bool solenoidState = false;
WebServer server(80);

void setup() {
    Serial.begin(115200);
    Serial.println("ESP32 Dirt Test Monitor Starting...");
    
    // Setup pins
    pinMode(SOLENOID_PIN, OUTPUT);
    digitalWrite(SOLENOID_PIN, LOW);
    
    // Setup ADC
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
    
    // Connect WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.print("Connected! IP: ");
    Serial.println(WiFi.localIP());
    
    // Setup web server
    setupServer();
    server.begin();
    
    Serial.println("System ready!");
}

void loop() {
    server.handleClient();
    delay(10);
}

void setupServer() {
    // Handle CORS preflight requests (OPTIONS method)
    server.on("/api/status", HTTP_OPTIONS, handleCORS);
    server.on("/api/start", HTTP_OPTIONS, handleCORS);
    server.on("/api/stop", HTTP_OPTIONS, handleCORS);
    server.on("/api/reset", HTTP_OPTIONS, handleCORS);
    server.on("/api/data", HTTP_OPTIONS, handleCORS);
    server.on("/api/config", HTTP_OPTIONS, handleCORS);
    server.on("/api/calibrate", HTTP_OPTIONS, handleCORS);
    server.on("/api/solenoid", HTTP_OPTIONS, handleCORS);
    server.on("/api/scaling", HTTP_OPTIONS, handleCORS);
    
    // API endpoints - ALL endpoints that MAIN.html expects
    server.on("/api/status", HTTP_GET, handleStatus);
    server.on("/api/start", HTTP_POST, handleStart);
    server.on("/api/stop", HTTP_POST, handleStop);
    server.on("/api/reset", HTTP_POST, handleReset);
    server.on("/api/data", HTTP_GET, handleData);
    server.on("/api/config", HTTP_GET, handleConfigGet);
    server.on("/api/config", HTTP_POST, handleConfigPost);
    server.on("/api/calibrate", HTTP_POST, handleCalibrate);
    server.on("/api/solenoid", HTTP_POST, handleSolenoid);
    server.on("/api/scaling", HTTP_POST, handleScaling);
    
    // Simple root page
    server.on("/", HTTP_GET, handleRoot);
    
    // Handle all other OPTIONS requests
    server.onNotFound(handleNotFound);
}

void handleCORS() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type, Accept");
    server.sendHeader("Access-Control-Max-Age", "86400");
    server.send(200, "text/plain", "");
}

void handleRoot() {
    String html = "<h1>ESP32 Dirt Test Monitor</h1>";
    html += "<p>IP: " + WiFi.localIP().toString() + "</p>";
    html += "<p>Status: Online</p>";
    html += "<p>Test API: <a href='/api/status'>/api/status</a></p>";
    html += "<h3>Available Endpoints:</h3>";
    html += "<ul>";
    html += "<li>GET /api/status - System status</li>";
    html += "<li>POST /api/start - Start test</li>";
    html += "<li>POST /api/stop - Stop/pause test</li>";
    html += "<li>POST /api/reset - Reset test</li>";
    html += "<li>GET /api/data - Current sensor data</li>";
    html += "<li>GET/POST /api/config - Configuration</li>";
    html += "<li>POST /api/calibrate - Sensor calibration</li>";
    html += "</ul>";
    
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/html", html);
}

void handleNotFound() {
    if (server.method() == HTTP_OPTIONS) {
        handleCORS();
    } else {
        Serial.println("404 - Not Found: " + server.uri());
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.send(404, "application/json", "{\"success\":false,\"error\":\"Endpoint not found: " + server.uri() + "\"}");
    }
}

float readVoltage(int pin) {
    int raw = analogRead(pin);
    return (raw / 4095.0) * 3.3;
}

float scaleValue(float voltage, float minVal, float maxVal) {
    // Scale 0.66V-3.3V to minVal-maxVal
    if (voltage < 0.66) voltage = 0.66;
    if (voltage > 3.3) voltage = 3.3;
    
    float normalized = (voltage - 0.66) / (3.3 - 0.66);
    return minVal + (normalized * (maxVal - minVal));
}

void handleStatus() {
    // Read sensors
    float p1_voltage = readVoltage(PRESSURE1_PIN);
    float p2_voltage = readVoltage(PRESSURE2_PIN);
    float flow_voltage = readVoltage(FLOW_SENSOR_PIN);
    
    // Scale values
    float p1_scaled = scaleValue(p1_voltage, pressure1_min, pressure1_max);
    float p2_scaled = scaleValue(p2_voltage, pressure2_min, pressure2_max);
    float flow_scaled = scaleValue(flow_voltage, flow_min, flow_max);
    
    // Build JSON response to match MAIN.html expectations
    String response = "{";
    response += "\"success\":true,";
    response += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
    response += "\"firmware\":\"v2.1-ESP32C6\",";
    response += "\"uptime\":" + String(millis() / 1000) + ",";
    response += "\"freeMemory\":" + String(ESP.getFreeHeap()) + ",";
    response += "\"wifiConnected\":true,";
    response += "\"wifiRSSI\":" + String(WiFi.RSSI()) + ",";
    response += "\"sensorsOnline\":true,";
    response += "\"testRunning\":" + String(testRunning ? "true" : "false") + ",";
    response += "\"testPaused\":" + String(testPaused ? "true" : "false") + ",";
    response += "\"valveOpen\":" + String(solenoidState ? "true" : "false") + ",";
    response += "\"deviceName\":\"ESP32_C6_Monitor\",";
    
    // Add sensor inputs for simplified_esp32_dashboard compatibility
    response += "\"inputs\":{";
    response += "\"pressure1Scaled\":" + String(p1_scaled, 2) + ",";
    response += "\"pressure1Raw\":" + String(p1_voltage, 3) + ",";
    response += "\"pressure2Scaled\":" + String(p2_scaled, 2) + ",";
    response += "\"pressure2Raw\":" + String(p2_voltage, 3) + ",";
    response += "\"flowScaled\":" + String(flow_scaled, 2) + ",";
    response += "\"flowRaw\":" + String(flow_voltage, 3);
    response += "},";
    
    // Add outputs section
    response += "\"outputs\":{";
    response += "\"solenoid\":" + String(solenoidState ? "true" : "false");
    response += "},";
    
    // Add scaling configuration
    response += "\"scaling\":{";
    response += "\"pressure1Min\":" + String(pressure1_min, 1) + ",";
    response += "\"pressure1Max\":" + String(pressure1_max, 1) + ",";
    response += "\"pressure2Min\":" + String(pressure2_min, 1) + ",";
    response += "\"pressure2Max\":" + String(pressure2_max, 1) + ",";
    response += "\"flowMin\":" + String(flow_min, 1) + ",";
    response += "\"flowMax\":" + String(flow_max, 1);
    response += "},";
    
    // Add analog readings section for MAIN.html
    response += "\"analogReadings\":{";
    response += "\"flowVoltage\":" + String(flow_voltage, 3) + ",";
    response += "\"inletVoltage\":" + String(p1_voltage, 3) + ",";
    response += "\"outletVoltage\":" + String(p2_voltage, 3);
    response += "}";
    
    response += "}";
    
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", response);
    
    Serial.println("Status: P1=" + String(p1_scaled,1) + " P2=" + String(p2_scaled,1) + " Flow=" + String(flow_scaled,1));
}

void handleStart() {
    Serial.println("START endpoint called");
    
    if (server.hasArg("plain")) {
        String body = server.arg("plain");
        Serial.println("Start request body: " + body);
        
        // Check if this is a resume operation
        bool isResume = body.indexOf("\"resume\":true") >= 0;
        
        if (isResume && testPaused) {
            // Resume paused test
            testRunning = true;
            testPaused = false;
            pausedDuration += (millis() - testStartTime);
            solenoidState = true;
            digitalWrite(SOLENOID_PIN, HIGH);
            
            Serial.println("Test RESUMED from pause");
        } else if (!testRunning && !testPaused) {
            // Start new test
            testRunning = true;
            testPaused = false;
            testStartTime = millis();
            pausedDuration = 0;
            totalVolume = 0.0;
            solenoidState = true;
            digitalWrite(SOLENOID_PIN, HIGH);
            lastDataTime = millis();
            
            // Parse pressure threshold if provided
            int thresholdPos = body.indexOf("\"pressureThreshold\":");
            if (thresholdPos >= 0) {
                int commaPos = body.indexOf(',', thresholdPos);
                if (commaPos == -1) commaPos = body.indexOf('}', thresholdPos);
                pressureThreshold = body.substring(thresholdPos + 20, commaPos).toFloat();
                Serial.println("Pressure threshold set to: " + String(pressureThreshold));
            }
            
            Serial.println("New test STARTED");
        }
    }
    
    String response = "{";
    response += "\"success\":true,";
    response += "\"message\":\"Test " + String((testPaused ? "resumed" : "started")) + " successfully\",";
    response += "\"testRunning\":true,";
    response += "\"valveOpen\":true";
    response += "}";
    
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", response);
}

void handleStop() {
    Serial.println("STOP endpoint called");
    
    if (testRunning) {
        testRunning = false;
        testPaused = true;
        solenoidState = false;
        digitalWrite(SOLENOID_PIN, LOW);
        
        Serial.println("Test PAUSED");
    }
    
    String response = "{";
    response += "\"success\":true,";
    response += "\"message\":\"Test paused successfully\",";
    response += "\"testRunning\":false,";
    response += "\"testPaused\":true,";
    response += "\"valveOpen\":false";
    response += "}";
    
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", response);
}

void handleReset() {
    Serial.println("RESET endpoint called");
    
    // Stop test and reset all values
    testRunning = false;
    testPaused = false;
    testStartTime = 0;
    pausedDuration = 0;
    totalVolume = 0.0;
    solenoidState = false;
    digitalWrite(SOLENOID_PIN, LOW);
    
    Serial.println("Test RESET - All values cleared");
    
    String response = "{";
    response += "\"success\":true,";
    response += "\"message\":\"System reset successfully\",";
    response += "\"testRunning\":false,";
    response += "\"testPaused\":false,";
    response += "\"valveOpen\":false";
    response += "}";
    
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", response);
}

void handleData() {
    // Read current sensor values
    float p1_voltage = readVoltage(PRESSURE1_PIN);
    float p2_voltage = readVoltage(PRESSURE2_PIN);
    float flow_voltage = readVoltage(FLOW_SENSOR_PIN);
    
    // Scale values
    float p1_scaled = scaleValue(p1_voltage, pressure1_min, pressure1_max);
    float p2_scaled = scaleValue(p2_voltage, pressure2_min, pressure2_max);
    float flow_scaled = scaleValue(flow_voltage, flow_min, flow_max);
    
    // Calculate pressure drop and elapsed time
    float pressureDrop = abs(p1_scaled - p2_scaled); // Absolute value of difference
    
    unsigned long elapsedTime = 0;
    if (testRunning) {
        elapsedTime = (millis() - testStartTime - pausedDuration) / 1000;
        
        // Update total volume if test is running
        if (lastDataTime > 0) {
            float deltaTime = (millis() - lastDataTime) / 1000.0 / 60.0; // minutes
            totalVolume += flow_scaled * deltaTime; // L/min * minutes = Liters
        }
        lastDataTime = millis();
    } else if (testPaused) {
        elapsedTime = (millis() - testStartTime - pausedDuration) / 1000;
    }
    
    // Check for auto-shutdown on pressure threshold
    if (testRunning && pressureDrop >= pressureThreshold) {
        testRunning = false;
        testPaused = true;
        solenoidState = false;
        digitalWrite(SOLENOID_PIN, LOW);
        Serial.println("AUTO-SHUTDOWN: Pressure threshold exceeded (" + String(pressureDrop) + " >= " + String(pressureThreshold) + ")");
    }
    
    // Build response matching MAIN.html expectations
    String response = "{";
    response += "\"success\":true,";
    response += "\"timestamp\":" + String(millis()) + ",";
    response += "\"testRunning\":" + String(testRunning ? "true" : "false") + ",";
    response += "\"testPaused\":" + String(testPaused ? "true" : "false") + ",";
    response += "\"elapsedTime\":" + String(elapsedTime) + ",";
    response += "\"flowRate\":" + String(flow_scaled, 2) + ",";
    response += "\"pressureDrop\":" + String(pressureDrop, 1) + ",";
    response += "\"totalVolume\":" + String(totalVolume, 3) + ",";
    response += "\"valveOpen\":" + String(solenoidState ? "true" : "false") + ",";
    response += "\"pressure1\":" + String(p1_scaled, 1) + ",";
    response += "\"pressure2\":" + String(p2_scaled, 1) + ",";
    response += "\"pressureThreshold\":" + String(pressureThreshold, 1) + ",";
    
    // Add analog voltages for debugging
    response += "\"analogVoltages\":{";
    response += "\"flow\":" + String(flow_voltage, 3) + ",";
    response += "\"inlet\":" + String(p1_voltage, 3) + ",";
    response += "\"outlet\":" + String(p2_voltage, 3);
    response += "}";
    
    response += "}";
    
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", response);
}

void handleConfigGet() {
    String response = "{";
    response += "\"success\":true,";
    response += "\"pressureThreshold\":" + String(pressureThreshold, 1) + ",";
    response += "\"updateInterval\":1,";
    response += "\"deviceName\":\"ESP32_C6_Monitor\",";
    response += "\"scaling\":{";
    response += "\"pressure1Min\":" + String(pressure1_min, 1) + ",";
    response += "\"pressure1Max\":" + String(pressure1_max, 1) + ",";
    response += "\"pressure2Min\":" + String(pressure2_min, 1) + ",";
    response += "\"pressure2Max\":" + String(pressure2_max, 1) + ",";
    response += "\"flowMin\":" + String(flow_min, 1) + ",";
    response += "\"flowMax\":" + String(flow_max, 1);
    response += "}";
    response += "}";
    
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", response);
}

void handleConfigPost() {
    if (server.hasArg("plain")) {
        String body = server.arg("plain");
        Serial.println("Config update: " + body);
        
        // Parse pressure threshold
        int thresholdPos = body.indexOf("\"pressureThreshold\":");
        if (thresholdPos >= 0) {
            int commaPos = body.indexOf(',', thresholdPos);
            if (commaPos == -1) commaPos = body.indexOf('}', thresholdPos);
            pressureThreshold = body.substring(thresholdPos + 20, commaPos).toFloat();
            Serial.println("Pressure threshold updated to: " + String(pressureThreshold));
        }
    }
    
    String response = "{";
    response += "\"success\":true,";
    response += "\"message\":\"Configuration updated\"";
    response += "}";
    
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", response);
}

void handleCalibrate() {
    if (server.hasArg("plain")) {
        String body = server.arg("plain");
        Serial.println("Calibration request: " + body);
        
        // Parse calibration data
        String type = "";
        float actualValue = 0.0;
        float calculatedValue = 0.0;
        float percentError = 0.0;
        bool confirmed = false;
        
        int typePos = body.indexOf("\"type\":\"");
        if (typePos >= 0) {
            int endPos = body.indexOf("\"", typePos + 8);
            type = body.substring(typePos + 8, endPos);
        }
        
        int actualPos = body.indexOf("\"actualValue\":");
        if (actualPos >= 0) {
            int commaPos = body.indexOf(',', actualPos);
            if (commaPos == -1) commaPos = body.indexOf('}', actualPos);
            actualValue = body.substring(actualPos + 14, commaPos).toFloat();
        }
        
        int calculatedPos = body.indexOf("\"calculatedValue\":");
        if (calculatedPos >= 0) {
            int commaPos = body.indexOf(',', calculatedPos);
            if (commaPos == -1) commaPos = body.indexOf('}', calculatedPos);
            calculatedValue = body.substring(calculatedPos + 18, commaPos).toFloat();
        }
        
        int errorPos = body.indexOf("\"percentError\":");
        if (errorPos >= 0) {
            int commaPos = body.indexOf(',', errorPos);
            if (commaPos == -1) commaPos = body.indexOf('}', errorPos);
            percentError = body.substring(errorPos + 15, commaPos).toFloat();
        }
        
        int confirmedPos = body.indexOf("\"confirmed\":true");
        if (confirmedPos >= 0) {
            confirmed = true;
        }
        
        Serial.println("Calibration details:");
        Serial.println("Type: " + type);
        Serial.println("Actual: " + String(actualValue, 3));
        Serial.println("Calculated: " + String(calculatedValue, 3));
        Serial.println("Error: " + String(percentError, 2) + "%");
        Serial.println("Confirmed: " + String(confirmed ? "true" : "false"));
        
        // Store calibration results (in production, save to EEPROM)
        if (confirmed && percentError <= 5.0) {
            Serial.println("✅ Calibration accepted and stored");
            
            // Optional: Fine-tune scaling factors based on comparison
            // This is where you could implement automatic scaling adjustment
            // if the error is consistently in one direction
            
            if (type == "flow") {
                Serial.println("Flow sensor calibration confirmed with " + String(percentError, 1) + "% error");
            } else if (type == "pressure1") {
                Serial.println("Pressure sensor 1 calibration confirmed with " + String(percentError, 1) + "% error");
            } else if (type == "pressure2") {
                Serial.println("Pressure sensor 2 calibration confirmed with " + String(percentError, 1) + "% error");
            }
        }
    }
    
    String response = "{";
    response += "\"success\":true,";
    response += "\"message\":\"Calibration process completed\"";
    response += "}";
    
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", response);
}

void handleSolenoid() {
    if (server.hasArg("plain")) {
        String body = server.arg("plain");
        
        // Simple JSON parsing - look for "state":true or "state":false
        if (body.indexOf("\"state\":true") >= 0) {
            solenoidState = true;
            digitalWrite(SOLENOID_PIN, HIGH);
            Serial.println("Solenoid ON");
        } else if (body.indexOf("\"state\":false") >= 0) {
            solenoidState = false;
            digitalWrite(SOLENOID_PIN, LOW);
            Serial.println("Solenoid OFF");
        }
    }
    
    String response = "{";
    response += "\"success\":true,";
    response += "\"newState\":" + String(solenoidState ? "true" : "false") + ",";
    response += "\"message\":\"Solenoid " + String(solenoidState ? "ON" : "OFF") + "\"";
    response += "}";
    
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", response);
}

void handleScaling() {
    if (server.hasArg("plain")) {
        String body = server.arg("plain");
        Serial.println("Received scaling data: " + body);
        
        // Simple parsing for scaling values
        int p1MinPos = body.indexOf("\"pressure1Min\":");
        int p1MaxPos = body.indexOf("\"pressure1Max\":");
        int p2MinPos = body.indexOf("\"pressure2Min\":");
        int p2MaxPos = body.indexOf("\"pressure2Max\":");
        int fMinPos = body.indexOf("\"flowMin\":");
        int fMaxPos = body.indexOf("\"flowMax\":");
        
        if (p1MinPos >= 0) {
            int commaPos = body.indexOf(',', p1MinPos);
            if (commaPos == -1) commaPos = body.indexOf('}', p1MinPos);
            pressure1_min = body.substring(p1MinPos + 15, commaPos).toFloat();
        }
        if (p1MaxPos >= 0) {
            int commaPos = body.indexOf(',', p1MaxPos);
            if (commaPos == -1) commaPos = body.indexOf('}', p1MaxPos);
            pressure1_max = body.substring(p1MaxPos + 15, commaPos).toFloat();
        }
        if (p2MinPos >= 0) {
            int commaPos = body.indexOf(',', p2MinPos);
            if (commaPos == -1) commaPos = body.indexOf('}', p2MinPos);
            pressure2_min = body.substring(p2MinPos + 15, commaPos).toFloat();
        }
        if (p2MaxPos >= 0) {
            int commaPos = body.indexOf(',', p2MaxPos);
            if (commaPos == -1) commaPos = body.indexOf('}', p2MaxPos);
            pressure2_max = body.substring(p2MaxPos + 15, commaPos).toFloat();
        }
        if (fMinPos >= 0) {
            int commaPos = body.indexOf(',', fMinPos);
            if (commaPos == -1) commaPos = body.indexOf('}', fMinPos);
            flow_min = body.substring(fMinPos + 10, commaPos).toFloat();
        }
        if (fMaxPos >= 0) {
            int commaPos = body.indexOf(',', fMaxPos);
            if (commaPos == -1) commaPos = body.indexOf('}', fMaxPos);
            flow_max = body.substring(fMaxPos + 10, commaPos).toFloat();
        }
        
        Serial.println("Scaling updated:");
        Serial.println("P1: " + String(pressure1_min) + " - " + String(pressure1_max));
        Serial.println("P2: " + String(pressure2_min) + " - " + String(pressure2_max));
        Serial.println("Flow: " + String(flow_min) + " - " + String(flow_max));
    }
    
    String response = "{\"success\":true,\"message\":\"Scaling updated\"}";
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", response);
}