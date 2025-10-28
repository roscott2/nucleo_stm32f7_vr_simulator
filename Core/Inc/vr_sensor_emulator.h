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
    uint32_t tooth_period_us;
    uint8_t current_tooth;
    uint32_t tooth_timer;
    float sine_phase;
    uint16_t dac_output;
    float current_degree;        // Current timing wheel position in degrees
    uint16_t tim6_prescaler;     // Current TIM6 prescaler value
    uint16_t quantized_rpm;      // RPM quantized to 500 intervals
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
uint16_t VR_Emulator_CalculateDAC_Value(float angle, uint8_t tooth_active);

/* Timer callbacks */
void VR_Emulator_TimerCallback(void);                    // TIM6: Waveform generation

/* New functions for TIM6 frequency control */
void VR_Emulator_UpdateTIM6Frequency(uint16_t rpm);     // Update TIM6 for current RPM
uint16_t VR_Emulator_CalculatePrescaler(uint16_t rpm);  // Calculate PSC for given RPM
void VR_Emulator_GenerateWaveformDegree(void);          // Generate waveform for 1 degree

#ifdef __cplusplus
}
#endif

#endif /* __VR_SENSOR_EMULATOR_H */
