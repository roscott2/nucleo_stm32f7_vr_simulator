/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : map_sensor_emulator.c
  * @brief          : MAP sensor emulator implementation
  ******************************************************************************
  * @attention
  *
  * MAP Sensor Emulator for NUCLEO-STM32F7
  * Simulates a Manifold Absolute Pressure (MAP) sensor
  * 
  * Implementation Notes:
  * - Uses DAC channel 2 (PA5) for analog output
  * - Pressure curve: High vacuum at idle (~8 PSI / 55 kPa) to atmospheric at WOT (~14.7 PSI / 101.3 kPa)
  * - TPS input from same potentiometer used for VR sensor RPM control
  * - Realistic automotive MAP sensor behavior with smooth pressure transitions
  * 
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "map_sensor_emulator.h"
#include <string.h>
#include <stdio.h>

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define MAP_EMULATOR_MAGIC_NUMBER   0xABCD1234   // For state validation
#define MAP_PRESSURE_SMOOTHING      0.05f        // Smoothing factor for pressure changes
#define MAP_UPDATE_RATE_MS          10           // Update rate in milliseconds

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static MAP_SensorState_t map_sensor_state;
static DAC_HandleTypeDef *map_dac_handle = NULL;
static uint32_t map_magic_check = MAP_EMULATOR_MAGIC_NUMBER;
static uint32_t last_update_time = 0;
static float pressure_smoothing_buffer = 0.0f;

/* Private function prototypes -----------------------------------------------*/
static void MAP_Emulator_CalculatePressure(void);
static void MAP_Emulator_UpdateDAC(void);
static float MAP_Emulator_ApplyPressureCurve(float tps_percent);
static float MAP_Emulator_SmoothPressureTransition(float target_pressure);
static bool MAP_Emulator_ValidateState(void);

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  Initialize MAP sensor emulator
  * @retval None
  */
void MAP_Emulator_Init(void)
{
    // Initialize state structure
    memset(&map_sensor_state, 0, sizeof(MAP_SensorState_t));
    
    // Set initial conditions (engine off - atmospheric pressure)
    map_sensor_state.tps_percent = MAP_TPS_MIN_PERCENT;
    map_sensor_state.current_rpm = 0;
    map_sensor_state.manifold_pressure_kpa = MAP_ATMOSPHERIC_PRESSURE_KPA;
    map_sensor_state.manifold_pressure_psi = MAP_ATMOSPHERIC_PRESSURE_PSI;
    map_sensor_state.voltage_output = MAP_MAX_VOLTAGE;
    map_sensor_state.dac_output = MAP_DAC_RESOLUTION - 1; // Max DAC for atmospheric pressure
    map_sensor_state.enabled = true;
    
    // Initialize smoothing buffer
    pressure_smoothing_buffer = MAP_ATMOSPHERIC_PRESSURE_KPA;
    
    // Get DAC handle (should be initialized in main.c)
    extern DAC_HandleTypeDef hdac;
    map_dac_handle = &hdac;
    
    // Start DAC channel 2
    if (map_dac_handle != NULL) {
        HAL_DAC_Start(map_dac_handle, MAP_DAC_CHANNEL);
        // Set initial output to atmospheric pressure (high voltage)
        HAL_DAC_SetValue(map_dac_handle, MAP_DAC_CHANNEL, DAC_ALIGN_12B_R, map_sensor_state.dac_output);
    }
    
    // Set validation magic number
    map_magic_check = MAP_EMULATOR_MAGIC_NUMBER;
    
    // Initialize timing
    last_update_time = HAL_GetTick();
}

/**
  * @brief  Update MAP sensor emulator (call periodically from main loop)
  * @retval None
  */
void MAP_Emulator_Update(void)
{
    if (!MAP_Emulator_ValidateState() || !map_sensor_state.enabled) {
        return;
    }
    
    uint32_t current_time = HAL_GetTick();
    if (current_time - last_update_time >= MAP_UPDATE_RATE_MS) {
        MAP_Emulator_CalculatePressure();
        MAP_Emulator_UpdateDAC();
        last_update_time = current_time;
    }
}

/**
  * @brief  Update MAP sensor based on TPS ADC reading
  * @param  tps_adc_value: ADC value from TPS potentiometer (0-4095)
  * @retval None
  */
void MAP_Emulator_UpdateFromTPS(uint16_t tps_adc_value)
{
    if (!MAP_Emulator_ValidateState() || !map_sensor_state.enabled) {
        return;
    }
    
    // Convert ADC reading to TPS percentage
    map_sensor_state.tps_percent = MAP_ADC_TO_TPS_PERCENT(tps_adc_value);
    
    // Also calculate RPM for reference (same potentiometer controls both)
    map_sensor_state.current_rpm = MAP_ADC_TO_RPM(tps_adc_value);
    
    // Clamp TPS to valid range
    if (map_sensor_state.tps_percent < MAP_TPS_MIN_PERCENT) {
        map_sensor_state.tps_percent = MAP_TPS_MIN_PERCENT;
    } else if (map_sensor_state.tps_percent > MAP_TPS_MAX_PERCENT) {
        map_sensor_state.tps_percent = MAP_TPS_MAX_PERCENT;
    }
}

/**
  * @brief  Get current manifold pressure in kPa
  * @retval Current manifold pressure in kPa
  */
float MAP_Emulator_GetPressure_kPa(void)
{
    return MAP_Emulator_ValidateState() ? map_sensor_state.manifold_pressure_kpa : MAP_ATMOSPHERIC_PRESSURE_KPA;
}

/**
  * @brief  Get current manifold pressure in PSI
  * @retval Current manifold pressure in PSI
  */
float MAP_Emulator_GetPressure_PSI(void)
{
    return MAP_Emulator_ValidateState() ? map_sensor_state.manifold_pressure_psi : MAP_ATMOSPHERIC_PRESSURE_PSI;
}

/**
  * @brief  Get current TPS position percentage
  * @retval Current TPS position (0-100%)
  */
float MAP_Emulator_GetTPS_Percent(void)
{
    return MAP_Emulator_ValidateState() ? map_sensor_state.tps_percent : 0.0f;
}

/**
  * @brief  Get current output voltage
  * @retval Current output voltage in volts
  */
float MAP_Emulator_GetVoltage(void)
{
    return MAP_Emulator_ValidateState() ? map_sensor_state.voltage_output : MAP_MIN_VOLTAGE;
}

/**
  * @brief  Get current DAC output value
  * @retval Current DAC output (0-4095)
  */
uint16_t MAP_Emulator_GetDACOutput(void)
{
    return MAP_Emulator_ValidateState() ? map_sensor_state.dac_output : 0;
}

/**
  * @brief  Get current RPM reading
  * @retval Current RPM
  */
uint16_t MAP_Emulator_GetRPM(void)
{
    return MAP_Emulator_ValidateState() ? map_sensor_state.current_rpm : 0;
}

/**
  * @brief  Set manual manifold pressure (for testing)
  * @param  pressure_kpa: Pressure in kPa
  * @retval None
  */
void MAP_Emulator_SetPressure_kPa(float pressure_kpa)
{
    if (!MAP_Emulator_ValidateState()) {
        return;
    }
    
    // Clamp pressure to valid range
    map_sensor_state.manifold_pressure_kpa = MAP_CLAMP_PRESSURE_KPA(pressure_kpa);
    map_sensor_state.manifold_pressure_psi = MAP_KPA_TO_PSI(map_sensor_state.manifold_pressure_kpa);
    
    // Update voltage and DAC output
    map_sensor_state.voltage_output = MAP_Emulator_PressureToVoltage(map_sensor_state.manifold_pressure_kpa);
    map_sensor_state.dac_output = MAP_Emulator_PressureToDAC(map_sensor_state.manifold_pressure_kpa);
}

/**
  * @brief  Set manual TPS position (for testing)
  * @param  tps_percent: TPS position (0-100%)
  * @retval None
  */
void MAP_Emulator_SetTPS_Percent(float tps_percent)
{
    if (!MAP_Emulator_ValidateState()) {
        return;
    }
    
    // Clamp TPS to valid range
    if (tps_percent < MAP_TPS_MIN_PERCENT) {
        tps_percent = MAP_TPS_MIN_PERCENT;
    } else if (tps_percent > MAP_TPS_MAX_PERCENT) {
        tps_percent = MAP_TPS_MAX_PERCENT;
    }
    
    map_sensor_state.tps_percent = tps_percent;
}

/**
  * @brief  Enable or disable MAP emulator
  * @param  enable: true to enable, false to disable
  * @retval None
  */
void MAP_Emulator_SetEnabled(bool enable)
{
    if (MAP_Emulator_ValidateState()) {
        map_sensor_state.enabled = enable;
        
        if (!enable && map_dac_handle != NULL) {
            // Set DAC to 0V when disabled
            HAL_DAC_SetValue(map_dac_handle, MAP_DAC_CHANNEL, DAC_ALIGN_12B_R, 0);
        }
    }
}

/**
  * @brief  Check if MAP emulator is enabled
  * @retval true if enabled, false if disabled
  */
bool MAP_Emulator_IsEnabled(void)
{
    return MAP_Emulator_ValidateState() && map_sensor_state.enabled;
}

/* Utility functions for conversion and calibration */

/**
  * @brief  Calculate manifold pressure from TPS position
  * @param  tps_percent: TPS position (0-100%)
  * @retval Manifold pressure in kPa
  */
float MAP_Emulator_CalculatePressureFromTPS(float tps_percent)
{
    // Apply realistic pressure curve based on TPS position
    return MAP_Emulator_ApplyPressureCurve(tps_percent);
}

/**
  * @brief  Convert manifold pressure to output voltage
  * @param  pressure_kpa: Pressure in kPa
  * @retval Output voltage in volts
  */
float MAP_Emulator_PressureToVoltage(float pressure_kpa)
{
    // Clamp pressure to valid range
    pressure_kpa = MAP_CLAMP_PRESSURE_KPA(pressure_kpa);
    
    // Linear mapping: low pressure (high vacuum) = low voltage, high pressure = high voltage
    float voltage = (pressure_kpa - MAP_IDLE_VACUUM_KPA) * MAP_MAX_VOLTAGE / 
                   (MAP_ATMOSPHERIC_PRESSURE_KPA - MAP_IDLE_VACUUM_KPA);
    
    // Ensure voltage is within valid range
    if (voltage < MAP_MIN_VOLTAGE) voltage = MAP_MIN_VOLTAGE;
    if (voltage > MAP_MAX_VOLTAGE) voltage = MAP_MAX_VOLTAGE;
    
    return voltage;
}

/**
  * @brief  Convert output voltage to manifold pressure
  * @param  voltage: Output voltage in volts
  * @retval Manifold pressure in kPa
  */
float MAP_Emulator_VoltageToPressure(float voltage)
{
    // Clamp voltage to valid range
    if (voltage < MAP_MIN_VOLTAGE) voltage = MAP_MIN_VOLTAGE;
    if (voltage > MAP_MAX_VOLTAGE) voltage = MAP_MAX_VOLTAGE;
    
    // Linear mapping: low voltage = low pressure (high vacuum), high voltage = high pressure
    float pressure = MAP_IDLE_VACUUM_KPA + (voltage * (MAP_ATMOSPHERIC_PRESSURE_KPA - MAP_IDLE_VACUUM_KPA) / MAP_MAX_VOLTAGE);
    
    return pressure;
}

/**
  * @brief  Convert pressure to DAC value
  * @param  pressure_kpa: Pressure in kPa
  * @retval DAC value (0-4095)
  */
uint16_t MAP_Emulator_PressureToDAC(float pressure_kpa)
{
    float voltage = MAP_Emulator_PressureToVoltage(pressure_kpa);
    uint16_t dac_value = MAP_VOLTAGE_TO_DAC(voltage);
    
    // Ensure DAC value is within range
    if (dac_value >= MAP_DAC_RESOLUTION) {
        dac_value = MAP_DAC_RESOLUTION - 1;
    }
    
    return dac_value;
}

/**
  * @brief  Convert DAC value to pressure
  * @param  dac_value: DAC value (0-4095)
  * @retval Pressure in kPa
  */
float MAP_Emulator_DACToPressure(uint16_t dac_value)
{
    // Clamp DAC value to valid range
    if (dac_value >= MAP_DAC_RESOLUTION) {
        dac_value = MAP_DAC_RESOLUTION - 1;
    }
    
    float voltage = (float)dac_value * MAP_MAX_VOLTAGE / MAP_DAC_RESOLUTION;
    return MAP_Emulator_VoltageToPressure(voltage);
}

/**
  * @brief  Get pressure description string for current pressure
  * @retval String describing current pressure condition
  */
const char* MAP_Emulator_GetPressureDescription(void)
{
    if (!MAP_Emulator_ValidateState()) {
        return "Invalid";
    }
    
    float pressure = map_sensor_state.manifold_pressure_kpa;
    
    if (pressure <= MAP_IDLE_VACUUM_KPA + 5.0f) {
        return "High Vacuum (Idle)";
    } else if (pressure <= MAP_IDLE_VACUUM_KPA + 20.0f) {
        return "Moderate Vacuum (Light Load)";
    } else if (pressure <= MAP_ATMOSPHERIC_PRESSURE_KPA - 15.0f) {
        return "Low Vacuum (Heavy Load)";
    } else {
        return "Atmospheric (WOT)";
    }
}

/**
  * @brief  Print current MAP sensor status (for debugging)
  * @retval None
  */
void MAP_Emulator_PrintStatus(void)
{
    if (!MAP_Emulator_ValidateState()) {
        printf("MAP Emulator: Invalid state\r\n");
        return;
    }
    
    printf("MAP Emulator Status:\r\n");
    printf("  TPS: %.1f%%\r\n", map_sensor_state.tps_percent);
    printf("  RPM: %u\r\n", map_sensor_state.current_rpm);
    printf("  Pressure: %.1f kPa (%.1f PSI)\r\n", 
           map_sensor_state.manifold_pressure_kpa, 
           map_sensor_state.manifold_pressure_psi);
    printf("  Voltage: %.2f V\r\n", map_sensor_state.voltage_output);
    printf("  DAC: %u\r\n", map_sensor_state.dac_output);
    printf("  Condition: %s\r\n", MAP_Emulator_GetPressureDescription());
    printf("  Enabled: %s\r\n", map_sensor_state.enabled ? "Yes" : "No");
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Calculate manifold pressure based on current TPS position
  * @retval None
  */
static void MAP_Emulator_CalculatePressure(void)
{
    // Calculate target pressure from TPS position
    float target_pressure = MAP_Emulator_ApplyPressureCurve(map_sensor_state.tps_percent);
    
    // Apply smoothing to prevent abrupt pressure changes
    float smoothed_pressure = MAP_Emulator_SmoothPressureTransition(target_pressure);
    
    // Update pressure values
    map_sensor_state.manifold_pressure_kpa = smoothed_pressure;
    map_sensor_state.manifold_pressure_psi = MAP_KPA_TO_PSI(smoothed_pressure);
    
    // Update voltage output
    map_sensor_state.voltage_output = MAP_Emulator_PressureToVoltage(smoothed_pressure);
    
    // Update DAC output value
    map_sensor_state.dac_output = MAP_Emulator_PressureToDAC(smoothed_pressure);
}

/**
  * @brief  Update DAC output with current pressure value
  * @retval None
  */
static void MAP_Emulator_UpdateDAC(void)
{
    if (map_dac_handle != NULL && map_sensor_state.enabled) {
        HAL_DAC_SetValue(map_dac_handle, MAP_DAC_CHANNEL, DAC_ALIGN_12B_R, map_sensor_state.dac_output);
    }
}

/**
  * @brief  Apply realistic pressure curve based on TPS position
  * @param  tps_percent: TPS position (0-100%)
  * @retval Manifold pressure in kPa
  */
static float MAP_Emulator_ApplyPressureCurve(float tps_percent)
{
    // Realistic automotive MAP sensor pressure curve
    // At idle (0% TPS): High vacuum (~55 kPa)
    // At WOT (100% TPS): Atmospheric pressure (~101.3 kPa)
    // Curve is non-linear - more vacuum at low TPS, rapid rise near WOT
    
    float normalized_tps = tps_percent / 100.0f; // 0.0 to 1.0
    
    // Apply exponential curve for realistic throttle response
    // More vacuum at low throttle positions, rapid pressure rise at higher positions
    float pressure_factor = powf(normalized_tps, 0.7f); // Exponential curve
    
    // Calculate pressure: idle vacuum to atmospheric
    float pressure = MAP_IDLE_VACUUM_KPA + 
                    (pressure_factor * (MAP_ATMOSPHERIC_PRESSURE_KPA - MAP_IDLE_VACUUM_KPA));
    
    return MAP_CLAMP_PRESSURE_KPA(pressure);
}

/**
  * @brief  Apply smoothing to pressure transitions for realistic response
  * @param  target_pressure: Target pressure in kPa
  * @retval Smoothed pressure in kPa
  */
static float MAP_Emulator_SmoothPressureTransition(float target_pressure)
{
    // Apply low-pass filter for smooth pressure transitions
    pressure_smoothing_buffer = (1.0f - MAP_PRESSURE_SMOOTHING) * pressure_smoothing_buffer + 
                               MAP_PRESSURE_SMOOTHING * target_pressure;
    
    return pressure_smoothing_buffer;
}

/**
  * @brief  Validate MAP emulator state
  * @retval true if state is valid, false otherwise
  */
static bool MAP_Emulator_ValidateState(void)
{
    return (map_magic_check == MAP_EMULATOR_MAGIC_NUMBER);
}