/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : map_sensor_emulator.h
  * @brief          : Header for MAP sensor emulator module
  ******************************************************************************
  * @attention
  *
  * MAP Sensor Emulator for NUCLEO-STM32F7
  * Simulates a Manifold Absolute Pressure (MAP) sensor
  * 
  * Features:
  * - Pressure range from atmospheric (0V) to wide open throttle (3.3V)
  * - TPS position controls manifold pressure
  * - Idle conditions (~2000 RPM) show high vacuum (low pressure)
  * - WOT conditions (~13500 RPM) show atmospheric pressure (high voltage)
  * - Realistic pressure curve based on throttle position
  * 
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAP_SENSOR_EMULATOR_H
#define __MAP_SENSOR_EMULATOR_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f7xx_hal.h"
#include <math.h>

/* Exported types ------------------------------------------------------------*/
typedef struct {
    float tps_percent;          // Throttle position (0-100%)
    uint16_t current_rpm;       // Current engine RPM
    float manifold_pressure_kpa; // Manifold absolute pressure (kPa)
    float manifold_pressure_psi; // Manifold absolute pressure (PSI)
    uint16_t dac_output;        // Current DAC output value
    float voltage_output;       // Current voltage output (V)
    bool enabled;               // MAP emulator enable flag
} MAP_SensorState_t;

/* Exported constants --------------------------------------------------------*/
/* Pressure and voltage characteristics */
#define MAP_MIN_VOLTAGE             0.0f        // Minimum output voltage (V) - High vacuum
#define MAP_MAX_VOLTAGE             3.3f        // Maximum output voltage (V) - Atmospheric pressure
#define MAP_DAC_RESOLUTION          4096        // 12-bit DAC resolution

/* Atmospheric and engine pressure characteristics */
#define MAP_ATMOSPHERIC_PRESSURE_KPA    101.3f      // Atmospheric pressure (kPa) at sea level
#define MAP_ATMOSPHERIC_PRESSURE_PSI    14.7f       // Atmospheric pressure (PSI) at sea level
#define MAP_IDLE_VACUUM_KPA            55.0f        // Typical idle manifold pressure (kPa) - high vacuum
#define MAP_IDLE_VACUUM_PSI            8.0f         // Typical idle manifold pressure (PSI) - high vacuum

/* RPM and throttle characteristics */
#define MAP_IDLE_RPM                2000        // Typical idle RPM
#define MAP_MAX_RPM                 13500       // Maximum RPM (WOT)
#define MAP_MIN_RPM                 0           // Minimum RPM

/* TPS characteristics */
#define MAP_TPS_MIN_PERCENT         0.0f        // Closed throttle (0%)
#define MAP_TPS_MAX_PERCENT         100.0f      // Wide open throttle (100%)
#define MAP_TPS_IDLE_PERCENT        5.0f        // Typical idle TPS position (5%)

/* DAC channel configuration */
#define MAP_DAC_CHANNEL             DAC_CHANNEL_2   // Use DAC channel 2 for MAP output
#define MAP_OUTPUT_Pin              GPIO_PIN_5      // PA5 for DAC channel 2
#define MAP_OUTPUT_GPIO_Port        GPIOA

/* Exported macro ------------------------------------------------------------*/
/* Pressure conversion macros */
#define MAP_KPA_TO_PSI(kpa)         ((kpa) * 0.145038f)
#define MAP_PSI_TO_KPA(psi)         ((psi) * 6.89476f)

/* Voltage conversion macros */
#define MAP_PRESSURE_TO_VOLTAGE(pressure_kpa) \
    (((pressure_kpa) - MAP_IDLE_VACUUM_KPA) * MAP_MAX_VOLTAGE / (MAP_ATMOSPHERIC_PRESSURE_KPA - MAP_IDLE_VACUUM_KPA))

#define MAP_VOLTAGE_TO_DAC(voltage) ((uint16_t)((voltage) * MAP_DAC_RESOLUTION / MAP_MAX_VOLTAGE))

#define MAP_PRESSURE_TO_DAC(pressure_kpa) (MAP_VOLTAGE_TO_DAC(MAP_PRESSURE_TO_VOLTAGE(pressure_kpa)))

/* TPS and RPM conversion macros */
#define MAP_ADC_TO_TPS_PERCENT(adc_value) ((float)(adc_value) * MAP_TPS_MAX_PERCENT / 4096.0f)
#define MAP_ADC_TO_RPM(adc_value) ((uint16_t)((uint32_t)(adc_value) * MAP_MAX_RPM / 4096))

/* Pressure validation macros */
#define MAP_CLAMP_PRESSURE_KPA(pressure) \
    ((pressure < MAP_IDLE_VACUUM_KPA) ? MAP_IDLE_VACUUM_KPA : \
     ((pressure > MAP_ATMOSPHERIC_PRESSURE_KPA) ? MAP_ATMOSPHERIC_PRESSURE_KPA : pressure))

#define MAP_IS_VALID_PRESSURE_KPA(pressure) \
    ((pressure >= MAP_IDLE_VACUUM_KPA) && (pressure <= MAP_ATMOSPHERIC_PRESSURE_KPA))

/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief  Initialize MAP sensor emulator
  * @retval None
  */
void MAP_Emulator_Init(void);

/**
  * @brief  Update MAP sensor emulator (call periodically from main loop)
  * @retval None
  */
void MAP_Emulator_Update(void);

/**
  * @brief  Update MAP sensor based on TPS ADC reading
  * @param  tps_adc_value: ADC value from TPS potentiometer (0-4095)
  * @retval None
  */
void MAP_Emulator_UpdateFromTPS(uint16_t tps_adc_value);

/**
  * @brief  Get current manifold pressure in kPa
  * @retval Current manifold pressure in kPa
  */
float MAP_Emulator_GetPressure_kPa(void);

/**
  * @brief  Get current manifold pressure in PSI
  * @retval Current manifold pressure in PSI
  */
float MAP_Emulator_GetPressure_PSI(void);

/**
  * @brief  Get current TPS position percentage
  * @retval Current TPS position (0-100%)
  */
float MAP_Emulator_GetTPS_Percent(void);

/**
  * @brief  Get current output voltage
  * @retval Current output voltage in volts
  */
float MAP_Emulator_GetVoltage(void);

/**
  * @brief  Get current DAC output value
  * @retval Current DAC output (0-4095)
  */
uint16_t MAP_Emulator_GetDACOutput(void);

/**
  * @brief  Get current RPM reading
  * @retval Current RPM
  */
uint16_t MAP_Emulator_GetRPM(void);

/**
  * @brief  Set manual manifold pressure (for testing)
  * @param  pressure_kpa: Pressure in kPa
  * @retval None
  */
void MAP_Emulator_SetPressure_kPa(float pressure_kpa);

/**
  * @brief  Set manual TPS position (for testing)
  * @param  tps_percent: TPS position (0-100%)
  * @retval None
  */
void MAP_Emulator_SetTPS_Percent(float tps_percent);

/**
  * @brief  Enable or disable MAP emulator
  * @param  enable: true to enable, false to disable
  * @retval None
  */
void MAP_Emulator_SetEnabled(bool enable);

/**
  * @brief  Check if MAP emulator is enabled
  * @retval true if enabled, false if disabled
  */
bool MAP_Emulator_IsEnabled(void);

/* Utility functions for conversion and calibration */

/**
  * @brief  Calculate manifold pressure from TPS position
  * @param  tps_percent: TPS position (0-100%)
  * @retval Manifold pressure in kPa
  */
float MAP_Emulator_CalculatePressureFromTPS(float tps_percent);

/**
  * @brief  Convert manifold pressure to output voltage
  * @param  pressure_kpa: Pressure in kPa
  * @retval Output voltage in volts
  */
float MAP_Emulator_PressureToVoltage(float pressure_kpa);

/**
  * @brief  Convert output voltage to manifold pressure
  * @param  voltage: Output voltage in volts
  * @retval Manifold pressure in kPa
  */
float MAP_Emulator_VoltageToPressure(float voltage);

/**
  * @brief  Convert pressure to DAC value
  * @param  pressure_kpa: Pressure in kPa
  * @retval DAC value (0-4095)
  */
uint16_t MAP_Emulator_PressureToDAC(float pressure_kpa);

/**
  * @brief  Convert DAC value to pressure
  * @param  dac_value: DAC value (0-4095)
  * @retval Pressure in kPa
  */
float MAP_Emulator_DACToPressure(uint16_t dac_value);

/**
  * @brief  Get pressure description string for current pressure
  * @retval String describing current pressure condition
  */
const char* MAP_Emulator_GetPressureDescription(void);

/**
  * @brief  Print current MAP sensor status (for debugging)
  * @retval None
  */
void MAP_Emulator_PrintStatus(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAP_SENSOR_EMULATOR_H */