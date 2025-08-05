# NUCLEO-STM32F7 VR Sensor Emulator

A C project for the NUCLEO-STM32F7 development board that emulates the analog output of a Variable Reluctance (VR) sensor with a realistic distorted sine wave pattern.

## Project Overview

This project simulates a VR sensor signal that would typically be found in automotive applications for crankshaft position sensing. The emulated output provides a more realistic distorted sine wave rather than a simple square wave, making it suitable for testing engine control units and other automotive systems.

## Hardware Specifications

### Trigger Wheel Pattern
- **Total teeth**: 18
- **Regular teeth (17)**: 4° width with 16° gaps
- **Missing tooth (18th)**: 12° width with 8° gap
- **Wheel diameter**: 88.5mm
- **RPM range**: 0 - 13,400 RPM

### Hardware Requirements
- NUCLEO-STM32F7 development board
- Potentiometer (for RPM control input)
- Analog output circuitry
- Oscilloscope (for signal verification)

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
│   │   └── vr_sensor_emulator.h
│   └── Src/
│       ├── main.c
│       ├── stm32f7xx_hal_msp.c
│       ├── stm32f7xx_it.c
│       └── vr_sensor_emulator.c
├── Drivers/
│   └── STM32F7xx_HAL_Driver/
├── Makefile
├── README.md
└── STM32F767ZITx_FLASH.ld
```

## Technical Implementation

### Key Features
1. **ADC Input**: Reads potentiometer voltage to determine target RPM
2. **DAC Output**: Generates analog VR sensor signal
3. **Timer-based Timing**: Precise tooth timing calculation
4. **Sine Wave Generation**: Creates distorted sine wave output
5. **Missing Tooth Pattern**: Simulates 18-tooth wheel with missing tooth

### Signal Characteristics
- **Waveform**: Distorted sine wave (not square wave)
- **Frequency**: Variable based on RPM and tooth count
- **Amplitude**: Configurable analog output level
- **Pattern**: 17 regular teeth + 1 missing tooth pattern

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
   - Adjust potentiometer to change simulated RPM (0-13400)
   - Monitor DAC output for VR sensor signal
   - Missing tooth pattern occurs every 18 teeth

3. **Signal Verification**:
   - Use oscilloscope to verify waveform shape
   - Check timing accuracy with frequency counter
   - Verify missing tooth gap timing

## Configuration

### Timing Calculations
- **Tooth frequency**: `RPM × 18 ÷ 60` Hz
- **Tooth period**: `60 ÷ (RPM × 18)` seconds
- **Regular tooth on-time**: 4° = `(4/360) × tooth_period`
- **Missing tooth gap**: 8° = `(8/360) × tooth_period`

### Customization
Key parameters can be adjusted in `vr_sensor_emulator.h`:
- Tooth count and timing
- Signal amplitude and distortion
- ADC and DAC scaling factors
- Timer prescaler values

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
