<!-- Use this file to provide workspace-specific custom instructions to Copilot. For more details, visit https://code.visualstudio.com/docs/copilot/copilot-customization#_use-a-githubcopilotinstructionsmd-file -->

# STM32F7 VR Sensor Emulator Project Instructions

This is an embedded C project for the NUCLEO-STM32F7 development board that emulates a Variable Reluctance (VR) sensor output.

## Project Specifications
- Target: NUCLEO-STM32F7 development board
- Trigger wheel: 18 teeth with missing tooth pattern
- 17 teeth: 4 degrees width, 16 degrees gap
- 18th tooth (missing): 12 degrees width, 8 degrees gap
- Wheel diameter: 88.5mm
- RPM range: 0-13400 RPM (controlled by potentiometer)
- Output: Distorted sine wave (not square wave)

## Development Environment
- STM32CubeMX version 6.1.0 for hardware abstraction layer generation
- VS Code on Windows with WSL (Ubuntu)
- GNU Make build system
- ARM GCC toolchain

## Key Features to Implement
- ADC reading from potentiometer for RPM control
- DAC output for analog VR sensor signal generation
- Timer-based precise timing for tooth pattern generation
- Sine wave generation with distortion for realistic VR sensor simulation
- Missing tooth detection pattern simulation

When generating code, prioritize:
- Real-time performance and precise timing
- Hardware abstraction layer (HAL) usage
- Modular code structure
- Proper interrupt handling
- Efficient memory usage for embedded constraints
