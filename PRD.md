# IoT Test Stand - Product Requirements Document

## 1. Executive Summary

### 1.1 Product Vision
The IoT Test Stand is a real-time monitoring and control system designed for automated filtration and flow testing applications in laboratory environments. The system provides accurate measurement of flow rates and pressure differentials across test samples while ensuring safety through automated threshold monitoring and emergency shutdown capabilities.

### 1.2 Business Objectives
- **Primary**: Create a reliable, automated monitoring solution for laboratory filtration and flow testing environments
- **Secondary**: Provide real-time data visualization and automated safety controls to reduce manual monitoring requirements by 90%
- **Tertiary**: Enable remote monitoring and comprehensive data export capabilities for analysis and reporting with zero data loss

### 1.3 Target Market
- **Primary**: Laboratory technicians and engineers conducting filtration performance tests
- **Secondary**: Research facilities requiring automated flow and pressure monitoring
- **Tertiary**: Industrial quality control environments

### 1.4 Success Criteria
- System uptime of >99.5% during extended test cycles (up to 72 hours continuous operation)
- Zero data loss during normal operation with complete test data integrity
- Maximum test duration capability of 72+ hours with continuous data logging
- Automated operation reducing operator intervention by 90%
- Setup time <30 minutes for new installations

## 2. Market Analysis

### 2.1 Problem Statement
Current laboratory testing solutions require operators to consistently monitor and adjust flow rates while manually recording pressure measurements and calculating filter capacity. This manual approach leads to:
- **Inconsistent Results**: Human variability in timing and measurement recording
- **Labor Intensive**: Constant operator presence required for multi-hour tests
- **Data Loss Risk**: Manual recording prone to errors and gaps
- **Limited Duration**: Practical test length limited by operator availability
- **High Cost**: Commercial automated solutions cost $15,000-$50,000

### 2.2 Solution Overview
The IoT Test Stand addresses these challenges by providing a self-contained, WiFi-enabled monitoring system with:
- Browser-based interface accessible from any device
- Real-time charting and data visualization
- Automated safety controls with configurable thresholds
- Comprehensive data export capabilities
- Extended operation capability (72+ hours unattended)
- Cost-effective implementation (<$1,500 per unit)

### 2.3 Competitive Analysis
- **Commercial Systems**: 
  - Cost: $15,000-$50,000
  - Pros: Professional support, validated systems
  - Cons: High cost, proprietary software, limited customization
- **Manual Methods**:
  - Cost: $500-$1,000 (basic instrumentation)
  - Pros: Low initial cost, simple setup
  - Cons: Labor-intensive, prone to human error, limited data collection
- **DIY Solutions**:
  - Cost: $500-$2,000
  - Pros: Customizable, moderate cost
  - Cons: Lack reliability, safety features, and professional interface

### 2.4 Value Proposition
- **Cost Effective**: 90% cost reduction compared to commercial systems
- **Automated Operation**: Eliminates need for constant operator monitoring
- **User Friendly**: Browser-based interface requiring minimal training (<5 minutes)
- **Safety Focused**: Automated threshold monitoring and emergency shutdown (<100ms response)
- **Data Rich**: Comprehensive logging and export capabilities with high precision
- **Flexible**: Configurable parameters and calibration options
- **Extended Operation**: Supports multi-day continuous testing (72+ hours)

## 3. Product Requirements

### 3.1 Functional Requirements

#### FR-001: Real-time Sensor Monitoring
**Description**: Continuously monitor flow rate and pressure differential across test samples
**Acceptance Criteria**:
- Update measurements at configurable intervals (0.5-120 seconds for data logging)
- Display real-time values with appropriate units (L/min, PSI)
- Calculate and display pressure drop (absolute difference between inlet and outlet pressure)
- Show cumulative volume in gallons with running total integration
- Display test elapsed time with pause/resume capability

**Priority**: P0 (Critical)
**Dependencies**: Hardware sensors, ADC functionality, ESP32-C6 platform

#### FR-002: Automated Safety Management
**Description**: Automatic safety shutdown when pressure thresholds are exceeded
**Acceptance Criteria**:
- Configurable pressure threshold (5-100 PSI, default 20 PSI)
- Immediate valve closure when threshold exceeded (response time <100ms)
- Visual alerts for threshold violations with clear status indication
- Emergency stop capability accessible from main interface
- Automatic test termination with complete data preservation

**Priority**: P0 (Critical)
**Dependencies**: Solenoid valve control, pressure sensors, GPIO control

#### FR-003: Test Session Management
**Description**: Comprehensive control over test operations with state management
**Acceptance Criteria**:
- Start new test with configurable parameters (pressure threshold)
- Pause test while maintaining data integrity and valve safety
- Resume paused test from previous state with accurate time adjustment
- Reset all measurements and return system to initial state
- Maintain test state consistency across all operations

**Priority**: P0 (Critical)
**Dependencies**: System state management, timing functions

#### FR-004: Real-time Data Visualization
**Description**: Dynamic graphical display of measurement trends and system status
**Acceptance Criteria**:
- Dual-axis chart showing flow rate and pressure drop over time
- Intelligent data point management displaying up to 15 evenly-spaced points
- Scrolling time series with automatic scaling and axis management
- Color-coded metrics with warning states (yellow at 75% threshold, red at 100%)
- Live data table showing last 15 measurements with precise timestamps

**Priority**: P1 (High)
**Dependencies**: Canvas API, real-time data updates, JavaScript charting

#### FR-005: Configuration Management
**Description**: Flexible system parameter configuration with immediate effect
**Acceptance Criteria**:
- Pressure threshold adjustment (5-100 PSI) with real-time validation
- Data logging interval configuration (0.5-120 seconds) with immediate effect
- Device naming and identification for multi-unit deployments
- Sensor scaling parameter adjustment (min/max ranges per sensor)
- All critical settings applied immediately without restart

**Priority**: P1 (High)
**Dependencies**: Input validation, parameter persistence

#### FR-006: Sensor Calibration System
**Description**: Field calibration capabilities for analog sensors with validation
**Acceptance Criteria**:
- Scaling parameter adjustment for voltage-to-engineering unit conversion
- Raw voltage display for troubleshooting and verification (0.66V-3.30V range)
- Calibration validation with accuracy percentage calculation
- Manual valve control for calibration procedures
- Real-time sensor reading display during calibration

**Priority**: P1 (High)
**Dependencies**: Sensor access, scaling algorithms, validation logic

#### FR-007: Comprehensive Data Export
**Description**: Multiple export formats for data analysis and reporting
**Acceptance Criteria**:
- CSV download with complete dataset including comprehensive metadata
- Copy-to-clipboard functionality for direct spreadsheet integration
- Automatic filename generation with timestamp and test parameters
- Export includes test summary, configuration, and high-precision sensor data
- Data preserved with 3+ decimal places for analysis accuracy

**Priority**: P1 (High)
**Dependencies**: File generation, browser download APIs, clipboard access

#### FR-008: Session Data Management
**Description**: Maintain and display test data during active sessions
**Acceptance Criteria**:
- Store up to 2000 data points in memory with automatic overflow management
- Display recent measurements in tabular format (last 15 entries)
- Preserve complete data integrity during pause/resume cycles
- Clear data only on explicit user reset command
- Memory-efficient data structures for extended operation (72+ hours)

**Priority**: P2 (Medium)
**Dependencies**: Memory management, circular buffer implementation

#### FR-009: Network Connectivity Management
**Description**: Reliable wireless connectivity with fault tolerance
**Acceptance Criteria**:
- Connect to specified WiFi network on startup with status reporting
- Display connection status, signal strength (RSSI), and network information
- Support multiple concurrent client connections for monitoring
- Graceful operation during temporary network disruptions
- Real-time connection status updates

**Priority**: P2 (Medium)
**Dependencies**: WiFi hardware, network protocols, ESP32-C6 connectivity

#### FR-010: Web Interface Design
**Description**: Modern, responsive browser-based control interface
**Acceptance Criteria**:
- Responsive design scaling from mobile (320px) to desktop (1920px) width
- Real-time updates without page refresh using polling architecture
- Cross-browser compatibility (Chrome, Firefox, Safari, Edge)
- Intuitive navigation with clear visual hierarchy and status indicators
- Accessibility features including sufficient color contrast

**Priority**: P2 (Medium)
**Dependencies**: Web technologies, responsive CSS design

### 3.2 Non-Functional Requirements

#### NFR-001: Performance Requirements
**Specifications**:
- Real-time display update latency: <500ms
- Web interface response time: <2 seconds for all operations
- Emergency shutdown response: <100 milliseconds from detection to valve closure
- WiFi reconnection time: <30 seconds after network restoration
- Memory usage: <80% of available ESP32-C6 memory during extended operation

**Priority**: P0 (Critical)

#### NFR-002: Reliability Requirements
**Specifications**:
- System uptime: >99.5% during extended test cycles (up to 72 hours)
- Data integrity: Zero data loss during normal operation conditions
- Automatic error recovery from transient network and sensor failures
- Mean time between failures: >2000 hours of continuous operation
- Graceful degradation during partial system failures

**Priority**: P0 (Critical)

#### NFR-003: Usability Requirements
**Specifications**:
- Training time: <5 minutes for basic operation (start/stop/export)
- Setup time: <30 minutes for new installation including sensor calibration
- Error recovery time: <2 minutes for common issues using documentation
- User satisfaction score: >4.0/5.0 in laboratory environment testing
- Documentation completeness: 100% feature coverage with examples

**Priority**: P1 (High)

#### NFR-004: Safety Requirements
**Specifications**:
- Fail-safe operation: Valve defaults to closed state on any power loss
- Automatic shutdown on sensor communication failure or invalid readings
- Hardware-level emergency stop capability independent of software
- Pressure relief through immediate valve closure on threshold violation
- Safety response time: <100 milliseconds for all critical safety functions

**Priority**: P0 (Critical)

## 4. Technical Specifications

### 4.1 Hardware Requirements

#### 4.1.1 Core Processing Unit
- **Microcontroller**: ESP32-C6 development board with integrated WiFi capability
- **Processing Power**: 160 MHz dual-core RISC-V processor
- **Memory**: 512 KB SRAM, 4 MB Flash storage
- **Connectivity**: WiFi 802.11 b/g/n (2.4 GHz) with RSSI monitoring

#### 4.1.2 Sensor Interface
- **ADC Resolution**: 12-bit (0-4095 counts) with 11dB attenuation
- **Analog Inputs**: 3 channels (GPIO 1, 2, 5)
- **Input Voltage Range**: 0-3.3V with 0.66-3.3V linear sensor response
- **Sample Rate**: Real-time polling with configurable logging intervals

#### 4.1.3 Sensor Specifications
- **Flow Sensor**: 0-25 L/min range (configurable), ±2% accuracy, analog output
- **Pressure Sensors**: 0-100 PSI range (configurable), ±1% accuracy, analog output (2 units)
- **Response Time**: <500 milliseconds for all sensors
- **Signal Range**: 0.66V-3.30V linear response (4-20mA equivalent)

#### 4.1.4 Control Systems
- **Solenoid Valve**: 24V normally closed, electrically operated
- **Valve Control**: N-Channel MOSFET via GPIO 10 with hardware protection
- **Safety Features**: Hardware emergency stop, fail-safe closed design
- **Response Time**: <50ms valve operation time

#### 4.1.5 Power Requirements
- **Input Voltage**: 24VDC ±10% for valve operation
- **Sensor Supply**: 12VDC for sensor operations (if required)
- **ESP32 Supply**: 3.3VDC via onboard regulator from USB or external supply
- **Total Power**: <15W maximum consumption during operation
- **Operating Temperature**: 0°C to +60°C (laboratory environment)

### 4.2 Software Architecture

#### 4.2.1 Backend (ESP32-C6)
- **Framework**: Arduino IDE with ESP32 core v2.0+
- **Language**: C++ with Arduino libraries and minimal external dependencies
- **Web Server**: Built-in HTTP server on port 80 with full CORS support
- **Data Processing**: Real-time sampling with configurable intervals and safety monitoring
- **State Management**: Simple state variables with atomic operations
- **Memory Management**: Static allocation patterns for predictable performance

#### 4.2.2 Frontend (Web Interface)
- **Technologies**: HTML5, CSS3, vanilla JavaScript (no external dependencies)
- **Communication**: HTTP/JSON with 500ms polling for real-time updates
- **Visualization**: Canvas API for real-time charting with automatic scaling
- **Responsiveness**: CSS Grid and Flexbox for adaptive layout across devices
- **Data Management**: Client-side structures with efficient export capabilities

#### 4.2.3 Communication Protocol
- **Network**: WiFi 802.11 b/g/n with automatic connection management
- **Protocol**: HTTP/1.1 over TCP/IP with RESTful API design
- **Data Format**: JSON for all API communication with consistent structure
- **API Style**: RESTful with standard HTTP methods and clear endpoints
- **Error Handling**: Comprehensive error codes and descriptive messages

### 4.3 API Endpoints

#### 4.3.1 System Control
- `GET /api/status` - Comprehensive system health and sensor readings
- `POST /api/start` - Start/resume test operations with configurable parameters
- `POST /api/stop` - Pause test operations while preserving data
- `POST /api/reset` - Complete system reset to initial state
- `GET /api/data` - Real-time measurement data with safety monitoring

#### 4.3.2 Configuration
- `GET /api/config` - Current system configuration and parameters
- `POST /api/config` - Update system parameters with validation
- `POST /api/scaling` - Sensor calibration and scaling parameters
- `POST /api/calibrate` - Calibration procedures and validation
- `POST /api/solenoid` - Manual valve control for maintenance and calibration

### 4.4 Data Structures

#### 4.4.1 Sensor Data Format
```json
{
  "timestamp": "Unix timestamp (milliseconds)",
  "flowRate": "float (L/min, 3 decimal precision)",
  "pressureDrop": "float (PSI, 2 decimal precision)",
  "totalVolume": "float (liters, 4 decimal precision)",
  "elapsedTime": "integer (seconds since test start)"
}
```

#### 4.4.2 System Status Format
```json
{
  "testRunning": "boolean (true during active test)",
  "testPaused": "boolean (true when paused)",
  "valveOpen": "boolean (solenoid state)",
  "sensorsOnline": "boolean (all sensors responding)",
  "wifiConnected": "boolean (network connectivity)"
}
```

## 5. User Experience Design

### 5.1 User Interface Requirements

#### 5.1.1 Dashboard Layout
- **Header**: System status, connection information, and device identification
- **Metrics Cards**: Large, clear display of key measurements with appropriate units
- **Control Panel**: Prominent test start/stop/reset controls with status indicators
- **Charts**: Real-time data visualization with dual-axis capability and legends
- **Data Table**: Recent measurements with export options and clear formatting
- **Settings**: Collapsible configuration panel for system parameters and calibration

#### 5.1.2 Visual Design
- **Color Scheme**: Professional dark theme with high contrast for laboratory environments
- **Typography**: Sans-serif fonts optimized for readability at various sizes
- **Status Colors**: Consistent color coding (Green=normal, Yellow=warning, Red=critical)
- **Animations**: Smooth transitions and hover effects for enhanced user experience
- **Responsive**: Mobile-friendly design with touch-optimized controls

#### 5.1.3 Interaction Design
- **Single-Click Actions**: Primary controls require only single interaction
- **Confirmation Dialogs**: Destructive actions (reset) require explicit confirmation
- **Real-time Feedback**: Immediate visual response to all user actions
- **Error Handling**: Clear error messages with suggested solutions
- **Progressive Disclosure**: Advanced features accessible through secondary menus

### 5.2 User Workflows

#### 5.2.1 System Setup Workflow
1. Connect ESP32-C6 to power and ensure WiFi network access
2. Access web interface via ESP32 IP address from any browser
3. Configure pressure threshold and data logging interval as needed
4. Calibrate sensors using known reference values if required
5. Verify system operation with brief test run

#### 5.2.2 Test Execution Workflow
1. Prepare test sample and connect to pneumatic/hydraulic system
2. Configure test parameters (pressure threshold, expected duration)
3. Start test and monitor real-time data via charts and metrics
4. Pause/resume as needed for sample changes or observations
5. Export comprehensive data upon test completion

#### 5.2.3 Data Analysis Workflow
1. Review test results using real-time charts and trend analysis
2. Examine detailed data table for specific measurements and timestamps
3. Export data in preferred format (CSV download or clipboard copy)
4. Import data into analysis software (Excel, MATLAB, Python, etc.)
5. Generate reports and documentation using exported datasets

## 6. Quality Assurance

### 6.1 Testing Strategy

#### 6.1.1 Unit Testing
- Individual sensor reading functions with known voltage inputs
- Data calculation algorithms (pressure drop, volume integration)
- Configuration management and parameter validation
- API endpoint functionality and response formatting

#### 6.1.2 Integration Testing
- Complete end-to-end workflow validation (start → data → stop → reset)
- Hardware-software integration with actual sensors and valve
- Network communication reliability under various conditions
- Error handling and recovery from simulated failure conditions

#### 6.1.3 Performance Testing
- Extended operation stress testing (72+ hours continuous)
- Memory usage monitoring and leak detection
- Network connectivity resilience testing
- Real-time performance validation under load

#### 6.1.4 Safety Testing
- Emergency shutdown verification with simulated over-pressure conditions
- Fail-safe behavior validation (power loss, network loss)
- Threshold monitoring accuracy with calibrated pressure sources
- Hardware protection systems validation

### 6.2 Validation Criteria

#### 6.2.1 Functional Validation
- All specified features implemented and working as documented
- User workflows complete successfully within specified time limits
- Error conditions handled gracefully with appropriate user feedback
- Performance requirements met under normal and stress conditions

#### 6.2.2 Safety Validation
- Emergency stops respond within 100ms specification
- Fail-safe valve operation confirmed under all failure modes
- Pressure thresholds enforce automatically with <1% error
- Hardware protection systems prevent damage under fault conditions

#### 6.2.3 User Acceptance Testing
- Setup completed by new users within 30-minute specification
- Training requirements under 5-minute specification verified
- User satisfaction scores above 4.0/5.0 in laboratory environment
- Documentation completeness verified by independent testing

## 7. Deployment and Support

### 7.1 Installation Requirements

#### 7.1.1 Hardware Setup
- ESP32-C6 programming via USB cable using Arduino IDE
- Sensor connections and initial calibration verification
- Power supply configuration and safety systems check
- Network configuration and IP address assignment

#### 7.1.2 Software Configuration
- Firmware upload and verification with serial monitor
- WiFi network credentials configuration
- Initial system configuration and parameter setup
- Sensor calibration using known reference standards

### 7.2 Documentation Requirements

#### 7.2.1 User Documentation
- Quick start guide with illustrated setup procedures (≤4 pages)
- Complete user manual covering all features and workflows
- Troubleshooting guide with common issues and solutions
- Video tutorials for key procedures (setup, calibration, operation)

#### 7.2.2 Technical Documentation
- Complete API reference with examples and error codes
- Hardware specifications and connection diagrams
- Software architecture documentation with component descriptions
- Maintenance procedures and calibration schedules

### 7.3 Support Structure

#### 7.3.1 Technical Support
- Email support for technical issues with <24 hour response
- Remote diagnostic capabilities via web interface
- Firmware update procedures and version management
- Hardware replacement guidance and part specifications

#### 7.3.2 Maintenance Requirements
- Quarterly sensor calibration verification recommended
- Annual system verification and performance testing
- Software updates provided as available with change logs
- Hardware inspection schedule for critical components

## 8. Risk Assessment

### 8.1 Technical Risks

#### 8.1.1 Hardware Risks
- **Sensor Drift**: 
  - Risk: Gradual calibration changes over time
  - Mitigation: Regular calibration procedures and drift monitoring
- **WiFi Connectivity**: 
  - Risk: Network interruptions during critical tests
  - Mitigation: Local data storage and automatic reconnection
- **Power Supply Issues**: 
  - Risk: Interruption during extended tests
  - Mitigation: UPS recommendation and low-power design
- **Component Failure**: 
  - Risk: Sensor or valve malfunction
  - Mitigation: Redundant measurements and comprehensive error detection

#### 8.1.2 Software Risks
- **Memory Limitations**: 
  - Risk: Memory exhaustion during extended operation
  - Mitigation: Efficient data structures and circular buffer management
- **Network Security**: 
  - Risk: Unauthorized access to system controls
  - Mitigation: Local network deployment and access controls
- **Browser Compatibility**: 
  - Risk: Interface issues on different platforms
  - Mitigation: Cross-platform testing and progressive enhancement

### 8.2 Operational Risks

#### 8.2.1 User Risks
- **Configuration Errors**: 
  - Risk: Incorrect parameter settings
  - Mitigation: Input validation, safe defaults, and clear documentation
- **Improper Installation**: 
  - Risk: Incorrect sensor connections or setup
  - Mitigation: Detailed documentation, validation procedures, and visual indicators
- **Data Loss**: 
  - Risk: Loss of test data due to user error or system failure
  - Mitigation: Real-time export capabilities and backup recommendations
- **Safety Incidents**: 
  - Risk: Over-pressure or equipment damage
  - Mitigation: Multiple safety layers, fail-safe design, and clear warnings

## 9. Success Metrics

### 9.1 Technical Performance Metrics
- **System Uptime**: Achieve >99.5% uptime during 72-hour continuous operations
- **Data Integrity**: Zero data loss during normal operation with complete audit trail
- **Response Time**: Emergency shutdown response consistently <100 milliseconds
- **Extended Operation**: Support continuous testing for 72+ hours without intervention
- **Memory Efficiency**: Maintain <80% memory usage throughout extended operation

### 9.2 User Experience Metrics
- **Setup Time**: <30 minutes for new user installation including calibration
- **Training Time**: <5 minutes for basic operation (verified through user testing)
- **User Satisfaction**: >4.0/5.0 satisfaction rating in laboratory environment
- **Error Recovery**: <2 minutes for common issue resolution using documentation
- **Feature Adoption**: >90% of users utilizing export and configuration features

### 9.3 Business Impact Metrics
- **Cost Reduction**: 90% reduction compared to commercial alternatives
- **Time Savings**: 90% reduction in manual monitoring requirements
- **Test Duration**: Enable 10x longer unattended test periods
- **Data Quality**: 95% reduction in data recording errors
- **Deployment Rate**: Support laboratory adoption within 1 month of introduction

## 10. Budget and Resources

### 10.1 Hardware Costs (Per Unit)
- ESP32-C6 Development Board: $25
- Flow Sensor (0-25 L/min): $400
- Pressure Sensors (2x, 0-100 PSI): $450
- Solenoid Valve and Control Electronics: $200
- Enclosure, Connectors, and Accessories: $75
- **Total Hardware Cost**: $1,150 per unit

### 10.2 Development Resources
- **Software Development**: 160 hours (firmware + web interface)
- **Testing and Validation**: 80 hours (unit, integration, performance testing)
- **Documentation**: 40 hours (user manual, API reference, setup guides)
- **Total Development**: 280 hours

### 10.3 Return on Investment
- **Commercial Alternative Cost**: $25,000-$50,000
- **IoT Test Stand Cost**: $1,150 + development amortization
- **Cost Savings**: 90-95% reduction per unit
- **Payback Period**: <3 months for typical laboratory deployment

## 11. Conclusion

The IoT Test Stand represents a significant advancement in automated laboratory testing technology. By eliminating the need for constant operator monitoring and providing precise, continuous measurements over extended periods, this system addresses critical operational challenges in research and quality control environments.

The detailed requirements outlined in this document provide a clear roadmap for development, ensuring that the final product meets the needs of laboratory professionals while maintaining the highest standards of safety and reliability. With its automated operation capabilities and extended testing duration support, the system is positioned to transform how filtration and flow testing is conducted in laboratory settings.

Success will be measured primarily by system uptime, data integrity, user adoption, and the ability to conduct extended multi-day tests without operator intervention. The comprehensive testing strategy and focus on reliability ensure that all requirements are met before deployment.

This project has the potential to significantly improve laboratory testing efficiency and accuracy, making automated monitoring capabilities accessible to a broader range of research facilities while maintaining the precision and reliability required for critical applications. The system's capacity for 72+ hour continuous operation with zero data loss will enable more comprehensive testing protocols and better characterization of filter and flow system performance over extended periods.

The combination of cost-effectiveness, ease of use, and professional-grade capabilities positions the IoT Test Stand as an ideal solution for laboratories seeking to modernize their testing capabilities without the prohibitive costs associated with traditional commercial systems.

---

**Document Version:** 2.1  
**Last Updated:** January 2025  
**Document Owner:** Development Team  
**Review Cycle:** Quarterly
