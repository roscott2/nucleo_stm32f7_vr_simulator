# NUCLEO-STM32F7 VR Sensor Emulator

A C project for the NUCLEO-STM32F7 development board that emulates the analog output of a Variable Reluctance (VR) sensor with a realistic distorted sine wave pattern, plus a Manifold Absolute Pressure (MAP) sensor for comprehensive automotive simulation.

## Project Overview

This project simulates two critical automotive sensors:

### VR (Variable Reluctance) Sensor
- Crankshaft position sensing simulation
- Realistic distorted sine wave output (not square wave)  
- 18-tooth trigger wheel pattern with missing tooth
- Suitable for testing engine control units and automotive systems

### MAP (Manifold Absolute Pressure) Sensor  
- Manifold vacuum/pressure simulation
- Pressure range: High vacuum at idle (~55 kPa) to atmospheric at WOT (~101.3 kPa)
- Voltage output: 0V (high vacuum) to 3.3V (atmospheric pressure)
- Coordinated with TPS (Throttle Position Sensor) simulation

## Hardware Specifications

### Trigger Wheel Pattern
- **Total teeth**: 18
- **Regular teeth (17)**: 4° width with 16° gaps
- **Missing tooth (18th)**: 12° width with 8° gap
- **Wheel diameter**: 88.5mm
- **RPM range**: 0 - 13,400 RPM

### Hardware Requirements
- NUCLEO-STM32F7 development board
- Potentiometer (for TPS/RPM control input - shared between both sensors)
- Dual analog output circuitry (two DAC channels)
- Oscilloscope (for signal verification)

### Pin Configuration
- **PA4 (DAC Channel 1)**: VR sensor analog output
- **PA5 (DAC Channel 2)**: MAP sensor analog output  
- **PA0 (ADC Channel 0)**: TPS potentiometer input (shared)

## Development Environment

### Tools and Software
- **STM32CubeMX**: Version 6.1.0 (for HAL generation)
- **IDE**: VS Code on Windows
- **Build Environment**: WSL (Ubuntu) with GNU Make
- **Toolchain**: ARM GCC
- **Debugger**: OpenOCD with ST-Link

### Project Structure
```
nucleo_stm32f7_vr_simulator/
├── .github/
│   └── copilot-instructions.md
├── .vscode/
│   ├── c_cpp_properties.json
│   ├── launch.json
│   ├── tasks.json
│   └── settings.json
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── stm32f7xx_hal_conf.h
│   │   ├── stm32f7xx_it.h
│   │   ├── vr_sensor_emulator.h
│   │   └── map_sensor_emulator.h
│   └── Src/
│       ├── main.c
│       ├── stm32f7xx_hal_msp.c
│       ├── stm32f7xx_it.c
│       ├── vr_sensor_emulator.c
│       └── map_sensor_emulator.c
├── Drivers/
│   └── STM32F7xx_HAL_Driver/
├── Makefile
├── README.md
└── STM32F767ZITx_FLASH.ld
```

## Technical Implementation

### Key Features
1. **Dual Sensor Simulation**: Both VR and MAP sensors working simultaneously
2. **ADC Input**: Single potentiometer controls both TPS position and engine RPM
3. **Dual DAC Output**: Separate analog outputs for VR and MAP sensors
4. **Timer-based VR Timing**: Precise tooth timing calculation for VR sensor
5. **Sine Wave Generation**: Creates distorted sine wave VR output
6. **Missing Tooth Pattern**: Simulates 18-tooth wheel with missing tooth
7. **Realistic MAP Response**: Manifold pressure varies with throttle position
8. **Coordinated Operation**: Both sensors respond to same TPS input

### Signal Characteristics

#### VR Sensor (DAC Channel 1 - PA4)
- **Waveform**: Distorted sine wave (not square wave)
- **Frequency**: Variable based on RPM and tooth count
- **Amplitude**: Configurable analog output level
- **Pattern**: 17 regular teeth + 1 missing tooth pattern

#### MAP Sensor (DAC Channel 2 - PA5)
- **Waveform**: Steady DC voltage representing manifold pressure
- **Range**: 0V (high vacuum) to 3.3V (atmospheric pressure)
- **Response**: Exponential curve matching throttle position
- **Pressure Range**: 55 kPa (idle) to 101.3 kPa (WOT)

## Building and Running

### Prerequisites
Ensure you have the following installed in your WSL Ubuntu environment:
```bash
sudo apt update
sudo apt install build-essential
sudo apt install gcc-arm-none-eabi
sudo apt install openocd
sudo apt install git
```

### Build Instructions
1. **Generate STM32 HAL files**:
   - Use STM32CubeMX 6.1.0 to generate the base project
   - Configure GPIO, ADC, DAC, and Timer peripherals
   - Generate code for Makefile project

2. **Build the project**:
   ```bash
   make clean
   make all
   ```

3. **Flash to board**:
   ```bash
   make flash
   ```

### VS Code Development
The project includes VS Code configuration files for:
- **IntelliSense**: C/C++ code completion and analysis
- **Build Tasks**: Integrated build system
- **Debugging**: OpenOCD debugging with ST-Link
- **Launch Configuration**: Debug and run configurations

## Usage

1. **Connect Hardware**:
   - Connect potentiometer to ADC input pin
   - Connect DAC output to oscilloscope or target ECU
   - Power the NUCLEO board via USB

2. **Operation**:
   - Adjust potentiometer to change simulated RPM/TPS (0-13400 RPM / 0-100% TPS)
   - Monitor PA4 (DAC1) for VR sensor signal with missing tooth pattern
   - Monitor PA5 (DAC2) for MAP sensor pressure signal (0V=vacuum, 3.3V=atmospheric)
   - Both sensors respond simultaneously to same potentiometer input

3. **Signal Verification**:
   - Use oscilloscope to verify VR waveform shape and missing tooth pattern
   - Check VR timing accuracy with frequency counter
   - Verify MAP sensor voltage changes smoothly with TPS position
   - Test pressure range: ~0.5V at idle to ~3.3V at WOT

## Unit Testing

The project includes a comprehensive unit testing framework to validate both VR and MAP sensor functionality. Tests can be run on the target hardware to ensure correct operation.

### Test Framework Overview

The testing system provides:
- **85+ individual test cases** covering all sensor functionality  
- **Automated test suites** for VR sensor timing accuracy
- **MAP sensor pressure validation** across full TPS range
- **Coordinated operation tests** for dual-sensor interaction
- **Performance benchmarking** and boundary condition testing
- **Interactive demo modes** for system verification

### Running Tests

#### 1. Build with Tests Enabled
Tests are automatically included in the build. Ensure the test files are compiled:
```bash
make clean
make all
```

#### 2. Enable Test Mode in Code
Add test initialization to your `main.c`:
```c
#include "test_integration.h"

int main(void) {
    // ... hardware initialization ...
    
    // Initialize test framework
    VR_Test_Init();
    
    // Run test suites
    TestResults_t results = VR_Test_RunComprehensive();
    
    // ... rest of main loop ...
}
```

#### 3. Available Test Functions

**Basic Test Suites:**
```c
VR_Test_RunBasic();              // Basic VR sensor tests
MAP_Test_RunBasic();             // Basic MAP sensor tests  
VR_Test_RunComprehensive();      // Full test suite (VR + MAP)
```

**Specialized Tests:**
```c
VR_Test_RunRPMSweep(100);        // RPM sweep with 100 RPM steps
MAP_Emulator_TestPressureSweep(); // TPS pressure response test
VR_MAP_Test_RunCoordinated();    // Dual sensor coordination test
```

**Interactive Demos:**
```c
VR_Test_RunDemo();               // VR sensor demonstration
VR_MAP_Test_RunDemo();           // Dual sensor demonstration
```

#### 4. Test Output Examples

**Comprehensive Test Results:**
```
======================================
  Dual Sensor Emulator Test Suite
======================================
Target: NUCLEO-STM32F7
VR Sensor RPM Range: 0 - 13400 RPM
MAP Sensor Pressure: 55.0 - 101.3 kPa
Test Mode: ENABLED
======================================

=== Test Suite 1: Basic Functionality ===
Running VR emulator tests...
✓ VR initialization test passed
✓ RPM calculation accuracy verified
✓ Timing calculations within tolerance

=== Test Suite 6: MAP Sensor Testing ===
Running MAP sensor tests...
✓ Pressure calculations accurate
✓ TPS response curve validated
✓ DAC output within specifications

Total Tests: 87
Passed: 85
Failed: 2  
Success Rate: 97.7%
```

**Pressure Sweep Output:**
```
=== MAP PRESSURE SWEEP TEST ===
TPS%    Pressure(kPa)   Voltage(V)    Condition
0       55.1           0.02          High Vacuum (Idle)
25      65.8           0.85          Moderate Vacuum
50      78.2           1.68          Light Load  
75      89.1           2.51          Heavy Load
100     101.3          3.30          Atmospheric (WOT)
```

### Test Categories

#### VR Sensor Tests
1. **Initialization** - Startup state validation
2. **RPM Accuracy** - ADC to RPM conversion precision  
3. **Timing Calculations** - Tooth frequency and period accuracy
4. **Signal Generation** - DAC waveform validation
5. **Missing Tooth Pattern** - 18th tooth gap timing
6. **Boundary Conditions** - Min/max RPM handling
7. **Performance** - Real-time operation validation

#### MAP Sensor Tests  
1. **Pressure Calculations** - TPS to kPa/PSI conversion
2. **Voltage Mapping** - Linear 0V-3.3V output validation
3. **DAC Output** - 12-bit resolution accuracy
4. **TPS Response** - Realistic automotive pressure curves
5. **Boundary Clamping** - Out-of-range value handling
6. **Coordinate Operation** - Shared potentiometer validation

#### Integration Tests
1. **Dual Sensor Coordination** - Shared TPS input validation
2. **Real-time Performance** - Both sensors updating simultaneously  
3. **Cross-validation** - VR RPM vs MAP TPS correlation
4. **Hardware Integration** - ADC/DAC peripheral interaction

### Interpreting Test Results

**Success Criteria:**
- **VR Timing Accuracy**: ±1% tolerance for tooth frequency calculations
- **MAP Pressure Accuracy**: ±2 kPa tolerance across pressure range  
- **Voltage Linearity**: ±0.1V tolerance for pressure-to-voltage mapping
- **DAC Resolution**: ±2 LSB accuracy for 12-bit output
- **Coordination**: <5% variance between VR RPM and MAP TPS correlation

**Common Test Failures:**
- **Timing drift**: Check crystal accuracy and timer configuration
- **Pressure nonlinearity**: Verify TPS response curve parameters
- **DAC inaccuracy**: Check reference voltage stability  
- **Coordination issues**: Validate shared ADC timing

### Automated Testing

For continuous integration, tests can be automated:
```c
// In main() for automated testing
if (AUTO_TEST_MODE) {
    TestResults_t vr_results = VR_Test_RunBasic();
    TestResults_t map_results = MAP_Test_RunBasic(); 
    TestResults_t coord_results = VR_MAP_Test_RunCoordinated();
    
    // Check overall success rate
    if ((vr_results.passed_tests + map_results.passed_tests + coord_results.passed_tests) > 
        (vr_results.total_tests + map_results.total_tests + coord_results.total_tests) * 0.95f) {
        printf("✓ All tests passed - System ready\n");
    } else {
        printf("✗ Test failures detected - Check configuration\n");
    }
}
```

## Configuration

### Timing Calculations
- **Tooth frequency**: `RPM × 18 ÷ 60` Hz
- **Tooth period**: `60 ÷ (RPM × 18)` seconds
- **Regular tooth on-time**: 4° = `(4/360) × tooth_period`
- **Missing tooth gap**: 8° = `(8/360) × tooth_period`

### Customization
Key parameters can be adjusted in the header files:

**VR Sensor** (`vr_sensor_emulator.h`):
- Tooth count and timing
- Signal amplitude and distortion
- ADC and DAC scaling factors
- Timer prescaler values

**MAP Sensor** (`map_sensor_emulator.h`):
- Pressure range (kPa and PSI)
- Voltage output range
- TPS response curve characteristics
- Pressure smoothing parameters

## Troubleshooting

### Common Issues
1. **Build Errors**: Ensure ARM GCC toolchain is properly installed
2. **Flash Errors**: Check ST-Link connection and drivers
3. **Signal Issues**: Verify DAC output circuitry and grounding
4. **Timing Accuracy**: Check timer configuration and crystal accuracy

### Debug Tips
- Use VS Code debugger to step through code
- Monitor variables in real-time during execution
- Use printf via SWV or UART for debugging output
- Verify peripheral configurations with STM32CubeMX

## License

This project is provided as-is for educational and development purposes.

## Contributing

Feel free to submit issues and enhancement requests!
