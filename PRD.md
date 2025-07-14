# ESP32 Dirt Test Monitor - Product Requirements Document

## 1. Executive Summary

### 1.1 Product Vision
The ESP32 Dirt Test Monitor is a real-time monitoring and control system designed for filtration and dirt testing applications. The system provides accurate measurement of flow rates and pressure differentials across test samples while ensuring safety through automated threshold monitoring and emergency shutdown capabilities.

### 1.2 Business Objectives
- **Primary**: Create a reliable, automated monitoring solution for dirt/filtration testing environments
- **Secondary**: Provide real-time data visualization and automated safety controls to reduce manual monitoring requirements
- **Tertiary**: Enable remote monitoring and comprehensive data export capabilities for analysis and reporting

### 1.3 Target Market
- **Primary**: Laboratory technicians and engineers conducting filtration performance tests

### 1.4 Success Criteria
- System uptime of >99.5% during extended test cycles (up to 72 hours continuous operation)
- Zero data loss during normal operation with complete test data integrity
- Maximum test duration capability of 72 hours with continuous data logging
- Automated operation reducing operator intervention by 90%

## 2. Market Analysis

### 2.1 Problem Statement
Current filtration testing solutions require operators to consistently monitor and adjust flow rates while manually taking data measurements of pressure drop and calculating the capacity of a filter from these measurements. Due to fluctuating flow rates, this capacity calculation can be incorrect as well as being a very manual test overall that has too much operator integration. Additionally, commercial solutions are prohibitively expensive for many laboratory environments.

### 2.2 Solution Overview
The ESP32 Dirt Test Monitor addresses these challenges by providing a self-contained, WiFi-enabled monitoring system with browser-based interface, real-time charting, automated safety controls, and comprehensive data export capabilities that eliminates manual monitoring and provides consistent, accurate measurements.

### 2.3 Competitive Analysis
- **Commercial Systems**: High cost ($15,000-$50,000), proprietary software, limited customization
- **Manual Methods**: Labor-intensive, prone to human error, limited data collection, inconsistent flow rates
- **DIY Solutions**: Lack reliability, safety features, and professional interface

### 2.4 Value Proposition
- **Automated Operation**: Eliminates need for constant operator monitoring
- **User Friendly**: Browser-based interface requiring minimal training
- **Safety Focused**: Automated threshold monitoring and emergency shutdown
- **Data Rich**: Comprehensive logging and export capabilities with high precision
- **Flexible**: Configurable parameters and calibration options
- **Extended Operation**: Supports multi-day continuous testing

## 3. Product Requirements

### 3.1 Functional Requirements

#### FR-001: Real-time Sensor Monitoring
**Description**: Continuously monitor flow rate and pressure differential across test samples
**Acceptance Criteria**:
- Update measurements at configurable intervals (0.5-120 seconds)
- Display current values with appropriate units (L/min, PSI)
- Calculate and display pressure drop (inlet - outlet pressure)
- Show cumulative volume in gallons with running total
- Display test elapsed time with pause/resume capability

**Priority**: P0 (Critical)
**Dependencies**: Hardware sensors, ADC functionality

#### FR-002: Automated Safety Management
**Description**: Automatic safety shutdown when pressure thresholds are exceeded
**Acceptance Criteria**:
- Configurable pressure threshold (5-100 PSI, default 20 PSI)
- Immediate valve closure when threshold exceeded (response time <100ms)
- Visual and audible alerts for threshold violations
- Emergency stop capability accessible from main interface
- Automatic test termination with data preservation

**Priority**: P0 (Critical)
**Dependencies**: Solenoid valve control, pressure sensors

#### FR-003: Test Session Management
**Description**: Comprehensive control over test operations with state management
**Acceptance Criteria**:
- Start new test with configurable parameters
- Pause test while maintaining data integrity and valve safety
- Resume paused test from previous state with time adjustment
- Reset all measurements and return system to initial state
- Maintain test state across temporary disconnections

**Priority**: P0 (Critical)
**Dependencies**: System state management, data persistence

#### FR-004: Real-time Data Visualization
**Description**: Dynamic graphical display of measurement trends and system status
**Acceptance Criteria**:
- Dual-axis chart showing flow rate and pressure drop over time
- Secondary chart displaying pressure drop vs. total volume
- Scrolling time series with intelligent data point management
- Color-coded metrics with warning states (yellow at 75% threshold, red at 100%)
- Live data table showing last 15 measurements with timestamp

**Priority**: P1 (High)
**Dependencies**: Canvas API, real-time data updates

#### FR-005: Configuration Management
**Description**: Flexible system parameter configuration with persistence
**Acceptance Criteria**:
- Pressure threshold adjustment (5-100 PSI) with real-time validation
- Update interval configuration (0.5-120 seconds) with immediate effect
- Device naming and identification for multi-unit deployments
- WiFi network configuration management
- All settings persisted in EEPROM across power cycles

**Priority**: P1 (High)
**Dependencies**: EEPROM storage, validation logic

#### FR-006: Sensor Calibration System
**Description**: Field calibration capabilities for analog sensors
**Acceptance Criteria**:
- Two-point calibration for flow and pressure sensors
- Raw voltage display for troubleshooting and verification
- Calibration wizard with step-by-step guidance
- Validation of calibration accuracy with error percentage
- Calibration data persistence and version tracking

**Priority**: P1 (High)
**Dependencies**: Sensor access, calibration algorithms

#### FR-007: Comprehensive Data Export
**Description**: Multiple export formats for data analysis and reporting
**Acceptance Criteria**:
- CSV download with complete dataset including metadata
- Copy-to-clipboard functionality for spreadsheet integration
- Automatic filename generation with timestamp
- Export includes test summary, configuration, and sensor data
- Preserve data precision for analysis (3+ decimal places)

**Priority**: P1 (High)
**Dependencies**: File generation, browser download APIs

#### FR-008: Session Data Management
**Description**: Maintain and display test data during active sessions
**Acceptance Criteria**:
- Store up to 500 data points in memory with overflow management
- Display recent measurements in tabular format
- Preserve data integrity during pause/resume cycles
- Clear data only on explicit user reset command
- Memory-efficient data structures for extended operation (72+ hours)

**Priority**: P2 (Medium)
**Dependencies**: Memory management, data structures

#### FR-009: Network Connectivity Management
**Description**: Reliable wireless connectivity with fault tolerance
**Acceptance Criteria**:
- Connect to specified WiFi network on startup
- Display connection status, signal strength, and network information
- Automatic reconnection after connection loss (30-second timeout)
- Graceful degradation in offline mode with local data retention
- Multiple client support for concurrent monitoring

**Priority**: P2 (Medium)
**Dependencies**: WiFi hardware, network protocols

#### FR-010: Web Interface Design
**Description**: Modern, responsive browser-based control interface
**Acceptance Criteria**:
- Responsive design scaling from 320px to 1920px width
- Real-time updates without page refresh using polling
- Cross-browser compatibility (Chrome, Firefox, Safari, Edge)
- Intuitive navigation with clear visual hierarchy
- Accessibility features including color contrast and keyboard navigation

**Priority**: P2 (Medium)
**Dependencies**: Web technologies, responsive design

### 3.2 Non-Functional Requirements

#### NFR-001: Performance Requirements
**Specifications**:
- Measurement update latency: <1 second
- Web interface response time: <2 seconds
- Emergency shutdown response: <100 milliseconds
- WiFi reconnection time: <30 seconds
- Memory usage: <80% of available ESP32 memory

**Priority**: P0 (Critical)

#### NFR-002: Reliability Requirements
**Specifications**:
- System uptime: >99.5% during extended test cycles (up to 72 hours)
- Data integrity: Zero data loss during normal operation
- Automatic error recovery from transient failures
- Watchdog timer protection against system lockup
- Mean time between failures: >2000 hours

**Priority**: P0 (Critical)

#### NFR-003: Usability Requirements
**Specifications**:
- Training time: <5 minutes for basic operation
- Setup time: <30 minutes for new installation
- Error recovery time: <2 minutes for common issues
- User satisfaction score: >4.0/5.0
- Documentation completeness: 100% feature coverage

**Priority**: P1 (High)

#### NFR-004: Safety Requirements
**Specifications**:
- Fail-safe operation: Valve defaults to closed on power loss
- Automatic shutdown on sensor communication failure
- Hardware-level emergency stop capability
- Pressure relief through immediate valve closure
- Safety response time: <100 milliseconds

**Priority**: P0 (Critical)

## 4. Technical Specifications

### 4.1 Hardware Requirements

#### 4.1.1 Core Processing Unit
- **Microcontroller**: ESP32-C6 development board with WiFi capability
- **Processing Power**: 160 MHz dual-core processor
- **Memory**: 512 KB RAM, 4 MB Flash storage
- **Connectivity**: WiFi 802.11 b/g/n (2.4 GHz)

#### 4.1.2 Sensor Interface
- **ADC Resolution**: 12-bit with 10-sample averaging
- **Analog Inputs**: 3 channels (GPIO 1, 2, 3)
- **Voltage Range**: 0-3.3V with 0.66-3.3V linear response
- **Sample Rate**: Configurable 0.5-120 seconds

#### 4.1.3 Sensor Specifications
- **Flow Sensor**: 0-25 L/min range, ±2% accuracy, analog output
- **Pressure Sensors**: 0-100 PSI range, ±1% accuracy, analog output (2 units)
- **Response Time**: <500 milliseconds for all sensors

#### 4.1.4 Control Systems
- **Solenoid Valve**: 24V normally closed, electrically operated
- **Valve Control**: N-Channel MOSFET (GPIO 10) with hardware protection
- **Safety Features**: Hardware emergency stop, fail-safe closed design

#### 4.1.5 Power Requirements
- **Input Voltage**: 24VDC ±10% for valve operation
- **Sensor Supply**: 12VDC for sensor operations
- **ESP32 Supply**: 3.3VDC via onboard regulator
- **Total Power**: <15W maximum consumption
- **Operating Temperature**: 0°C to +60°C

### 4.2 Software Architecture

#### 4.2.1 Backend (ESP32)
- **Framework**: Arduino IDE with ESP32 core
- **Language**: C++ with Arduino libraries
- **Web Server**: HTTP server on port 80 with CORS support
- **Data Processing**: Real-time sampling at configurable intervals with filtering
- **State Management**: Finite state machine for test control
- **Storage**: EEPROM for configuration persistence

#### 4.2.2 Frontend (Web Interface)
- **Technologies**: HTML5, CSS3, vanilla JavaScript
- **Communication**: HTTP/JSON with polling mechanism
- **Visualization**: Canvas API for real-time charting
- **Responsiveness**: CSS Grid and Flexbox for adaptive layout
- **Data Management**: Client-side data structures with export capabilities

#### 4.2.3 Communication Protocol
- **Network**: WiFi 802.11 b/g/n
- **Protocol**: HTTP/1.1 over TCP/IP
- **Data Format**: JSON for API communication
- **API Style**: RESTful with standard HTTP methods
- **Error Handling**: Comprehensive error codes and messages

### 4.3 API Endpoints

#### 4.3.1 System Control
- `GET /api/status` - System health and sensor readings
- `POST /api/start` - Start/resume test operations
- `POST /api/stop` - Pause test operations
- `POST /api/reset` - Reset system state
- `GET /api/data` - Real-time measurement data

#### 4.3.2 Configuration
- `POST /api/config` - Update system parameters
- `POST /api/scaling` - Sensor calibration and scaling
- `POST /api/calibrate` - Calibration procedures
- `POST /api/solenoid` - Manual valve control

### 4.4 Data Structures

#### 4.4.1 Sensor Data
```json
{
  "timestamp": "ISO 8601 timestamp",
  "flowRate": "float (L/min)",
  "pressureDrop": "float (PSI)",
  "totalVolume": "float (gallons)",
  "elapsedTime": "integer (seconds)"
}
```

#### 4.4.2 System Status
```json
{
  "testRunning": "boolean",
  "testPaused": "boolean",
  "valveOpen": "boolean",
  "sensorsOnline": "boolean",
  "wifiConnected": "boolean"
}
```

## 5. User Experience Design

### 5.1 User Interface Requirements

#### 5.1.1 Dashboard Layout
- **Header**: System status and connection information
- **Metrics Cards**: Real-time display of key measurements
- **Control Panel**: Test start/stop/reset controls with status indicators
- **Charts**: Real-time data visualization with dual-axis capability
- **Data Table**: Recent measurements with export options
- **Settings**: Configuration panel for system parameters

#### 5.1.2 Visual Design
- **Color Scheme**: Dark theme with high contrast for industrial environments
- **Typography**: Sans-serif fonts optimized for readability
- **Status Colors**: Green (normal), yellow (warning), red (critical)
- **Animations**: Smooth transitions and hover effects
- **Responsive**: Mobile-friendly design with touch-optimized controls

#### 5.1.3 Interaction Design
- **Single-Click Actions**: Primary controls require single interaction
- **Confirmation Dialogs**: Destructive actions require confirmation
- **Real-time Feedback**: Immediate visual response to user actions
- **Error Handling**: Clear error messages with suggested solutions
- **Progressive Disclosure**: Advanced features accessible through secondary menus

### 5.2 User Workflows

#### 5.2.1 System Setup Workflow
1. Connect ESP32 to power and WiFi network
2. Access web interface via IP address
3. Configure pressure threshold and update interval
4. Calibrate sensors if required
5. Verify system operation with test run

#### 5.2.2 Test Execution Workflow
1. Prepare test sample and connect to system
2. Configure test parameters (pressure threshold, duration)
3. Start test and monitor real-time data
4. Pause/resume as needed for sample changes
5. Export data upon test completion

#### 5.2.3 Data Analysis Workflow
1. Review test results in real-time charts
2. Examine data table for detailed measurements
3. Export data in preferred format (CSV or clipboard)
4. Import data into analysis software
5. Generate reports and documentation

## 6. Quality Assurance

### 6.1 Testing Strategy

#### 6.1.1 Unit Testing
- Individual sensor reading functions
- Data calculation algorithms
- Configuration management
- API endpoint functionality

#### 6.1.2 Integration Testing
- End-to-end workflow validation
- Hardware-software integration
- Network communication reliability
- Error handling and recovery

#### 6.1.3 Performance Testing
- Extended operation stress testing (72+ hours)
- Memory usage monitoring
- Network connectivity resilience
- Real-time performance validation

#### 6.1.4 Safety Testing
- Emergency shutdown verification
- Fail-safe behavior validation
- Threshold monitoring accuracy
- Hardware protection systems

### 6.2 Validation Criteria

#### 6.2.1 Functional Validation
- All specified features implemented and working
- User workflows complete successfully
- Error conditions handled gracefully
- Performance requirements met

#### 6.2.2 Safety Validation
- Emergency stops respond within 100ms
- Fail-safe valve operation confirmed
- Pressure thresholds enforce automatically
- Hardware protection systems active

#### 6.2.3 User Acceptance Testing
- Setup completed by new users within 30 minutes
- Training requirements under 5 minutes
- User satisfaction scores above 4.0/5.0
- Documentation completeness verified

## 7. Deployment and Support

### 7.1 Installation Requirements

#### 7.1.1 Hardware Setup
- ESP32 programming via USB cable
- Sensor connections and calibration
- Power supply and safety systems
- Network configuration and testing

#### 7.1.2 Software Configuration
- Firmware upload and verification
- WiFi network setup
- Initial system configuration

### 7.2 Documentation Requirements

#### 7.2.1 User Documentation
- Quick start guide with illustrated setup
- Complete user manual with all features
- Troubleshooting guide with common issues
- Video tutorials for key procedures

#### 7.2.2 Technical Documentation
- API reference with examples
- Hardware specifications and schematics
- Software architecture documentation
- Maintenance procedures and schedules

### 7.3 Support Structure

#### 7.3.1 Technical Support
- Email support for technical issues
- Remote diagnostic capabilities
- Firmware update procedures
- Hardware replacement guidance

#### 7.3.2 Maintenance Requirements
- Quarterly sensor calibration
- Annual system verification
- Software updates as available
- Hardware inspection schedule

## 8. Risk Assessment

### 8.1 Technical Risks

#### 8.1.1 Hardware Risks
- **Sensor Drift**: Mitigation through regular calibration and monitoring
- **WiFi Connectivity**: Offline mode and automatic reconnection
- **Power Supply Issues**: UPS recommendation and low-power design
- **Component Failure**: Redundant measurements and error detection

#### 8.1.2 Software Risks
- **Firmware Bugs**: Extensive testing and update mechanism
- **Memory Limitations**: Efficient data structures and garbage collection
- **Network Security**: Local network deployment and access controls
- **Browser Compatibility**: Cross-platform testing and progressive enhancement

### 8.2 Operational Risks

#### 8.2.1 User Risks
- **Configuration Errors**: Input validation and default settings
- **Improper Installation**: Detailed documentation and validation procedures
- **Data Loss**: Real-time export and backup recommendations
- **Safety Incidents**: Multiple safety layers and fail-safe design

## 9. Success Metrics

### 9.1 Technical Performance Metrics
- **System Uptime**: Achieve >99.5% uptime during 72-hour continuous operations
- **Data Integrity**: Zero data loss during normal operation with complete test records
- **Response Time**: Emergency shutdown response <100 milliseconds
- **Extended Operation**: Support continuous testing for 72+ hours without intervention

### 9.2 User Experience Metrics
- **Setup Time**: <30 minutes for new user installation
- **Training Time**: <5 minutes for basic operation
- **User Satisfaction**: >4.0/5.0 satisfaction rating
- **Error Recovery**: <2 minutes for common issue resolution

## 10. Budget and Resources

### 10.1 Hardware Costs (Per Unit)
- ESP32-C6 Development Board: $25
- Sensors (Flow + 2 Pressure): $850
- Solenoid Valve and Control: $200
- Enclosure and Connectors: $50
- **Total Hardware Cost**: $1,125

## 11. Conclusion

The ESP32 Dirt Test Monitor represents a significant advancement in automated filtration testing technology. By eliminating the need for constant operator monitoring and providing precise, continuous measurements over extended periods, this system addresses critical operational challenges in laboratory environments.

The detailed requirements outlined in this document provide a clear roadmap for development, ensuring that the final product meets the needs of laboratory technicians and engineers while maintaining the highest standards of safety and reliability. With its automated operation capabilities and extended testing duration support, the system is positioned to transform how filtration testing is conducted.

Success will be measured primarily by system uptime, data integrity, and the ability to conduct extended multi-day tests without operator intervention. The comprehensive testing strategy and focus on reliability ensure that all requirements are met before deployment.

This project has the potential to significantly improve filtration testing efficiency and accuracy, making automated monitoring capabilities accessible while maintaining the precision and reliability required for critical laboratory applications. The system's capacity for 72+ hour continuous operation with zero data loss will enable more comprehensive testing protocols and better characterization of filter performance over extended periods.
