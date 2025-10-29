# Fixed 100kHz VR Sensor Waveform Generation

## Architecture Overview

The VR sensor emulator now uses a fixed 100kHz timer frequency (TIM6) for waveform generation, providing excellent resolution at low RPM while maintaining adequate performance at high RPM.

## Timer Configuration

### TIM6 - Fixed 100kHz Waveform Generation
- **Frequency**: Fixed 100kHz (10μs period)
- **Configuration**: PSC=1079, ARR=0  
- **Clock Source**: APB1 Timer Clock = 108MHz
- **Calculation**: 108MHz ÷ ((1079+1) × (0+1)) = 100kHz

### TIM2 - RPM Update Timer  
- **Frequency**: 20Hz (50ms period)
- **Purpose**: Read ADC potentiometer and update RPM
- **Configuration**: PSC=10799, ARR=499

## Waveform Resolution Analysis

### Resolution Comparison at Different RPMs

| RPM   | Revolution Time | Samples/Revolution | Regular Tooth Samples | Gap Samples | Quality |
|-------|----------------|-------------------|---------------------|-------------|---------|
| **500**   | 120ms         | 12,000           | ~133 per tooth     | ~533 per gap | **Excellent** |
| **1000**  | 60ms          | 6,000            | ~67 per tooth      | ~267 per gap | **Excellent** |
| **3000**  | 20ms          | 2,000            | ~22 per tooth      | ~89 per gap  | **Very Good** |
| **6000**  | 10ms          | 1,000            | ~11 per tooth      | ~44 per gap  | **Good** |
| **10000** | 6ms           | 600              | ~7 per tooth       | ~27 per gap  | **Adequate** |
| **13400** | 4.5ms         | 450              | ~5 per tooth       | ~20 per gap  | **Acceptable** |

### Key Improvements

**At Low RPM (500 RPM):**
- **133 samples per tooth active** vs 4 samples in variable approach
- **33x improvement** in waveform quality
- Smooth, high-resolution sine waves
- Excellent tooth edge definition

**At High RPM (13400 RPM):**
- **5 samples per tooth active** vs 4 samples in variable approach  
- Still provides adequate waveform quality
- Maintains timing accuracy

## Timing Implementation

### Time-based Tracking
```c
// Revolution timing  
uint32_t revolution_period_us = 60000000UL / rpm;    // μs per revolution
uint32_t tooth_period_us = revolution_period_us / 18; // μs per tooth

// Position tracking (updated every 10μs)
vr_state.revolution_timer_us += 10;  // Track position in revolution
vr_state.tooth_timer_us += 10;       // Track position within current tooth
```

### Tooth Timing Calculation
```c
// Regular tooth (4° active, 16° gap out of 20° total)
uint32_t tooth_active_us = (tooth_period_us * 4) / 20;   // 20% active
uint32_t tooth_gap_us = (tooth_period_us * 16) / 20;     // 80% gap

// Missing tooth (12° active, 8° gap out of 20° total)  
uint32_t tooth_active_us = (tooth_period_us * 12) / 20;  // 60% active
uint32_t tooth_gap_us = (tooth_period_us * 8) / 20;      // 40% gap
```

## Waveform Generation

### Progress-based Sine Generation
```c
// For tooth active periods (rising above DC offset)
float progress = (float)tooth_timer_us / (float)tooth_active_us;  // 0.0 to 1.0
float sine_angle = progress * π;           // 0 to π
float sine_value = sin(sine_angle);        // 0 to 1 to 0  
output_voltage = 0.5 + (sine_value * 0.5); // 1.65V to 3.3V

// For gap periods (falling below DC offset)
float gap_progress = (float)gap_time / (float)tooth_gap_us;
float sine_angle = gap_progress * π;        
float sine_value = sin(sine_angle);         
output_voltage = 0.5 - (sine_value * 0.5); // 1.65V to 0V
```

### Waveform Characteristics
- **DC Offset**: 50% (1.65V with 3.3V reference)
- **Peak Voltage**: 3.3V (at tooth midpoint)
- **Minimum Voltage**: 0V (at gap midpoint)  
- **Sine Wave Period**: Spans entire tooth/gap duration
- **Distortion**: Subtle harmonics for VR sensor realism

## Performance Benefits

### Low RPM Operation
- **Smooth waveforms** with hundreds of samples per tooth
- **Precise edge timing** for accurate ECU trigger detection
- **Clean sine wave shape** matching real VR sensors
- **Excellent missing tooth definition**

### High RPM Operation  
- **Adequate resolution** even at maximum 13400 RPM
- **Maintains timing accuracy** for ECU synchronization
- **Consistent waveform shape** across RPM range
- **Predictable performance** without frequency changes

### System Stability
- **No prescaler changes** during operation
- **Fixed interrupt rate** simplifies timing analysis
- **Consistent CPU load** regardless of RPM
- **Simplified debugging** with predictable timing

## Expected Waveform Quality

At 500 RPM, the waveform will show:
- **Very smooth sine waves** with excellent resolution
- **Sharp, clean transitions** at tooth edges
- **Accurate timing** for ECU trigger detection
- **Realistic VR sensor characteristics**

At 13400 RPM, the waveform will show:
- **Recognizable sine wave shape** with 5+ samples per tooth
- **Adequate timing accuracy** for high-performance engines  
- **Clear missing tooth pattern** for synchronization
- **Acceptable quality** for ECU operation

This fixed 100kHz approach provides the best balance of waveform quality at low speeds while maintaining functionality across the entire RPM range.