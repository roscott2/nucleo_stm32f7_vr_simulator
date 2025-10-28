# STM32F7 VR Sensor Emulator - New Timer Architecture

## Architecture Overview

The VR sensor emulator has been completely restructured with a new timer-based architecture for improved waveform quality and precise timing control.

### Timer Architecture

#### TIM2 - RPM Update Timer (50ms intervals)
- **Frequency**: 20 Hz (50ms period)  
- **Configuration**: PSC=10799, ARR=499
- **Purpose**: Reads potentiometer ADC and updates RPM every 50ms
- **Interrupt Handler**: Calls `VR_Emulator_UpdateRPM()`
- **Clock Source**: APB1 Timer Clock = 108MHz

#### TIM6 - VR Waveform Generation Timer (Variable frequency)
- **Frequency**: Variable based on RPM (RPM × 6 Hz for 1 interrupt per degree)
- **Configuration**: PSC=variable, ARR=9 (fixed)
- **Purpose**: Generates VR waveform with 1 interrupt per degree of timing wheel rotation
- **Interrupt Handler**: Calls `VR_Emulator_TimerCallback()`
- **Clock Source**: APB1 Timer Clock = 108MHz

### Key Features

#### RPM Quantization
- RPM values are quantized to 500 RPM intervals (0, 500, 1000, 1500... 16000)
- This provides stable timing and reduces prescaler changes
- Smoother operation and more predictable waveform generation

#### Degree-based Waveform Generation
- Each timer interrupt represents 1 degree of timing wheel rotation
- Precise control over tooth timing and waveform shape
- Supports 18-tooth trigger wheel with missing tooth pattern

#### Timing Wheel Pattern
- **17 Regular Teeth**: 4° tooth width, 16° gap = 20° total per tooth
- **1 Missing Tooth**: 12° tooth width, 8° gap = 20° total
- **Total**: 360° rotation for complete cycle

### Function Mapping

| Function | Purpose | Timer Association |
|----------|---------|-------------------|
| `VR_Emulator_UpdateRPM()` | Read ADC, update RPM | Called by TIM2 |
| `VR_Emulator_TimerCallback()` | Generate waveform for current degree | Called by TIM6 |
| `VR_Emulator_UpdateTIM6Frequency()` | Update TIM6 frequency for new RPM | Called when RPM changes |
| `VR_Emulator_CalculatePrescaler()` | Calculate TIM6 prescaler for frequency | Helper function |
| `VR_Emulator_GenerateWaveformDegree()` | Generate DAC output for current degree | Core waveform generation |

### Performance Characteristics

#### Frequency Range
- **Minimum RPM**: 500 RPM → TIM6 frequency = 3000 Hz
- **Maximum RPM**: 13400 RPM → TIM6 frequency = 80400 Hz
- **Prescaler Range**: 13 to 360 (for 500-16000 RPM quantized range)

#### Timing Accuracy
- **Degree Resolution**: 1° per interrupt (0.1% of full rotation)
- **ADC Update Rate**: 20 Hz (adequate for human potentiometer input)
- **Waveform Update Rate**: 360 interrupts per engine revolution

### Implementation Benefits

1. **Higher Waveform Quality**: Variable frequency TIM6 provides optimal resolution at all RPM levels
2. **Stable Operation**: RPM quantization prevents frequent prescaler changes
3. **Real-time Performance**: Interrupt-driven architecture ensures precise timing
4. **Scalable Design**: Easy to modify tooth patterns or timing wheel configurations
5. **Reduced CPU Load**: Main loop no longer handles timing-critical operations

### Testing Notes

- The new architecture eliminates the 100Hz limitation of the previous design
- At 13400 RPM, the system generates 80.4kHz interrupts vs previous 100Hz
- This represents an 804x improvement in timing resolution at maximum RPM