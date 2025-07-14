# ESP32 Dirt Test Monitor - API Reference

## Base Configuration

**Base URL:** `http://{ESP32_IP_ADDRESS}`  
**Port:** 80  
**Authentication:** None  
**Content-Type:** `application/json` for POST requests

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

## System Control Endpoints

### GET /api/status

Retrieves system status and current sensor readings.

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
  "analogReadings": {
    "flowVoltage": 1.234,
    "inletVoltage": 0.987,
    "outletVoltage": 0.765
  },
  "inputs": {
    "pressure1Scaled": 15.6,
    "pressure2Scaled": 12.8,
    "flowScaled": 3.45,
    "pressure1Raw": 0.987,
    "pressure2Raw": 0.765,
    "flowRaw": 1.234
  },
  "outputs": {
    "solenoid": false
  },
  "scaling": {
    "pressure1Min": 0.0,
    "pressure1Max": 100.0,
    "pressure2Min": 0.0,
    "pressure2Max": 100.0,
    "flowMin": 0.0,
    "flowMax": 25.0
  }
}
```

### POST /api/start

Starts a new test or resumes a paused test.

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
- `pressureThreshold` (float): Maximum pressure before auto-stop (5.0-100.0 PSI)
- `resume` (boolean): true to resume paused test, false for new test

**Response:**
```json
{
  "success": true,
  "message": "Test started successfully",
  "testRunning": true,
  "valveOpen": true,
  "startTime": 1234567890
}
```

### POST /api/stop

Pauses the current test without resetting data.

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

Resets the system to initial state, clearing all test data.

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
  "valveOpen": false,
  "dataCleared": true
}
```

### GET /api/data

Retrieves current measurement data during active test.

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
  "inletPressure": 22.4,
  "outletPressure": 13.7,
  "analogVoltages": {
    "flow": 1.456,
    "inlet": 1.234,
    "outlet": 0.987
  }
}
```

## Configuration Endpoints

### POST /api/config

Updates system configuration parameters.

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
- `updateInterval` (float): Data polling interval (0.5-120.0 seconds)
- `deviceName` (string): Device identifier (max 32 characters)

**Response:**
```json
{
  "success": true,
  "message": "Configuration updated",
  "configurationSaved": true
}
```

### POST /api/scaling

Updates sensor scaling parameters for calibration.

**Request:**
```http
POST /api/scaling
Content-Type: application/json

{
  "pressure1Min": 0.0,
  "pressure1Max": 100.0,
  "pressure2Min": 0.0,
  "pressure2Max": 100.0,
  "flowMin": 0.0,
  "flowMax": 25.0
}
```

**Parameters:**
- `pressure1Min/Max` (float): Inlet pressure sensor range (0-100 PSI)
- `pressure2Min/Max` (float): Outlet pressure sensor range (0-100 PSI)
- `flowMin/Max` (float): Flow sensor range (0-25 L/min)

**Response:**
```json
{
  "success": true,
  "message": "Scaling configuration updated",
  "scalingSaved": true
}
```

### POST /api/calibrate

Performs sensor calibration procedures.

**Request:**
```http
POST /api/calibrate
Content-Type: application/json

{
  "type": "flow",
  "knownValue": 5.25,
  "calibrationPoint": "max",
  "confirmed": true
}
```

**Parameters:**
- `type` (string): Sensor type ("flow", "pressure")
- `knownValue` (float): Reference standard reading
- `calibrationPoint` (string): "min" or "max" calibration point
- `confirmed` (boolean): Final confirmation of calibration

**Response:**
```json
{
  "success": true,
  "message": "Flow sensor calibration completed",
  "calibrationType": "flow",
  "calibrationPoint": "max",
  "referenceValue": 5.25,
  "measuredValue": 5.18,
  "percentError": 1.3
}
```

### POST /api/solenoid

Manually controls the solenoid valve state.

**Request:**
```http
POST /api/solenoid
Content-Type: application/json

{
  "state": true
}
```

**Parameters:**
- `state` (boolean): true to open valve, false to close

**Response:**
```json
{
  "success": true,
  "newState": true,
  "message": "Solenoid ON",
  "valveOpen": true
}
```

## Error Codes

### HTTP Status Codes
- **200 OK**: Request successful
- **400 Bad Request**: Invalid parameters or malformed JSON
- **404 Not Found**: Endpoint not available
- **409 Conflict**: Operation not permitted in current state
- **500 Internal Server Error**: Hardware or system failure
- **503 Service Unavailable**: System not ready

### Common Error Responses

**Invalid Parameters:**
```json
{
  "success": false,
  "error": "Invalid pressure threshold value. Must be between 5.0 and 100.0 PSI",
  "timestamp": 1234567890
}
```

**System Not Ready:**
```json
{
  "success": false,
  "error": "Sensors not online. Check connections and power supply",
  "timestamp": 1234567890
}
```

**Hardware Failure:**
```json
{
  "success": false,
  "error": "Solenoid valve control failure. Check 24V power supply",
  "timestamp": 1234567890
}
```

## Rate Limiting

Recommended request rates for optimal performance:
- **Status/Data polling**: Maximum 2 Hz (every 0.5 seconds)
- **Configuration updates**: Maximum 1 request per second
- **Manual controls**: No specific limits

## CORS Support

All endpoints include CORS headers for browser compatibility:
```
Access-Control-Allow-Origin: *
Access-Control-Allow-Methods: GET, POST, OPTIONS
Access-Control-Allow-Headers: Content-Type, Accept
```

## Usage Examples

### JavaScript Fetch Examples

**Check System Status:**
```javascript
fetch('http://192.168.1.100/api/status')
  .then(response => response.json())
  .then(data => console.log(data));
```

**Start Test:**
```javascript
fetch('http://192.168.1.100/api/start', {
  method: 'POST',
  headers: {'Content-Type': 'application/json'},
  body: JSON.stringify({
    pressureThreshold: 30.0,
    resume: false
  })
})
.then(response => response.json())
.then(data => console.log(data));
```

**Get Real-time Data:**
```javascript
setInterval(() => {
  fetch('http://192.168.1.100/api/data')
    .then(response => response.json())
    .then(data => {
      if (data.success) {
        console.log(`Flow: ${data.flowRate} L/min, Pressure: ${data.pressureDrop} PSI`);
      }
    });
}, 1000);
```

### cURL Examples

**System Status:**
```bash
curl -X GET http://192.168.1.100/api/status
```

**Start Test:**
```bash
curl -X POST http://192.168.1.100/api/start \
  -H "Content-Type: application/json" \
  -d '{"pressureThreshold": 25.0, "resume": false}'
```

**Update Configuration:**
```bash
curl -X POST http://192.168.1.100/api/config \
  -H "Content-Type: application/json" \
  -d '{"pressureThreshold": 30.0, "updateInterval": 2.0}'
```

## Data Types Reference

### Sensor Readings
- **Flow Rate**: float (L/min, 0-25 range)
- **Pressure**: float (PSI, 0-100 range)
- **Volume**: float (gallons, cumulative)
- **Time**: integer (seconds since test start)

### System States
- **Test Running**: boolean (true during active test)
- **Test Paused**: boolean (true when paused)
- **Valve Open**: boolean (true when valve is open)
- **Sensors Online**: boolean (true when all sensors responding)

### Configuration Values
- **Pressure Threshold**: float (5.0-100.0 PSI)
- **Update Interval**: float (0.5-120.0 seconds)
- **Device Name**: string (max 32 characters)

## Notes

- All timestamp values are Unix timestamps (seconds since epoch)
- Sensor readings are updated at the configured interval (default 1 second)
- System automatically saves configuration to EEPROM
- Emergency shutdown occurs automatically when pressure thresholds are exceeded
- Multiple clients can connect simultaneously for monitoring