# IoT Test Stand - Software Architecture Documentation v2.1

## 1. Overview

### 1.1 System Architecture
The IoT Test Stand implements a distributed client-server architecture where the ESP32-C6 microcontroller serves as both a real-time data acquisition system and HTTP web server, communicating with browser-based clients over WiFi using RESTful HTTP/JSON protocols.

### 1.2 Design Principles
- **Fail-Safe Design**: System defaults to safe states (valve closed) on any failure condition
- **Real-time Processing**: Continuous sensor monitoring with automatic safety threshold enforcement
- **Stateful Operation**: Comprehensive test state management with pause/resume capabilities  
- **Modular Design**: Independent, loosely-coupled components for maintainability
- **Memory Efficiency**: Static allocation patterns optimized for extended operation (72+ hours)
- **API-First Design**: Complete functionality accessible via RESTful HTTP interface

### 1.3 Hardware Platform
- **Microcontroller**: ESP32-C6 (160 MHz dual-core, 512KB RAM, 4MB Flash)
- **Connectivity**: WiFi 802.11 b/g/n (2.4 GHz)
- **Analog Interface**: 12-bit ADC with 0-3.3V range
- **Digital I/O**: GPIO control for solenoid valve

## 2. System Components Overview

```
┌─────────────────────┐    WiFi/HTTP    ┌─────────────────────┐
│   Browser Client    │◄───────────────►│   ESP32-C6 Server   │
│                     │   JSON/REST     │                     │
│ ┌─────────────────┐ │                 │ ┌─────────────────┐ │
│ │  Web Interface  │ │                 │ │  HTTP Server    │ │
│ │  (JavaScript)   │ │                 │ │  (Arduino C++)  │ │
│ └─────────────────┘ │                 │ └─────────────────┘ │
│ ┌─────────────────┐ │                 │ ┌─────────────────┐ │
│ │ Real-time Charts│ │                 │ │ State Machine   │ │
│ │  & Visualization│ │                 │ │  & Test Control │ │
│ └─────────────────┘ │                 │ └─────────────────┘ │
│ ┌─────────────────┐ │                 │ ┌─────────────────┐ │
│ │ Data Export &   │ │                 │ │ Sensor Interface│ │
│ │ Configuration   │ │                 │ │  & Safety Logic │ │
│ └─────────────────┘ │                 │ └─────────────────┘ │
└─────────────────────┘                 └─────────────────────┘
                                                  │
                                                  │ GPIO/ADC
                                                  ▼
                                        ┌─────────────────────┐
                                        │   Hardware Layer    │
                                        │                     │
│ GPIO 1  - Inlet Pressure (ADC1_CH1)  │
│ GPIO 2  - Outlet Pressure (ADC1_CH2) │
│ GPIO 5  - Flow Sensor (ADC1_CH5)     │
│ GPIO 10 - Solenoid Control (Digital) │
                                        └─────────────────────┘
```

## 3. Backend Architecture (ESP32-C6)

### 3.1 Application Layer Structure

#### 3.1.1 Main Application Loop
```cpp
void loop() {
    server.handleClient();  // Process HTTP requests
    delay(10);              // Minimal delay for system stability
}
```

**Design Rationale:**
- Minimal blocking operations in main loop
- HTTP server handles all client communication
- No background tasks - all processing triggered by API calls
- Event-driven architecture with immediate response to requests

#### 3.1.2 Global State Management
```cpp
// Test session state variables
bool testRunning = false;
bool testPaused = false;
unsigned long testStartTime = 0;
unsigned long pausedDuration = 0;
float totalVolume = 0.0;
unsigned long lastDataTime = 0;
float pressureThreshold = 20.0;

// Hardware state
bool solenoidState = false;
```

**State Transition Rules:**
- `IDLE → RUNNING`: Start test, open valve, reset timers
- `RUNNING → PAUSED`: Close valve, preserve data and timing
- `PAUSED → RUNNING`: Resume test, adjust timing for pause duration
- `ANY → IDLE`: Reset all state, close valve, clear data

### 3.2 Hardware Abstraction Layer

#### 3.2.1 ADC Interface
```cpp
float readVoltage(int pin) {
    int raw = analogRead(pin);
    return (raw / 4095.0) * 3.3;  // Convert to voltage
}
```

**ADC Configuration:**
- 12-bit resolution (0-4095 counts)
- 11dB attenuation for 0-3.3V range
- Linear conversion formula
- No averaging or filtering (real-time response priority)

#### 3.2.2 Sensor Scaling Algorithm
```cpp
float scaleValue(float voltage, float minVal, float maxVal) {
    // Clamp voltage to valid sensor range
    if (voltage < 0.66) voltage = 0.66;
    if (voltage > 3.3) voltage = 3.3;
    
    // Linear scaling from voltage range to engineering units
    float normalized = (voltage - 0.66) / (3.3 - 0.66);
    return minVal + (normalized * (maxVal - minVal));
}
```

**Scaling Parameters:**
- Input Range: 0.66V - 3.30V (sensor linear response range)
- Output Range: Configurable min/max values per sensor
- Default Ranges: Pressure 0-50 PSI, Flow 0-10 L/min
- Real-time reconfigurable via `/api/scaling` endpoint

#### 3.2.3 Valve Control
```cpp
// Solenoid control with immediate hardware response
void controlValve(bool open) {
    solenoidState = open;
    digitalWrite(SOLENOID_PIN, open ? HIGH : LOW);
}
```

**Safety Features:**
- Fail-safe design: Valve defaults to closed (LOW)
- Immediate response to control commands
- State tracking for API responses
- Manual override capability via `/api/solenoid`

### 3.3 HTTP Server Architecture

#### 3.3.1 Route Configuration
```cpp
void setupServer() {
    // CORS preflight handling
    server.on("/api/status", HTTP_OPTIONS, handleCORS);
    server.on("/api/start", HTTP_OPTIONS, handleCORS);
    // ... (all endpoints)
    
    // API endpoints
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
    
    // Root endpoint and 404 handler
    server.on("/", HTTP_GET, handleRoot);
    server.onNotFound(handleNotFound);
}
```

#### 3.3.2 CORS Implementation
```cpp
void handleCORS() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type, Accept");
    server.sendHeader("Access-Control-Max-Age", "86400");
    server.send(200, "text/plain", "");
}
```

**Browser Compatibility:**
- Full CORS support for cross-origin requests
- Handles preflight OPTIONS requests
- Compatible with all modern browsers
- No authentication required (trusted network assumption)

#### 3.3.3 JSON Response Generation
```cpp
// Example: Status endpoint response structure
String response = "{";
response += "\"success\":true,";
response += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
response += "\"firmware\":\"v2.1-ESP32C6\",";
// ... additional fields
response += "}";
```

**JSON Structure Standards:**
- Always include `success` boolean field
- Consistent field naming (camelCase)
- Numeric precision appropriate for application (2-3 decimal places)
- Structured data grouping (inputs, outputs, scaling, etc.)

### 3.4 Data Processing Layer

#### 3.4.1 Real-time Calculations
```cpp
// Pressure drop calculation
float pressureDrop = abs(p1_scaled - p2_scaled);

// Volume integration over time
if (testRunning && lastDataTime > 0) {
    float deltaTime = (millis() - lastDataTime) / 1000.0 / 60.0; // minutes
    totalVolume += flow_scaled * deltaTime; // L/min * minutes = Liters
}
lastDataTime = millis();
```

**Calculation Methods:**
- Pressure Drop: Absolute difference between inlet and outlet pressures
- Volume Integration: Trapezoidal integration of flow rate over time
- Elapsed Time: Accounts for pause durations automatically
- Real-time Updates: Calculations performed on each `/api/data` request

#### 3.4.2 Safety Monitoring
```cpp
// Automatic safety shutdown logic
if (testRunning && pressureDrop >= pressureThreshold) {
    testRunning = false;
    testPaused = true;
    solenoidState = false;
    digitalWrite(SOLENOID_PIN, LOW);
    Serial.println("AUTO-SHUTDOWN: Pressure threshold exceeded");
}
```

**Safety Features:**
- Continuous threshold monitoring during active tests
- Immediate valve closure on threshold violation
- Data preservation during emergency stops
- Automatic state transition to paused (allows resume)
- Serial logging for diagnostic purposes

### 3.5 Memory Management Strategy

#### 3.5.1 Static Allocation Pattern
```cpp
// Global variables for system state (stack allocation)
bool testRunning = false;
unsigned long testStartTime = 0;
float totalVolume = 0.0;

// No dynamic memory allocation
// No malloc/free calls
// No STL containers that allocate dynamically
```

**Memory Efficiency:**
- All data structures statically allocated at compile time
- No heap fragmentation issues
- Predictable memory usage patterns
- Suitable for extended operation (72+ hours)
- Memory usage <80% of available ESP32 memory

#### 3.5.2 String Handling
```cpp
// Efficient string concatenation for JSON responses
String response = "{";
response += "\"field\":\"value\",";
response += "\"number\":" + String(value, precision);
response += "}";
```

**String Management:**
- Arduino String class with automatic memory management
- Careful concatenation to avoid excessive reallocations
- Precision control for numeric conversions
- No permanent string storage (responses generated on-demand)

## 4. Frontend Architecture (Web Interface)

### 4.1 Client-Side Architecture

#### 4.1.1 Global State Management
```javascript
// Application state object
const AppState = {
    connection: {
        connected: false,
        esp32IP: null,
        baseUrl: ''
    },
    testData: {
        running: false,
        paused: false,
        elapsedTime: 0,
        currentReadings: {},
        dataLog: []
    },
    configuration: {
        pressureThreshold: 20,
        dataLoggingInterval: 1.0,
        sensorScaling: {}
    },
    ui: {
        chartInitialized: false,
        lastUpdate: null
    }
};
```

#### 4.1.2 HTTP Communication Wrapper
```javascript
// Centralized API communication
class ESP32Client {
    constructor(baseUrl) {
        this.baseUrl = baseUrl;
        this.timeout = 5000;
    }
    
    async request(endpoint, options = {}) {
        const url = `${this.baseUrl}${endpoint}`;
        const defaultOptions = {
            headers: { 'Content-Type': 'application/json' },
            timeout: this.timeout
        };
        
        try {
            const response = await fetch(url, { ...defaultOptions, ...options });
            return await response.json();
        } catch (error) {
            console.error(`API request failed: ${endpoint}`, error);
            throw error;
        }
    }
    
    async getStatus() { return this.request('/api/status'); }
    async getData() { return this.request('/api/data'); }
    async startTest(config) { 
        return this.request('/api/start', {
            method: 'POST',
            body: JSON.stringify(config)
        });
    }
}
```

### 4.2 Real-time Data Management

#### 4.2.1 Polling Architecture
```javascript
// Configurable polling system
class DataPollingManager {
    constructor(client, updateCallback) {
        this.client = client;
        this.callback = updateCallback;
        this.interval = null;
        this.pollRate = 500; // 500ms for real-time display
        this.logInterval = 1.0; // 1 second for data logging
    }
    
    start() {
        this.interval = setInterval(async () => {
            try {
                const data = await this.client.getData();
                this.callback(data);
            } catch (error) {
                console.error('Polling error:', error);
            }
        }, this.pollRate);
    }
    
    stop() {
        if (this.interval) {
            clearInterval(this.interval);
            this.interval = null;
        }
    }
}
```

**Polling Strategy:**
- High-frequency polling (500ms) for smooth real-time display
- Configurable data logging interval (0.5-600 seconds)
- Automatic error handling and recovery
- Efficient data point management with overflow protection

#### 4.2.2 Time-Based Data Logging
```javascript
// Intelligent data logging with configurable intervals
function shouldLogData() {
    if (dataLog.length === 0) return true; // Always log first point
    
    const timeSinceLastLog = elapsedSeconds - lastLogTime;
    return timeSinceLastLog >= dataLoggingInterval;
}

function processData(data) {
    // Always update real-time displays
    updateMetricDisplays(data);
    
    // Log data at configured intervals
    if (shouldLogData()) {
        const logEntry = {
            time: elapsedSeconds,
            flow: data.flowRate,
            pressure: data.pressureDrop,
            volume: data.totalVolume
        };
        
        dataLog.push(logEntry);
        lastLogTime = elapsedSeconds;
        
        // Update visualization
        updateChart(logEntry.time, logEntry.flow, logEntry.pressure);
        updateDataTable();
        
        // Manage memory usage
        if (dataLog.length > 2000) {
            dataLog.shift(); // Remove oldest entries
        }
    }
}
```

**Data Management Strategy:**
- Dual-rate system: Fast display updates, configurable data logging
- Time-based logging prevents data gaps during variable network conditions
- Automatic memory management with circular buffer behavior
- Preserves all data during pause/resume cycles

### 4.3 Visualization Engine

#### 4.3.1 Canvas-Based Real-time Charting
```javascript
class RealTimeChart {
    constructor(canvasElement, options = {}) {
        this.canvas = canvasElement;
        this.ctx = canvasElement.getContext('2d');
        this.width = canvasElement.width;
        this.height = canvasElement.height;
        this.padding = { top: 40, right: 120, bottom: 60, left: 120 };
        this.maxDisplayPoints = options.maxPoints || 15;
    }
    
    updateData(time, flowRate, pressureDrop) {
        chartData.labels.push(time);
        chartData.flow.push(flowRate);
        chartData.pressure.push(pressureDrop);
        
        this.render();
    }
    
    render() {
        this.clearCanvas();
        this.drawGrid();
        this.drawAxes();
        this.drawDataLines();
        this.drawLabels();
    }
    
    getDisplayData() {
        // Return evenly spaced points for smooth visualization
        if (chartData.labels.length <= this.maxDisplayPoints) {
            return chartData; // Use all points if few enough
        }
        
        // Calculate evenly spaced indices
        const totalPoints = chartData.labels.length;
        const step = (totalPoints - 1) / (this.maxDisplayPoints - 1);
        
        const spacedData = { labels: [], flow: [], pressure: [] };
        
        for (let i = 0; i < this.maxDisplayPoints; i++) {
            const index = i === this.maxDisplayPoints - 1 
                ? totalPoints - 1  // Always include last point
                : Math.round(i * step);
                
            spacedData.labels.push(chartData.labels[index]);
            spacedData.flow.push(chartData.flow[index]);
            spacedData.pressure.push(chartData.pressure[index]);
        }
        
        return spacedData;
    }
}
```

**Visualization Features:**
- Dual-axis charting (flow rate and pressure drop)
- Intelligent data point selection for smooth display
- Responsive canvas sizing
- Real-time scaling and axis management
- Color-coded data series with legends

#### 4.3.2 Data Export System
```javascript
// Comprehensive data export with multiple formats
class DataExportManager {
    constructor(dataLog, testMetadata) {
        this.data = dataLog;
        this.metadata = testMetadata;
    }
    
    generateCSV() {
        const headers = ['Timestamp (ISO)', 'Elapsed Time (s)', 'Flow Rate (L/min)', 
                        'Pressure Drop (PSI)', 'Total Volume (gal)'];
        
        const rows = this.data.map(row => [
            new Date(Date.now() - (this.metadata.totalTime - row.time) * 1000).toISOString(),
            row.time,
            row.flow.toFixed(3),
            row.pressure.toFixed(2),
            row.volume.toFixed(4)
        ]);
        
        const csvContent = [
            this.generateMetadataHeader(),
            headers.join(','),
            ...rows.map(row => row.join(','))
        ].join('\n');
        
        return csvContent;
    }
    
    async copyToClipboard() {
        const tableData = this.generateTableFormat();
        
        try {
            await navigator.clipboard.writeText(tableData);
            return { success: true, format: 'clipboard' };
        } catch (error) {
            return { success: false, error: error.message };
        }
    }
    
    downloadCSV(filename) {
        const csvContent = this.generateCSV();
        const blob = new Blob([csvContent], { type: 'text/csv;charset=utf-8;' });
        
        const url = window.URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = filename;
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        window.URL.revokeObjectURL(url);
        
        return { success: true, format: 'csv', filename };
    }
}
```

## 5. Communication Protocols

### 5.1 HTTP/JSON Protocol Implementation

#### 5.1.1 Request/Response Flow
```
Client                                    ESP32-C6 Server
  │                                             │
  ├─ HTTP GET /api/status ──────────────────────►│
  │                                             ├─ readVoltage() × 3
  │                                             ├─ scaleValue() × 3  
  │                                             ├─ JSON generation
  │◄─────────── JSON Response ─────────────────┤
  │                                             │
  ├─ HTTP POST /api/start ─────────────────────►│
  │     {"pressureThreshold": 25.0}             ├─ Parse JSON body
  │                                             ├─ Update state variables
  │                                             ├─ digitalWrite(SOLENOID_PIN, HIGH)
  │◄─────────── Success Response ──────────────┤
```

#### 5.1.2 Error Handling Strategy
```javascript
// Exponential backoff retry mechanism
class RetryHandler {
    constructor(maxRetries = 3, baseDelay = 1000) {
        this.maxRetries = maxRetries;
        this.baseDelay = baseDelay;
    }
    
    async executeWithRetry(operation) {
        for (let attempt = 0; attempt <= this.maxRetries; attempt++) {
            try {
                return await operation();
            } catch (error) {
                if (attempt === this.maxRetries) {
                    throw new Error(`Operation failed after ${this.maxRetries} retries: ${error.message}`);
                }
                
                const delay = this.baseDelay * Math.pow(2, attempt);
                await new Promise(resolve => setTimeout(resolve, delay));
                console.log(`Retry attempt ${attempt + 1} after ${delay}ms delay`);
            }
        }
    }
}
```

### 5.2 Network Management

#### 5.2.1 WiFi Connection Handling
```cpp
// ESP32 WiFi connection with status monitoring
void setupWiFi() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi");
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.print("Connected! IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("WiFi connection failed!");
        // Could implement AP mode fallback here
    }
}
```

**Network Features:**
- Automatic connection on startup
- Connection status monitoring
- IP address assignment and reporting
- Signal strength monitoring (RSSI)
- Future enhancement: Automatic reconnection logic

## 6. Data Structures and Algorithms

### 6.1 Sensor Data Processing

#### 6.1.1 Voltage-to-Engineering Units Conversion
```cpp
struct SensorCalibration {
    float minVoltage = 0.66;    // Sensor minimum output
    float maxVoltage = 3.30;    // Sensor maximum output  
    float minValue;             // Engineering unit minimum
    float maxValue;             // Engineering unit maximum
    
    float convert(float voltage) {
        // Clamp voltage to valid range
        voltage = constrain(voltage, minVoltage, maxVoltage);
        
        // Linear interpolation
        float normalized = (voltage - minVoltage) / (maxVoltage - minVoltage);
        return minValue + (normalized * (maxValue - minValue));
    }
};
```

#### 6.1.2 Real-time Volume Integration
```cpp
class VolumeIntegrator {
private:
    unsigned long lastUpdateTime;
    float cumulativeVolume;
    
public:
    void update(float flowRate) {
        unsigned long currentTime = millis();
        
        if (lastUpdateTime > 0) {
            float deltaTimeMinutes = (currentTime - lastUpdateTime) / 60000.0;
            cumulativeVolume += flowRate * deltaTimeMinutes; // L/min × min = L
        }
        
        lastUpdateTime = currentTime;
    }
    
    float getVolume() const { return cumulativeVolume; }
    void reset() { cumulativeVolume = 0.0; lastUpdateTime = 0; }
};
```

### 6.2 State Machine Implementation

#### 6.2.1 Test State Management
```cpp
enum TestState {
    IDLE,       // No test active, valve closed
    RUNNING,    // Test active, valve open, data logging
    PAUSED,     // Test paused, valve closed, data preserved
    EMERGENCY   // Emergency stop, valve closed, data preserved
};

class TestStateMachine {
private:
    TestState currentState = IDLE;
    unsigned long stateEntryTime;
    
public:
    bool transitionTo(TestState newState) {
        if (isValidTransition(currentState, newState)) {
            currentState = newState;
            stateEntryTime = millis();
            onStateEntry(newState);
            return true;
        }
        return false;
    }
    
private:
    bool isValidTransition(TestState from, TestState to) {
        switch (from) {
            case IDLE: return (to == RUNNING);
            case RUNNING: return (to == PAUSED || to == EMERGENCY || to == IDLE);
            case PAUSED: return (to == RUNNING || to == IDLE);
            case EMERGENCY: return (to == IDLE);
            default: return false;
        }
    }
    
    void onStateEntry(TestState state) {
        switch (state) {
            case RUNNING:
                digitalWrite(SOLENOID_PIN, HIGH);
                Serial.println("State: RUNNING - Valve opened");
                break;
            case PAUSED:
            case EMERGENCY:
            case IDLE:
                digitalWrite(SOLENOID_PIN, LOW);
                Serial.println("State: " + String(state) + " - Valve closed");
                break;
        }
    }
};
```

## 7. Safety and Error Handling

### 7.1 Hardware Safety Implementation

#### 7.1.1 Pressure Threshold Monitoring
```cpp
class SafetyMonitor {
private:
    float pressureThreshold;
    unsigned long lastSafetyCheck;
    bool emergencyState = false;
    
public:
    SafetyMonitor(float threshold) : pressureThreshold(threshold) {}
    
    bool checkPressureSafety(float pressureDrop) {
        lastSafetyCheck = millis();
        
        if (pressureDrop >= pressureThreshold) {
            if (!emergencyState) {
                triggerEmergencyStop();
                emergencyState = true;
            }
            return false; // Unsafe condition
        }
        
        emergencyState = false;
        return true; // Safe to continue
    }
    
private:
    void triggerEmergencyStop() {
        digitalWrite(SOLENOID_PIN, LOW);  // Immediate valve closure
        testRunning = false;
        testPaused = true;
        
        Serial.println("EMERGENCY STOP: Pressure threshold exceeded!");
        // Additional safety actions could be added here
    }
};
```

### 7.2 Network Error Recovery

#### 7.2.1 Connection Health Monitoring
```javascript
class ConnectionHealthMonitor {
    constructor(client, healthCallback) {
        this.client = client;
        this.callback = healthCallback;
        this.consecutiveFailures = 0;
        this.maxFailures = 3;
        this.healthCheckInterval = 5000; // 5 seconds
    }
    
    startMonitoring() {
        this.healthInterval = setInterval(async () => {
            try {
                await this.client.getStatus();
                this.onSuccess();
            } catch (error) {
                this.onFailure(error);
            }
        }, this.healthCheckInterval);
    }
    
    onSuccess() {
        if (this.consecutiveFailures > 0) {
            console.log('Connection restored');
            this.callback({ connected: true, recovered: true });
        }
        this.consecutiveFailures = 0;
    }
    
    onFailure(error) {
        this.consecutiveFailures++;
        console.log(`Connection failure ${this.consecutiveFailures}/${this.maxFailures}:`, error);
        
        if (this.consecutiveFailures >= this.maxFailures) {
            this.callback({ 
                connected: false, 
                error: error.message,
                failureCount: this.consecutiveFailures 
            });
        }
    }
}
```

## 8. Performance Optimization

### 8.1 Memory Usage Optimization

#### 8.1.1 String Memory Management
```cpp
// Avoid String concatenation in loops - pre-allocate capacity
String buildStatusResponse() {
    String response;
    response.reserve(1024); // Pre-allocate memory to avoid reallocations
    
    response = "{";
    response += "\"success\":true,";
    response += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
    // ... continue building response
    response += "}";
    
    return response;
}
```

#### 8.1.2 Efficient Data Structures
```javascript
// Client-side circular buffer for memory efficiency
class CircularBuffer {
    constructor(maxSize) {
        this.buffer = new Array(maxSize);
        this.maxSize = maxSize;
        this.head = 0;
        this.tail = 0;
        this.count = 0;
    }
    
    push(item) {
        this.buffer[this.head] = item;
        this.head = (this.head + 1) % this.maxSize;
        
        if (this.count < this.maxSize) {
            this.count++;
        } else {
            this.tail = (this.tail + 1) % this.maxSize;
        }
    }
    
    getRecent(num) {
        const result = [];
        const actualNum = Math.min(num, this.count);
        
        for (let i = 0; i < actualNum; i++) {
            const index = (this.head - 1 - i + this.maxSize) % this.maxSize;
            result.unshift(this.buffer[index]);
        }
        
        return result;
    }
}
```

### 8.2 Real-time Performance

#### 8.2.1 Response Time Optimization
```cpp
// Minimize processing time in critical paths
void handleData() {
    unsigned long startTime = micros();
    
    // Read all sensors quickly
    float p1_voltage = readVoltage(PRESSURE1_PIN);
    float p2_voltage = readVoltage(PRESSURE2_PIN);
    float flow_voltage = readVoltage(FLOW_SENSOR_PIN);
    
    // Perform calculations
    float p1_scaled = scaleValue(p1_voltage, pressure1_min, pressure1_max);
    float p2_scaled = scaleValue(p2_voltage, pressure2_min, pressure2_max);
    float flow_scaled = scaleValue(flow_voltage, flow_min, flow_max);
    float pressureDrop = abs(p1_scaled - p2_scaled);
    
    // Safety check (critical timing)
    if (testRunning && pressureDrop >= pressureThreshold) {
        digitalWrite(SOLENOID_PIN, LOW); // Immediate response
        testRunning = false;
        testPaused = true;
    }
    
    // Build and send response
    String response = buildDataResponse(/* parameters */);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", response);
    
    unsigned long processingTime = micros() - startTime;
    if (processingTime > 50000) { // 50ms threshold
        Serial.println("WARNING: Long processing time: " + String(processingTime) + "μs");
    }
}
```

## 9. Testing and Validation

### 9.1 Unit Testing Framework

#### 9.1.1 Sensor Reading Validation
```cpp
class SensorTest {
public:
    static bool testVoltageReading() {
        // Test with known analog values
        // Verify conversion accuracy
        float testVoltage = 1.65; // Mid-range
        float expected = 1.65;
        float actual = readVoltage(TEST_PIN);
        
        return abs(actual - expected) < 0.05; // 50mV tolerance
    }
    
    static bool testScaling() {
        float testVoltage = 1.98; // (1.98-0.66)/(3.3-0.66) = 0.5 = 50%
        float expected = 25.0; // 50% of 0-50 range
        float actual = scaleValue(testVoltage, 0.0, 50.0);
        
        return abs(actual - expected) < 0.1;
    }
    
    static void runAllTests() {
        Serial.println("Running sensor tests...");
        Serial.println("Voltage reading: " + String(testVoltageReading() ? "PASS" : "FAIL"));
        Serial.println("Scaling: " + String(testScaling() ? "PASS" : "FAIL"));
    }
};
```

### 9.2 Integration Testing

#### 9.2.1 End-to-End API Testing
```javascript
class APITestSuite {
    constructor(baseUrl) {
        this.baseUrl = baseUrl;
        this.client = new ESP32Client(baseUrl);
    }
    
    async runFullTestSuite() {
        const results = {
            status: await this.testStatusEndpoint(),
            config: await this.testConfigEndpoint(),
            testFlow: await this.testCompleteFlow(),
            safety: await this.testSafetyFeatures()
        };
        
        return results;
    }
    
    async testCompleteFlow() {
        try {
            // Test complete start -> data -> stop -> reset flow
            const startResult = await this.client.startTest({ pressureThreshold: 30.0 });
            if (!startResult.success) return { success: false, stage: 'start' };
            
            await new Promise(resolve => setTimeout(resolve, 2000)); // 2 second test
            
            const dataResult = await this.client.getData();
            if (!dataResult.success) return { success: false, stage: 'data' };
            
            const stopResult = await this.client.stopTest();
            if (!stopResult.success) return { success: false, stage: 'stop' };
            
            const resetResult = await this.client.resetTest();
            if (!resetResult.success) return { success: false, stage: 'reset' };
            
            return { success: true, stages: ['start', 'data', 'stop', 'reset'] };
        } catch (error) {
            return { success: false, error: error.message };
        }
    }
}
```

## 10. Deployment and Maintenance

### 10.1 Firmware Build Process

#### 10.1.1 Arduino IDE Configuration
```
Board: "ESP32C6 Dev Module"
Upload Speed: "921600"
CPU Frequency: "160MHz (WiFi)"
Flash Mode: "QIO"
Flash Size: "4MB (32Mb)"
Partition Scheme: "Default 4MB with spiffs"
```

#### 10.1.2 Library Dependencies
```cpp
#include <WiFi.h>        // ESP32 WiFi functionality
#include <WebServer.h>   // HTTP server implementation
// No additional libraries required - uses Arduino core only
```

### 10.2 System Monitoring

#### 10.2.1 Diagnostic Logging
```cpp
class SystemDiagnostics {
private:
    unsigned long bootTime;
    uint32_t loopCount = 0;
    uint32_t apiRequestCount = 0;
    
public:
    SystemDiagnostics() : bootTime(millis()) {}
    
    void logSystemHealth() {
        Serial.println("=== System Health Report ===");
        Serial.println("Uptime: " + String((millis() - bootTime) / 1000) + " seconds");
        Serial.println("Free Memory: " + String(ESP.getFreeHeap()) + " bytes");
        Serial.println("WiFi RSSI: " + String(WiFi.RSSI()) + " dBm");
        Serial.println("Loop Count: " + String(loopCount));
        Serial.println("API Requests: " + String(apiRequestCount));
        Serial.println("============================");
    }
    
    void incrementLoopCount() { loopCount++; }
    void incrementAPICount() { apiRequestCount++; }
};
```

## 11. Security Considerations

### 11.1 Network Security

#### 11.1.1 Input Validation
```cpp
class InputValidator {
public:
    static bool validatePressureThreshold(float value) {
        return (value >= 5.0f && value <= 100.0f);
    }
    
    static bool validateScalingRange(float min, float max) {
        return (min < max && min >= 0.0f && max <= 1000.0f);
    }
    
    static String sanitizeDeviceName(const String& name) {
        String sanitized = "";
        for (int i = 0; i < name.length() && i < 32; i++) {
            char c = name.charAt(i);
            if (isAlphaNumeric(c) || c == '_' || c == '-') {
                sanitized += c;
            }
        }
        return sanitized;
    }
};
```

### 11.2 Operational Security

#### 11.2.1 Safe Defaults
```cpp
// System defaults to safe operational parameters
const float DEFAULT_PRESSURE_THRESHOLD = 20.0;  // Conservative threshold
const bool DEFAULT_VALVE_STATE = false;         // Valve closed
const float DEFAULT_SENSOR_MIN = 0.0;           // Safe scaling ranges
const float DEFAULT_SENSOR_MAX = 50.0;
```

## 12. Future Enhancements

### 12.1 Planned Improvements

#### 12.1.1 Data Persistence
```cpp
// Future: EEPROM-based configuration storage
class ConfigurationManager {
    struct PersistentConfig {
        uint32_t magic = 0xDEADBEEF;
        float pressureThreshold;
        float sensorScaling[6]; // min/max for 3 sensors
        char deviceName[32];
        uint32_t checksum;
    };
    
    void saveConfig(const PersistentConfig& config);
    bool loadConfig(PersistentConfig& config);
    bool validateConfig(const PersistentConfig& config);
};
```

#### 12.1.2 Advanced Calibration
```cpp
// Future: Multi-point calibration with curve fitting
class AdvancedCalibration {
    struct CalibrationPoint {
        float voltage;
        float engineeringValue;
    };
    
    std::vector<CalibrationPoint> calibrationPoints;
    
    float interpolate(float voltage) {
        // Implement linear or polynomial interpolation
        // between multiple calibration points
    }
};
```

This comprehensive software architecture documentation reflects the current implementation while providing a foundation for future enhancements. The system is designed for reliability, safety, and ease of use in laboratory environments requiring precise, long-duration filtration testing.
