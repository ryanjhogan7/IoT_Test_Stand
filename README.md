# ESP32 Dirt Test Monitor

Automated filtration testing system for laboratory environments. Eliminates manual monitoring and provides continuous measurements over extended periods (72+ hours).

## Overview

Traditional filtration testing requires constant operator monitoring and manual data collection, leading to inconsistent results and excessive labor. This system automates the entire process with real-time monitoring, safety controls, and comprehensive data logging.

## Key Features

- **Automated Operation**: No operator intervention required during testing
- **Extended Testing**: 72+ hour continuous operation capability
- **Real-time Monitoring**: Flow rate (0-25 L/min) and pressure differential (0-100 PSI)
- **Safety Controls**: Automated threshold monitoring and emergency shutdown
- **Data Export**: CSV and clipboard formats with zero data loss
- **Web Interface**: Browser-based dashboard accessible from any device

## Hardware Requirements

### Components
- ESP32-C6 Development Board
- Flow Sensor (0-25 L/min, ±2% accuracy)
- 2x Pressure Sensors (0-100 PSI, ±1% accuracy)
- 24V Solenoid Valve (normally closed)

### Power Supply
- 24VDC for valve operation
- 12VDC for sensor operations
- 3.3VDC for ESP32 (via onboard regulator)

### Pin Configuration
```
GPIO 1  - Inlet Pressure Sensor
GPIO 2  - Outlet Pressure Sensor
GPIO 3  - Flow Sensor
GPIO 10 - Solenoid Valve Control
```

## Quick Start

1. **Hardware Setup**: Connect sensors and valve according to pin configuration
2. **Software**: Upload `main_backend.ino` to ESP32 using Arduino IDE
3. **Configuration**: Update WiFi credentials in code
4. **Access**: Connect to ESP32 IP address via web browser
5. **Testing**: Configure parameters and start automated testing

## API Endpoints

- `GET /api/status` - System status and sensor readings
- `POST /api/start` - Start/resume test
- `POST /api/stop` - Pause test
- `POST /api/reset` - Reset system
- `POST /api/config` - Update configuration
- `POST /api/calibrate` - Sensor calibration

## Performance

- **Uptime**: >99.5% during 72-hour operation
- **Data Integrity**: Zero data loss
- **Response Time**: <100ms emergency shutdown
- **Accuracy**: ±2% flow, ±1% pressure

## Files

- `microcontroller.ino` - ESP32 firmware
- `website.html` - Web interface
- `PRD.md` - Product requirements
- `api_reference.md` - Complete API documentation

## Safety Features

- Automatic pressure threshold monitoring
- Emergency shutdown capability
- Fail-safe valve operation (closes on power loss)
- Real-time visual alerts

## Support

Create an issue in this repository for technical support or questions.
