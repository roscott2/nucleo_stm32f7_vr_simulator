/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : vr_sensor_emulator.h
  * @brief          : Header for VR sensor emulator module
  ******************************************************************************
  * @attention
  *
  * VR Sensor Emulator for NUCLEO-STM32F7
  * Emulates a Variable Reluctance sensor with 18-tooth trigger wheel
  * 
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __VR_SENSOR_EMULATOR_H
#define __VR_SENSOR_EMULATOR_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f7xx_hal.h"
#include <math.h>

/* Exported types ------------------------------------------------------------*/
typedef struct {
    uint16_t rpm_adc_value;
    uint16_t target_rpm;
    uint32_t tooth_period_us;         // Period for each tooth in microseconds  
    uint8_t current_tooth;
    uint32_t tooth_timer_us;          // Current position within tooth period (microseconds)
    float sine_phase;
    uint16_t dac_output;
    uint32_t revolution_period_us;    // Total revolution period in microseconds
    uint32_t revolution_timer_us;     // Current position in revolution (microseconds)
} VR_SensorState_t;

/* Exported constants --------------------------------------------------------*/
#define TRIGGER_WHEEL_TEETH         18
#define REGULAR_TOOTH_COUNT         17
#define MISSING_TOOTH_INDEX         17  // 18th tooth (0-indexed)

#define REGULAR_TOOTH_ANGLE         4.0f    // degrees
#define REGULAR_TOOTH_GAP           16.0f   // degrees
#define MISSING_TOOTH_ANGLE         12.0f   // degrees
#define MISSING_TOOTH_GAP           8.0f    // degrees

#define WHEEL_DIAMETER_MM           88.5f
#define MAX_RPM                     13400
#define MIN_RPM                     0

#define ADC_RESOLUTION              4096    // 12-bit ADC
#define DAC_RESOLUTION              4096    // 12-bit DAC
#define DAC_MAX_VOLTAGE             3.3f    // Volts

/* VR sensor signal characteristics */
#define VR_AMPLITUDE_SCALE          0.8f    // Scale factor for sine wave amplitude
#define VR_DISTORTION_FACTOR        0.15f   // Distortion amount
#define VR_DC_OFFSET                0.4f    // DC offset as fraction of full scale

/* Exported macro ------------------------------------------------------------*/
#define DEGREES_TO_RADIANS(deg)     ((deg) * M_PI / 180.0f)
#define RPM_TO_TOOTH_FREQ(rpm)      ((rpm) * TRIGGER_WHEEL_TEETH / 60.0f)
#define TOOTH_FREQ_TO_PERIOD_US(freq) (1000000.0f / (freq))

/* Exported functions prototypes ---------------------------------------------*/
void VR_Emulator_Init(void);
void VR_Emulator_Update(void);
void VR_Emulator_SetRPM(uint16_t rpm);
uint16_t VR_Emulator_GetRPM(void);
uint16_t VR_Emulator_ReadPotentiometer(void);
void VR_Emulator_GenerateSignal(void);
uint16_t VR_Emulator_CalculateDAC_Value_Fixed(float progress, uint8_t tooth_active);

/* Timer callbacks */
void VR_Emulator_TimerCallback(void);                    // TIM6: Fixed 100kHz waveform generation

/* Functions for fixed-rate waveform generation */
void VR_Emulator_UpdateRPM(void);                       // TIM2: Read ADC and update RPM
void VR_Emulator_GenerateWaveformFixed(void);           // Generate waveform at fixed 100kHz rate

#ifdef __cplusplus
}
#endif

#endif /* __VR_SENSOR_EMULATOR_H */
