# VR Sensor Emulator Unit Tests

## Overview

This document describes the comprehensive unit test suite for the NUCLEO-STM32F7 VR sensor emulator. The tests validate the RPM input functionality across the full range of 0 to 13,400 RPM.

## Test Architecture

### Test Files Structure
```
Core/
├── Inc/
│   ├── test_vr_emulator.h      # Test function declarations
│   └── test_integration.h       # Test integration interface
└── Src/
    ├── test_vr_emulator.c       # Core test implementations
    ├── test_integration.c       # Test runner and integration
    └── main_with_tests_example.c # Example main.c integration
```

## Test Categories

### 1. ADC to RPM Mapping Tests
**Purpose**: Validate the conversion from ADC values to RPM
**Coverage**: 20 test points across the ADC range (0-4095)
**Validation**:
- Correct RPM calculation from ADC input
- Linear mapping accuracy
- Edge case handling

### 2. RPM Conversion Calculations
**Purpose**: Verify tooth frequency and period calculations
**Coverage**: All RPM values from test cases
**Validation**:
- Tooth frequency = RPM × 18 ÷ 60
- Tooth period = 1,000,000 ÷ tooth_frequency (μs)
- Calculation accuracy within 2% tolerance

### 3. Timing Calculations
**Purpose**: Validate precise timing for trigger wheel simulation
**Test Points**:
- 600 RPM → 10 Hz wheel frequency
- 1800 RPM → 30 Hz wheel frequency  
- 6000 RPM → 100 Hz wheel frequency
- 13400 RPM → 223.3 Hz wheel frequency

### 4. Boundary Conditions
**Purpose**: Test edge cases and limits
**Coverage**:
- Minimum RPM (0)
- Maximum RPM (13,400)
- ADC boundary values (0, 4095)
- RPM clamping behavior

### 5. SetRPM Function Validation
**Purpose**: Test RPM setting and clamping logic
**Coverage**:
- Normal RPM values
- Over-limit RPM values (>13,400)
- Proper clamping to MAX_RPM

## Test Data

### RPM Test Cases (20 Points)
| ADC Value | Expected RPM | Tooth Freq (Hz) | Period (μs) |
|-----------|--------------|-----------------|-------------|
| 0         | 0            | 0.0             | 0           |
| 205       | 670          | 201.0           | 4975        |
| 409       | 1340         | 402.0           | 2488        |
| 614       | 2010         | 603.0           | 1658        |
| 819       | 2680         | 804.0           | 1244        |
| 1024      | 3350         | 1005.0          | 995         |
| ...       | ...          | ...             | ...         |
| 4095      | 13400        | 4020.0          | 249         |

## Running Tests

### Basic Test Execution
```c
#include "test_integration.h"

// Initialize test environment
VR_Test_Init();

// Run basic functionality tests
TestResults_t results = VR_Test_RunBasic();

// Check results
if (results.failed_tests == 0) {
    printf("✓ All tests passed!\n");
}
```

### Comprehensive Test Suite
```c
// Run all test suites
TestResults_t results = VR_Test_RunComprehensive();

// Results include:
// - Basic functionality
// - RPM range testing
// - ADC conversion testing  
// - Boundary conditions
// - Performance testing
```

### RPM Sweep Testing
```c
// Test every 100 RPM from 0 to 13400
TestResults_t results = VR_Test_RunRPMSweep(100);

// This creates 135 test points covering the full range
```

### Individual RPM Validation
```c
// Test specific RPM value
bool result = VR_Test_ValidateRPM(3000);

if (result) {
    printf("3000 RPM validation passed\n");
}
```

## Integration with Main Application

### Method 1: Button-Triggered Tests
```c
// In main loop
Handle_Button_Press();

// Button handler
static void Handle_Button_Press(void) {
    if (button_pressed) {
        TestResults_t results = VR_Test_RunBasic();
        // Display results via LEDs or UART
    }
}
```

### Method 2: Auto-Run on Startup
```c
int main(void) {
    // Initialize system
    VR_Emulator_Init();
    VR_Test_Init();
    
    // Auto-run tests
    TestResults_t results = VR_Test_RunBasic();
    
    if (results.failed_tests == 0) {
        printf("System validated - entering normal operation\n");
    }
    
    // Continue with main loop
}
```

### Method 3: UART Command Interface
```c
// UART command handler
void Handle_UART_Command(char* cmd) {
    if (strcmp(cmd, "test") == 0) {
        VR_Test_RunBasic();
    }
    else if (strcmp(cmd, "test-rpm") == 0) {
        VR_Test_RunRPMSweep(500);
    }
    else if (strcmp(cmd, "demo") == 0) {
        VR_Test_RunDemo();
    }
}
```

## Test Output Examples

### Successful Test Run
```
=== VR Sensor Emulator Unit Tests ===
Testing RPM range: 0 to 13400 RPM

Testing ADC to RPM mapping...
✓ ADC to RPM mapping tests completed

Testing RPM conversion calculations...
✓ RPM conversion calculation tests completed

Testing timing calculations...
✓ Timing calculation tests completed

Testing boundary conditions...
✓ Boundary condition tests completed

Testing VR_Emulator_SetRPM function...
✓ SetRPM function tests completed

=== Test Results Summary ===
Total Tests: 85
Passed: 85
Failed: 0
✓ ALL TESTS PASSED!
Success Rate: 100.0%
```

### Failed Test Example
```
Testing ADC to RPM mapping...
TEST FAILED: ADC 1024 -> RPM conversion (expected: 3350, got: 3355)
✓ ADC to RPM mapping tests completed

=== Test Results Summary ===
Total Tests: 85
Passed: 84
Failed: 1
✗ 1 TESTS FAILED!
Success Rate: 98.8%
```

## Performance Characteristics

### Test Execution Time
- Basic tests: ~2 seconds
- Comprehensive tests: ~10 seconds
- RPM sweep (100 RPM steps): ~5 seconds

### Memory Usage
- Test code: ~8KB Flash, ~2KB RAM
- Can be disabled with compile-time flags

### Real-Time Impact
- Tests run outside main control loop
- No impact on VR signal generation during normal operation
- Button-triggered execution prevents interference

## Validation Criteria

### Tolerance Settings
- RPM calculation: ±2% tolerance
- Frequency calculation: ±2% tolerance  
- Timing calculation: ±2% tolerance
- ADC conversion: ±1 RPM tolerance

### Pass/Fail Criteria
- **PASS**: All calculations within tolerance
- **FAIL**: Any calculation exceeds tolerance
- **ERROR**: System configuration issues

## Troubleshooting

### Common Issues
1. **High failure rate**: Check system clock configuration
2. **ADC conversion errors**: Verify ADC reference voltage
3. **Timing inaccuracies**: Check timer prescaler settings
4. **Compilation errors**: Ensure all test files are included in Makefile

### Debug Tips
- Enable verbose test output
- Use individual test functions for isolation
- Check hardware connections (potentiometer, DAC output)
- Verify floating-point math library linkage

## Future Enhancements

### Planned Test Additions
1. Signal quality validation (harmonic analysis)
2. Missing tooth pattern verification
3. Temperature drift testing
4. Long-term stability testing
5. Noise immunity testing

### Performance Optimizations
1. Fast test mode (reduced test points)
2. Background test execution
3. Test result caching
4. Automatic regression testing
