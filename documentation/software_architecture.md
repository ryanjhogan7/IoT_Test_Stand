# ESP32 Dirt Test Monitor - Software Architecture Documentation

## 1. Overview

### 1.1 System Architecture
The ESP32 Dirt Test Monitor employs a distributed client-server architecture where the ESP32 microcontroller functions as both a data acquisition unit and web server, communicating with browser-based clients over WiFi using HTTP protocols.

### 1.2 Design Principles
- **Separation of Concerns**: Clear distinction between hardware control, data processing, and presentation layers
- **Fail-Safe Design**: System defaults to safe states on failures
- **Real-time Processing**: Continuous sensor monitoring with configurable update intervals
- **Stateful Operation**: Persistent test state management across power cycles
- **Modular Design**: Independent components for easy maintenance and testing

## 2. System Components

```
┌─────────────────────┐    WiFi/HTTP    ┌─────────────────────┐
│   Browser Client    │◄───────────────►│     ESP32 Server    │
│                     │                 │                     │
│ ┌─────────────────┐ │                 │ ┌─────────────────┐ │
│ │  Web Interface  │ │                 │ │  HTTP Server    │ │
│ │   (Frontend)    │ │                 │ │   (Backend)     │ │
│ └─────────────────┘ │                 │ └─────────────────┘ │
│ ┌─────────────────┐ │                 │ ┌─────────────────┐ │
│ │ Data Visualization│ │                 │ │ Data Processing │ │
│ │    & Charts     │ │                 │ │   & Control     │ │
│ └─────────────────┘ │                 │ └─────────────────┘ │
│ ┌─────────────────┐ │                 │ ┌─────────────────┐ │
│ │ Export & Config │ │                 │ │ Hardware I/O    │ │
│ │   Management    │ │                 │ │   Interface     │ │
│ └─────────────────┘ │                 │ └─────────────────┘ │
└─────────────────────┘                 └─────────────────────┘
                                                  │
                                                  │ GPIO/ADC
                                                  ▼
                                        ┌─────────────────────┐
                                        │   Physical Layer    │
                                        │                     │
                                        │ • Flow Sensor       │
                                        │ • Pressure Sensors  │
                                        │ • Solenoid Valve    │
                                        └─────────────────────┘
```

## 3. Backend Architecture (ESP32)

### 3.1 Application Layers

#### 3.1.1 Hardware Abstraction Layer (HAL)
```cpp
// Sensor interface abstraction
class SensorInterface {
public:
    virtual float readVoltage() = 0;
    virtual float getScaledValue() = 0;
    virtual bool isOnline() = 0;
};

// GPIO control abstraction
class ValveController {
public:
    void open();
    void close();
    bool isOpen();
    void setFailSafe();
};
```

**Responsibilities:**
- ADC reading and voltage conversion
- GPIO control for valve operation
- Hardware-specific timing and delays
- Sensor communication protocols

#### 3.1.2 Data Processing Layer
```cpp
// Data processing and filtering
class DataProcessor {
private:
    MovingAverage filter;
    CalibrationData calibration;
    
public:
    SensorReading processSample(float rawVoltage);
    float calculatePressureDrop(float inlet, float outlet);
    float calculateVolume(float flowRate, float deltaTime);
    bool validateReading(SensorReading reading);
};
```

**Responsibilities:**
- Sensor data filtering and noise reduction
- Unit conversion and scaling
- Mathematical calculations (pressure drop, volume)
- Data validation and range checking

#### 3.1.3 State Management Layer
```cpp
// System state machine
enum SystemState {
    IDLE,
    TESTING,
    PAUSED,
    ERROR,
    EMERGENCY_STOP
};

class StateManager {
private:
    SystemState currentState;
    TestSession activeSession;
    
public:
    bool transitionTo(SystemState newState);
    void handleEvent(SystemEvent event);
    TestSession& getCurrentSession();
};
```

**Responsibilities:**
- Test session lifecycle management
- State transitions and validation
- Event handling and response
- Data persistence across states

#### 3.1.4 Communication Layer
```cpp
// HTTP server and API handling
class APIController {
private:
    WebServer server;
    JsonHandler jsonHandler;
    
public:
    void setupRoutes();
    void handleStatusRequest();
    void handleControlRequest();
    void sendResponse(APIResponse response);
};
```

**Responsibilities:**
- HTTP request/response handling
- JSON serialization/deserialization
- CORS header management
- Error response formatting

### 3.2 Data Flow Architecture

```
Sensor Hardware → ADC Reading → Filtering → Scaling → State Update → JSON Response
       ↓               ↓           ↓         ↓          ↓            ↓
   GPIO Pins     Raw Voltage   Noise      Eng.     Test Data    HTTP Client
                               Removal    Units     Storage
```

#### 3.2.1 Sensor Data Pipeline
1. **Acquisition**: 12-bit ADC samples at configured intervals
2. **Filtering**: 10-sample moving average for noise reduction
3. **Conversion**: Raw voltage to engineering units using calibration
4. **Validation**: Range checking and plausibility tests
5. **Storage**: Update internal state and test data arrays
6. **Transmission**: Format for JSON API responses

#### 3.2.2 Control Flow Pipeline
1. **Request Reception**: HTTP POST with JSON payload
2. **Validation**: Parameter checking and state verification
3. **Execution**: Hardware control and state updates
4. **Confirmation**: Response generation and transmission
5. **Persistence**: Critical settings saved to EEPROM

### 3.3 Memory Management

#### 3.3.1 Data Structures
```cpp
// Circular buffer for sensor data
template<typename T, size_t N>
class CircularBuffer {
private:
    T data[N];
    size_t head, tail, count;
    
public:
    void push(const T& item);
    T& operator[](size_t index);
    size_t size() const { return count; }
};

// Test session data
struct TestSession {
    uint32_t startTime;
    uint32_t pausedDuration;
    float totalVolume;
    CircularBuffer<SensorReading, 500> dataPoints;
    TestConfiguration config;
};
```

#### 3.3.2 Memory Allocation Strategy
- **Static Allocation**: All major data structures pre-allocated
- **Circular Buffers**: Fixed-size arrays with overflow protection
- **Stack Usage**: Minimize recursion and large local variables
- **EEPROM Usage**: Configuration data persistence (1KB allocated)

## 4. Frontend Architecture (Web Interface)

### 4.1 Component Structure

```
index.html
├── Global State Management
├── Connection Manager
├── Real-time Data Handler
├── Chart Visualization Engine
├── Control Interface
├── Configuration Manager
└── Export Utilities
```

#### 4.1.1 State Management
```javascript
// Global application state
const AppState = {
    connection: {
        connected: false,
        esp32IP: null,
        lastUpdate: null
    },
    testData: {
        running: false,
        paused: false,
        startTime: null,
        dataPoints: [],
        currentReadings: {}
    },
    configuration: {
        pressureThreshold: 20,
        updateInterval: 1000,
        sensorScaling: {}
    }
};
```

#### 4.1.2 Communication Layer
```javascript
// HTTP communication wrapper
class ESP32Client {
    constructor(baseUrl) {
        this.baseUrl = baseUrl;
        this.timeout = 5000;
    }
    
    async fetchWithTimeout(url, options = {}) {
        // Implementation with timeout and error handling
    }
    
    async getStatus() { /* ... */ }
    async startTest(config) { /* ... */ }
    async stopTest() { /* ... */ }
}
```

### 4.2 Data Flow Patterns

#### 4.2.1 Polling Architecture
```javascript
// Configurable polling for real-time updates
class PollingManager {
    constructor(esp32Client, updateCallback) {
        this.client = esp32Client;
        this.callback = updateCallback;
        this.interval = null;
        this.rate = 1000; // milliseconds
    }
    
    start() {
        this.interval = setInterval(() => {
            this.client.getData()
                .then(data => this.callback(data))
                .catch(error => this.handleError(error));
        }, this.rate);
    }
}
```

#### 4.2.2 Event-Driven Updates
```javascript
// Event system for UI updates
class EventBus {
    constructor() {
        this.listeners = {};
    }
    
    emit(event, data) {
        if (this.listeners[event]) {
            this.listeners[event].forEach(callback => callback(data));
        }
    }
    
    on(event, callback) {
        if (!this.listeners[event]) {
            this.listeners[event] = [];
        }
        this.listeners[event].push(callback);
    }
}
```

### 4.3 Visualization Architecture

#### 4.3.1 Chart Engine
```javascript
// Canvas-based real-time charting
class RealTimeChart {
    constructor(canvasElement, options) {
        this.canvas = canvasElement;
        this.ctx = canvasElement.getContext('2d');
        this.dataPoints = [];
        this.maxPoints = options.maxPoints || 50;
    }
    
    addDataPoint(timestamp, values) {
        this.dataPoints.push({timestamp, ...values});
        if (this.dataPoints.length > this.maxPoints) {
            this.dataPoints.shift();
        }
        this.render();
    }
    
    render() {
        // Canvas drawing implementation
    }
}
```

## 5. Communication Protocols

### 5.1 HTTP/JSON Protocol

#### 5.1.1 Request/Response Cycle
```
Client                           ESP32 Server
  │                                    │
  ├─ HTTP GET /api/status ────────────►│
  │                                    ├─ Read sensors
  │                                    ├─ Format JSON
  │◄──── JSON Response ───────────────┤
  │                                    │
  ├─ HTTP POST /api/start ────────────►│
  │     {"pressureThreshold": 25.0}    ├─ Validate params
  │                                    ├─ Update state
  │                                    ├─ Control hardware
  │◄──── Success/Error Response ──────┤
```

#### 5.1.2 Error Handling Strategy
```javascript
// Exponential backoff for failed requests
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
                if (attempt === this.maxRetries) throw error;
                await this.delay(this.baseDelay * Math.pow(2, attempt));
            }
        }
    }
}
```

### 5.2 WiFi Network Management

#### 5.2.1 Connection State Machine
```cpp
enum WiFiState {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    RECONNECTING,
    FAILED
};

class WiFiManager {
private:
    WiFiState currentState;
    uint32_t lastConnectionAttempt;
    uint8_t retryCount;
    
public:
    void update();
    bool connect(const char* ssid, const char* password);
    void handleDisconnection();
    WiFiState getState() const { return currentState; }
};
```

## 6. Data Management

### 6.1 Test Data Structure

#### 6.1.1 Data Point Schema
```cpp
struct SensorReading {
    uint32_t timestamp;     // Unix timestamp
    float flowRate;         // L/min
    float inletPressure;    // PSI
    float outletPressure;   // PSI
    float pressureDrop;     // PSI (calculated)
    float totalVolume;      // Gallons (cumulative)
    uint8_t sensorStatus;   // Bit flags for sensor health
};
```

#### 6.1.2 Configuration Persistence
```cpp
struct PersistentConfig {
    uint32_t magic;         // Validation marker
    uint16_t version;       // Config version
    float pressureThreshold;
    uint16_t updateInterval;
    SensorCalibration flowCal;
    SensorCalibration pressure1Cal;
    SensorCalibration pressure2Cal;
    char deviceName[32];
    uint32_t checksum;      // Data integrity
};
```

### 6.2 Memory Optimization

#### 6.2.1 Circular Buffer Implementation
```cpp
// Memory-efficient data storage
class DataBuffer {
private:
    static const size_t BUFFER_SIZE = 500;
    SensorReading buffer[BUFFER_SIZE];
    size_t writeIndex;
    size_t count;
    
public:
    void addReading(const SensorReading& reading) {
        buffer[writeIndex] = reading;
        writeIndex = (writeIndex + 1) % BUFFER_SIZE;
        if (count < BUFFER_SIZE) count++;
    }
    
    SensorReading* getRecentReadings(size_t num) {
        // Return pointer to recent readings
    }
};
```

## 7. Safety and Error Handling

### 7.1 Safety Architecture

#### 7.1.1 Hardware Safety Layer
```cpp
class SafetyMonitor {
private:
    float pressureThreshold;
    uint32_t lastSafetyCheck;
    bool emergencyState;
    
public:
    void checkPressureLimits(float pressure);
    void emergencyShutdown();
    void watchdogReset();
    bool isSafeToOperate();
};
```

#### 7.1.2 Software Watchdog
```cpp
// Prevent system lockup
class SoftwareWatchdog {
private:
    uint32_t lastFeed;
    uint32_t timeout;
    
public:
    void feed() { lastFeed = millis(); }
    void check() {
        if (millis() - lastFeed > timeout) {
            ESP.restart();
        }
    }
};
```

### 7.2 Error Recovery Mechanisms

#### 7.2.1 Sensor Failure Handling
```cpp
enum SensorError {
    SENSOR_OK,
    SENSOR_DISCONNECTED,
    SENSOR_OUT_OF_RANGE,
    SENSOR_NOISY,
    SENSOR_CALIBRATION_ERROR
};

class SensorValidator {
public:
    SensorError validateReading(float voltage, float scaledValue);
    void handleSensorError(SensorError error);
    bool attemptRecovery();
};
```

## 8. Performance Considerations

### 8.1 Real-time Constraints

#### 8.1.1 Timing Requirements
- **Sensor Reading**: 10ms maximum processing time
- **Safety Check**: 1ms maximum response time
- **HTTP Response**: 100ms maximum for status requests
- **Emergency Shutdown**: 50ms maximum from detection to valve closure

#### 8.1.2 Resource Utilization
```cpp
// Performance monitoring
class PerformanceMonitor {
private:
    uint32_t loopStartTime;
    uint32_t maxLoopTime;
    uint16_t memoryUsage;
    
public:
    void startLoop() { loopStartTime = micros(); }
    void endLoop() {
        uint32_t loopTime = micros() - loopStartTime;
        if (loopTime > maxLoopTime) maxLoopTime = loopTime;
    }
    
    void updateMemoryUsage() {
        memoryUsage = ESP.getFreeHeap();
    }
};
```

### 8.2 Scalability Design

#### 8.2.1 Multi-client Support
```cpp
// Handle multiple simultaneous connections
class ClientManager {
private:
    static const uint8_t MAX_CLIENTS = 4;
    WiFiClient clients[MAX_CLIENTS];
    uint32_t lastActivity[MAX_CLIENTS];
    
public:
    int8_t addClient(WiFiClient& client);
    void removeInactiveClients();
    void broadcastUpdate(const String& data);
};
```

## 9. Security Considerations

### 9.1 Network Security

#### 9.1.1 Access Control
- Local network deployment only (no internet exposure)
- No authentication required (trusted network assumption)
- Input validation on all API endpoints
- Rate limiting to prevent abuse

#### 9.1.2 Data Protection
```cpp
// Input validation
class InputValidator {
public:
    bool validatePressureThreshold(float value) {
        return (value >= 5.0f && value <= 100.0f);
    }
    
    bool validateUpdateInterval(float value) {
        return (value >= 0.5f && value <= 120.0f);
    }
    
    bool validateDeviceName(const String& name) {
        return (name.length() <= 32 && isAlphanumeric(name));
    }
};
```

## 10. Testing Strategy

### 10.1 Unit Testing Framework

#### 10.1.1 Embedded Testing
```cpp
// Unit test structure for embedded code
class TestSuite {
private:
    uint16_t testsPassed;
    uint16_t testsFailed;
    
public:
    void runAllTests();
    void assertEqual(float expected, float actual, float tolerance);
    void assertTrue(bool condition, const char* message);
    void printResults();
};
```

### 10.2 Integration Testing

#### 10.2.1 Hardware-in-the-Loop Testing
- Sensor simulation using programmable voltage sources
- Valve operation verification with flow measurement
- Network connectivity stress testing
- Extended operation validation (72+ hours)

### 10.3 Performance Testing

#### 10.3.1 Load Testing
```javascript
// Automated client for load testing
class LoadTestClient {
    constructor(esp32IP, requestsPerSecond) {
        this.baseUrl = `http://${esp32IP}`;
        this.rps = requestsPerSecond;
    }
    
    async runLoadTest(durationSeconds) {
        const interval = 1000 / this.rps;
        const endTime = Date.now() + (durationSeconds * 1000);
        
        while (Date.now() < endTime) {
            await this.makeRequest();
            await this.sleep(interval);
        }
    }
}
```

## 11. Deployment Architecture

### 11.1 Build Process

#### 11.1.1 Firmware Compilation
```
Source Code → Arduino IDE → ESP32 Compiler → Binary Firmware
     ↓              ↓              ↓              ↓
  *.ino files   Preprocessing   Linking      *.bin file
```

#### 11.1.2 Web Assets Embedding
```cpp
// Web assets stored in program memory
const char index_html[] PROGMEM = R"(
<!DOCTYPE html>
<html>
...
</html>
)";
```

### 11.2 Update Mechanism

#### 11.2.1 Over-the-Air Updates
```cpp
// OTA update capability
class OTAManager {
private:
    bool updateInProgress;
    
public:
    bool beginUpdate(size_t updateSize);
    bool writeUpdate(uint8_t* data, size_t length);
    bool endUpdate();
    void rollbackUpdate();
};
```

## 12. Maintenance and Monitoring

### 12.1 Diagnostic Information

#### 12.1.1 System Health Metrics
```cpp
struct SystemHealth {
    uint32_t uptime;           // Seconds since boot
    uint32_t freeMemory;       // Bytes available
    float cpuUtilization;      // Percentage
    int8_t wifiSignalStrength; // dBm
    uint16_t errorCount;       // Cumulative errors
    float temperature;         // Internal temp (if available)
};
```

### 12.2 Logging and Debugging

#### 12.2.1 Debug Output
```cpp
// Configurable debug logging
enum LogLevel {
    LOG_ERROR,
    LOG_WARNING,
    LOG_INFO,
    LOG_DEBUG
};

class Logger {
private:
    LogLevel currentLevel;
    
public:
    void log(LogLevel level, const char* format, ...);
    void setLevel(LogLevel level) { currentLevel = level; }
};
```

This software architecture documentation provides a comprehensive overview of the system design, including component interactions, data flows, safety mechanisms, and performance considerations. It serves as a reference for developers working on the system and provides insight into the technical decisions and design patterns used throughout the project.
