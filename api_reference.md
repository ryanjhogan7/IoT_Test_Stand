# IoT Test Stand - API Reference v2.1

## Base Configuration

**Base URL:** `http://{ESP32_IP_ADDRESS}`  
**Port:** 80  
**Authentication:** None  
**Content-Type:** `application/json` for POST requests  
**CORS:** Full CORS support enabled for all endpoints

## Response Format

All API responses follow this JSON structure:

```json
{
  "success": true|false,
  "message": "Optional descriptive message",
  "error": "Error description (only when success=false)",
  "timestamp": 1234567890
}
```

## System Status & Control Endpoints

### GET /api/status

Retrieves comprehensive system status, sensor readings, and hardware information.

**Request:**
```http
GET /api/status
```

**Response:**
```json
{
  "success": true,
  "ip": "192.168.1.100",
  "firmware": "v2.1-ESP32C6",
  "uptime": 3600,
  "freeMemory": 245760,
  "wifiConnected": true,
  "wifiRSSI": -45,
  "sensorsOnline": true,
  "testRunning": false,
  "testPaused": false,
  "valveOpen": false,
  "deviceName": "ESP32_C6_Monitor",
  "inputs": {
    "pressure1Scaled": 15.6,
    "pressure1Raw": 0.987,
    "pressure2Scaled": 12.8,
    "pressure2Raw": 0.765,
    "flowScaled": 3.45,
    "flowRaw": 1.234
  },
  "outputs": {
    "solenoid": false
  },
  "scaling": {
    "pressure1Min": 0.0,
    "pressure1Max": 50.0,
    "pressure2Min": 0.0,
    "pressure2Max": 50.0,
    "flowMin": 0.0,
    "flowMax": 10.0
  },
  "analogReadings": {
    "flowVoltage": 1.234,
    "inletVoltage": 0.987,
    "outletVoltage": 0.765
  }
}
```

### POST /api/start

Starts a new test or resumes a paused test with configurable parameters.

**Request:**
```http
POST /api/start
Content-Type: application/json

{
  "pressureThreshold": 20.0,
  "resume": false
}
```

**Parameters:**
- `pressureThreshold` (float, optional): Maximum pressure before auto-stop (5.0-100.0 PSI, default: 20.0)
- `resume` (boolean, optional): true to resume paused test, false for new test (default: false)

**Response:**
```json
{
  "success": true,
  "message": "Test started successfully",
  "testRunning": true,
  "valveOpen": true
}
```

**Behavior:**
- If `resume: true` and test is paused: Resumes test, opens valve, maintains existing data
- If `resume: false` or test not paused: Starts new test, resets all counters and data
- Automatically parses and applies pressure threshold from request body

### POST /api/stop

Pauses the current test, closes valve, but preserves all test data.

**Request:**
```http
POST /api/stop
```

**Response:**
```json
{
  "success": true,
  "message": "Test paused successfully",
  "testRunning": false,
  "testPaused": true,
  "valveOpen": false
}
```

### POST /api/reset

Completely resets the system to initial state, clearing all test data and timers.

**Request:**
```http
POST /api/reset
```

**Response:**
```json
{
  "success": true,
  "message": "System reset successfully",
  "testRunning": false,
  "testPaused": false,
  "valveOpen": false
}
```

**Side Effects:**
- Clears all test data and timing information
- Resets total volume calculation
- Closes solenoid valve
- Returns system to idle state

## Real-time Data Endpoint

### GET /api/data

Retrieves current measurement data with automatic volume calculation and safety monitoring.

**Request:**
```http
GET /api/data
```

**Response:**
```json
{
  "success": true,
  "timestamp": 1234567890,
  "testRunning": true,
  "testPaused": false,
  "elapsedTime": 125,
  "flowRate": 4.23,
  "pressureDrop": 8.7,
  "totalVolume": 8.854,
  "valveOpen": true,
  "pressure1": 22.4,
  "pressure2": 13.7,
  "pressureThreshold": 20.0,
  "analogVoltages": {
    "flow": 1.456,
    "inlet": 1.234,
    "outlet": 0.987
  }
}
```

**Calculated Fields:**
- `pressureDrop`: Absolute difference between pressure1 and pressure2
- `totalVolume`: Cumulative volume in liters (calculated from flow rate × time)
- `elapsedTime`: Seconds since test start (excluding paused duration)

**Automatic Safety Features:**
- Monitors pressure drop against threshold
- Automatically stops test and closes valve if threshold exceeded
- Preserves data when auto-stop occurs

## Configuration Endpoints

### GET /api/config

Retrieves current system configuration and sensor scaling parameters.

**Request:**
```http
GET /api/config
```

**Response:**
```json
{
  "success": true,
  "pressureThreshold": 20.0,
  "updateInterval": 1,
  "deviceName": "ESP32_C6_Monitor",
  "scaling": {
    "pressure1Min": 0.0,
    "pressure1Max": 50.0,
    "pressure2Min": 0.0,
    "pressure2Max": 50.0,
    "flowMin": 0.0,
    "flowMax": 10.0
  }
}
```

### POST /api/config

Updates system configuration parameters with immediate effect.

**Request:**
```http
POST /api/config
Content-Type: application/json

{
  "pressureThreshold": 25.0,
  "updateInterval": 2.0,
  "deviceName": "Lab_Monitor_01"
}
```

**Parameters:**
- `pressureThreshold` (float): Auto-stop pressure limit (5.0-100.0 PSI)
- `updateInterval` (float): Reserved for future use
- `deviceName` (string): Device identifier (display only)

**Response:**
```json
{
  "success": true,
  "message": "Configuration updated"
}
```

### POST /api/scaling

Updates sensor scaling parameters for voltage-to-engineering unit conversion.

**Request:**
```http
POST /api/scaling
Content-Type: application/json

{
  "pressure1Min": 0.0,
  "pressure1Max": 50.0,
  "pressure2Min": 0.0,
  "pressure2Max": 50.0,
  "flowMin": 0.0,
  "flowMax": 10.0
}
```

**Parameters:**
- `pressure1Min/Max` (float): Inlet pressure sensor range (PSI)
- `pressure2Min/Max` (float): Outlet pressure sensor range (PSI)
- `flowMin/Max` (float): Flow sensor range (L/min)

**Scaling Algorithm:**
- Input voltage range: 0.66V - 3.30V (linear)
- Output: `minVal + ((voltage - 0.66) / (3.30 - 0.66)) × (maxVal - minVal)`
- Voltages below 0.66V or above 3.30V are clamped to valid range

**Response:**
```json
{
  "success": true,
  "message": "Scaling updated"
}
```

## Hardware Control Endpoints

### POST /api/solenoid

Manually controls the solenoid valve state (independent of test operation).

**Request:**
```http
POST /api/solenoid
Content-Type: application/json

{
  "state": true
}
```

**Parameters:**
- `state` (boolean): true to open valve, false to close valve

**Response:**
```json
{
  "success": true,
  "newState": true,
  "message": "Solenoid ON"
}
```

**Safety Notes:**
- Manual control overrides test operation
- Use with caution during active tests
- Primarily intended for calibration and maintenance

### POST /api/calibrate

Handles sensor calibration procedures and validation.

**Request:**
```http
POST /api/calibrate
Content-Type: application/json

{
  "type": "flow",
  "actualValue": 5.25,
  "calculatedValue": 5.18,
  "percentError": 1.3,
  "confirmed": true
}
```

**Parameters:**
- `type` (string): Sensor type ("flow", "pressure1", "pressure2")
- `actualValue` (float): Reference standard reading
- `calculatedValue` (float): System calculated value
- `percentError` (float): Calculated accuracy error percentage
- `confirmed` (boolean): Final confirmation of calibration acceptance

**Response:**
```json
{
  "success": true,
  "message": "Calibration process completed"
}
```

**Calibration Logic:**
- Logs all calibration data to serial console
- Accepts calibrations with ≤5% error automatically
- Stores calibration results for audit trail
- Future enhancement: Automatic scaling factor adjustment

## Hardware Configuration

### Pin Assignments (ESP32-C6)
```
GPIO 1  - Pressure Sensor 1 (Inlet) - ADC1_CH1
GPIO 2  - Pressure Sensor 2 (Outlet) - ADC1_CH2  
GPIO 5  - Flow Sensor - ADC1_CH5
GPIO 10 - Solenoid Valve Control - Digital Output
```

### ADC Configuration
- **Resolution**: 12-bit (0-4095 counts)
- **Attenuation**: ADC_11db (0-3.3V range)
- **Voltage Conversion**: (raw / 4095.0) × 3.3V
- **Sensor Range**: 0.66V - 3.30V linear response

## Error Handling

### HTTP Status Codes
- **200 OK**: Request successful
- **400 Bad Request**: Invalid parameters or malformed JSON
- **404 Not Found**: Endpoint not available  
- **500 Internal Server Error**: Hardware or system failure

### Common Error Responses

**Endpoint Not Found:**
```json
{
  "success": false,
  "error": "Endpoint not found: /api/invalid"
}
```

**System State Conflict:**
```json
{
  "success": false,
  "error": "Cannot start test while another test is running"
}
```

**Hardware Failure:**
```json
{
  "success": false,
  "error": "Sensor communication failure. Check connections."
}
```

## CORS Support

All endpoints include comprehensive CORS headers:
```
Access-Control-Allow-Origin: *
Access-Control-Allow-Methods: GET, POST, OPTIONS
Access-Control-Allow-Headers: Content-Type, Accept
Access-Control-Max-Age: 86400
```

**OPTIONS Preflight:** All endpoints support OPTIONS method for preflight requests.

## Rate Limiting & Performance

### Recommended Request Rates
- **Status/Data polling**: Maximum 2 Hz (every 0.5 seconds)
- **Configuration updates**: Maximum 1 request per second
- **Manual controls**: No specific limits, but allow time for hardware response

### Performance Characteristics
- **Response Time**: <100ms for most endpoints
- **Memory Usage**: <80% of ESP32 available memory
- **Concurrent Connections**: Supports multiple simultaneous clients
- **Uptime**: >99.5% during extended operation

## Usage Examples

### JavaScript Fetch API

**Real-time Data Polling:**
```javascript
async function pollData() {
  try {
    const response = await fetch('http://192.168.1.100/api/data');
    const data = await response.json();
    
    if (data.success) {
      console.log(`Flow: ${data.flowRate} L/min, Pressure: ${data.pressureDrop} PSI`);
      
      // Check for auto-shutdown
      if (!data.testRunning && data.testPaused) {
        console.log('Test auto-stopped due to pressure threshold');
      }
    }
  } catch (error) {
    console.error('Data fetch failed:', error);
  }
}

// Poll every second
setInterval(pollData, 1000);
```

**Test Control with Error Handling:**
```javascript
async function startTest(pressureThreshold = 20.0) {
  try {
    const response = await fetch('http://192.168.1.100/api/start', {
      method: 'POST',
      headers: {'Content-Type': 'application/json'},
      body: JSON.stringify({
        pressureThreshold: pressureThreshold,
        resume: false
      })
    });
    
    const result = await response.json();
    
    if (result.success) {
      console.log('Test started successfully');
      return true;
    } else {
      console.error('Test start failed:', result.error);
      return false;
    }
  } catch (error) {
    console.error('Network error:', error);
    return false;
  }
}
```

### cURL Examples

**System Status:**
```bash
curl -X GET http://192.168.1.100/api/status | jq '.'
```

**Start Test with Custom Threshold:**
```bash
curl -X POST http://192.168.1.100/api/start \
  -H "Content-Type: application/json" \
  -d '{"pressureThreshold": 30.0, "resume": false}'
```

**Update Sensor Scaling:**
```bash
curl -X POST http://192.168.1.100/api/scaling \
  -H "Content-Type: application/json" \
  -d '{
    "pressure1Min": 0.0,
    "pressure1Max": 100.0,
    "pressure2Min": 0.0, 
    "pressure2Max": 100.0,
    "flowMin": 0.0,
    "flowMax": 25.0
  }'
```

## Data Types & Validation

### Sensor Readings
- **Flow Rate**: float (L/min, range determined by scaling parameters)
- **Pressure**: float (PSI, range determined by scaling parameters)  
- **Volume**: float (liters, cumulative calculation)
- **Time**: integer (seconds since test start)
- **Voltage**: float (volts, 0.0-3.3V range)

### System States
- **Test Running**: boolean (true during active test with open valve)
- **Test Paused**: boolean (true when paused, preserving data)
- **Valve Open**: boolean (true when solenoid is energized)
- **Sensors Online**: boolean (true when all sensors responding)

### Configuration Constraints
- **Pressure Threshold**: 5.0-100.0 PSI (validated on input)
- **Scaling Min/Max**: Must satisfy min < max relationship
- **Device Name**: String, display purposes only

## Implementation Notes

### Automatic Safety Features
1. **Pressure Monitoring**: Continuous comparison of pressure drop vs. threshold
2. **Auto-Shutdown**: Immediate test stop and valve closure when threshold exceeded  
3. **Data Preservation**: All data maintained during emergency stops
4. **State Consistency**: System maintains consistent state across all operations

### Memory Management
- **Circular Buffers**: Used for sensor data storage with automatic overflow handling
- **Static Allocation**: All major data structures pre-allocated to prevent memory fragmentation
- **Garbage Collection**: Minimal dynamic allocation to prevent memory leaks

### Network Reliability
- **Connection Monitoring**: Automatic detection of client disconnections
- **Retry Logic**: Built-in resilience for temporary network issues
- **Multiple Clients**: Support for concurrent monitoring sessions

This API provides a comprehensive interface for automated filtration testing with built-in safety features, real-time monitoring, and flexible configuration options suitable for laboratory environments requiring precision and reliability.
