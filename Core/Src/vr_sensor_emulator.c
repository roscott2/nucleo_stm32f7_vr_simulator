/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : vr_sensor_emulator.c
  * @brief          : VR sensor emulator implementation
  ******************************************************************************
  * @attention
  *
  * VR Sensor Emulator for NUCLEO-STM32F7
  * Emulates a Variable Reluctance sensor with 18-tooth trigger wheel
  * 
  * Features:
  * - 18-tooth trigger wheel with missing tooth pattern
  * - Distorted sine wave output (not square wave)
  * - RPM control via potentiometer (0-13400 RPM)
  * - Precise timing using hardware timers
  * 
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "vr_sensor_emulator.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static VR_SensorState_t vr_state = {0};
extern ADC_HandleTypeDef hadc1;
extern DAC_HandleTypeDef hdac;
extern TIM_HandleTypeDef htim6;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static void VR_Emulator_UpdateTimerPeriod(void);
static float VR_Emulator_CalculateToothAngle(uint8_t tooth_index, float position_in_tooth);
static float VR_Emulator_ApplyDistortion(float base_sine, float angle);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
  * @brief  Initialize VR sensor emulator
  * @retval None
  */
void VR_Emulator_Init(void)
{
    // Initialize state structure
    vr_state.rpm_adc_value = 0;
    vr_state.target_rpm = 0;
    vr_state.tooth_period_us = 0;
    vr_state.current_tooth = 0;
    vr_state.tooth_timer = 0;
    vr_state.sine_phase = 0.0f;
    vr_state.dac_output = (uint16_t)(DAC_RESOLUTION * VR_DC_OFFSET);
    
    // Set initial DAC output to DC offset
    HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, vr_state.dac_output);
}

/**
  * @brief  Update VR sensor emulator (call from main loop)
  * @retval None
  */
void VR_Emulator_Update(void)
{
    // Read potentiometer and update target RPM
    uint16_t pot_value = VR_Emulator_ReadPotentiometer();
    uint16_t new_rpm = (uint32_t)pot_value * MAX_RPM / ADC_RESOLUTION;
    
    if (new_rpm != vr_state.target_rpm) {
        VR_Emulator_SetRPM(new_rpm);
    }
}

/**
  * @brief  Set target RPM
  * @param  rpm: Target RPM (0 to MAX_RPM)
  * @retval None
  */
void VR_Emulator_SetRPM(uint16_t rpm)
{
    if (rpm > MAX_RPM) {
        rpm = MAX_RPM;
    }
    
    vr_state.target_rpm = rpm;
    
    if (rpm > 0) {
        // Calculate tooth frequency and period
        float tooth_freq = RPM_TO_TOOTH_FREQ(rpm);
        vr_state.tooth_period_us = (uint32_t)TOOTH_FREQ_TO_PERIOD_US(tooth_freq);
        
        // Update timer period for precise timing
        VR_Emulator_UpdateTimerPeriod();
    } else {
        vr_state.tooth_period_us = 0;
        // Set DAC to DC offset when stopped
        vr_state.dac_output = (uint16_t)(DAC_RESOLUTION * VR_DC_OFFSET);
        HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, vr_state.dac_output);
    }
}

/**
  * @brief  Get current target RPM
  * @retval Current RPM setting
  */
uint16_t VR_Emulator_GetRPM(void)
{
    return vr_state.target_rpm;
}

/**
  * @brief  Read potentiometer value via ADC
  * @retval ADC value (0 to ADC_RESOLUTION-1)
  */
uint16_t VR_Emulator_ReadPotentiometer(void)
{
    HAL_ADC_Start(&hadc1);
    
    if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK) {
        vr_state.rpm_adc_value = HAL_ADC_GetValue(&hadc1);
    }
    
    HAL_ADC_Stop(&hadc1);
    
    return vr_state.rpm_adc_value;
}

/**
  * @brief  Timer callback for precise tooth timing
  * @retval None
  */
void VR_Emulator_TimerCallback(void)
{
    if (vr_state.target_rpm == 0) {
        return; // No signal generation when stopped
    }
    
    // Generate VR sensor signal
    VR_Emulator_GenerateSignal();
}

/**
  * @brief  Generate VR sensor signal
  * @retval None
  */
void VR_Emulator_GenerateSignal(void)
{
    static uint32_t phase_accumulator = 0;
    static uint8_t tooth_state = 0; // 0 = gap, 1 = tooth
    static uint32_t tooth_timer = 0;
    
    if (vr_state.tooth_period_us == 0) {
        return;
    }
    
    // Calculate time step based on timer frequency (1ms for TIM6)
    uint32_t time_step_us = 1000; // 1ms timer
    
    // Update tooth timing
    tooth_timer += time_step_us;
    
    // Calculate current tooth angle
    float tooth_angle = VR_Emulator_CalculateToothAngle(vr_state.current_tooth, 
                                                        (float)tooth_timer / vr_state.tooth_period_us);
    
    // Determine if we're in a tooth or gap
    uint8_t is_tooth_active = 0;
    
    if (vr_state.current_tooth == MISSING_TOOTH_INDEX) {
        // Missing tooth pattern: 12° tooth, 8° gap
        float tooth_fraction = (float)tooth_timer / vr_state.tooth_period_us;
        float tooth_width_fraction = MISSING_TOOTH_ANGLE / (MISSING_TOOTH_ANGLE + MISSING_TOOTH_GAP);
        is_tooth_active = (tooth_fraction < tooth_width_fraction) ? 1 : 0;
    } else {
        // Regular tooth pattern: 4° tooth, 16° gap
        float tooth_fraction = (float)tooth_timer / vr_state.tooth_period_us;
        float tooth_width_fraction = REGULAR_TOOTH_ANGLE / (REGULAR_TOOTH_ANGLE + REGULAR_TOOTH_GAP);
        is_tooth_active = (tooth_fraction < tooth_width_fraction) ? 1 : 0;
    }
    
    // Calculate DAC output value
    vr_state.dac_output = VR_Emulator_CalculateDAC_Value(tooth_angle, is_tooth_active);
    
    // Output to DAC
    HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, vr_state.dac_output);
    
    // Check if tooth period is complete
    if (tooth_timer >= vr_state.tooth_period_us) {
        tooth_timer = 0;
        vr_state.current_tooth = (vr_state.current_tooth + 1) % TRIGGER_WHEEL_TEETH;
    }
}

/**
  * @brief  Calculate DAC output value for given angle and tooth state
  * @param  angle: Current angle in radians
  * @param  tooth_active: 1 if tooth is active, 0 if in gap
  * @retval DAC value (0 to DAC_RESOLUTION-1)
  */
uint16_t VR_Emulator_CalculateDAC_Value(float angle, uint8_t tooth_active)
{
    float output_voltage = VR_DC_OFFSET; // Start with DC offset
    
    if (tooth_active) {
        // Generate distorted sine wave for tooth
        float base_sine = sinf(angle);
        
        // Apply distortion to make it more realistic
        float distorted_sine = VR_Emulator_ApplyDistortion(base_sine, angle);
        
        // Scale and add to DC offset
        output_voltage += distorted_sine * VR_AMPLITUDE_SCALE;
    }
    
    // Clamp to valid range
    if (output_voltage < 0.0f) output_voltage = 0.0f;
    if (output_voltage > 1.0f) output_voltage = 1.0f;
    
    // Convert to DAC value
    return (uint16_t)(output_voltage * DAC_RESOLUTION);
}

/**
  * @brief  Calculate tooth angle based on tooth index and position
  * @param  tooth_index: Current tooth index (0-17)
  * @param  position_in_tooth: Position within tooth period (0.0-1.0)
  * @retval Angle in radians
  */
static float VR_Emulator_CalculateToothAngle(uint8_t tooth_index, float position_in_tooth)
{
    // Calculate base angle for this tooth
    float tooth_base_angle = (float)tooth_index * (360.0f / TRIGGER_WHEEL_TEETH);
    
    // Add position within tooth
    float tooth_span = (tooth_index == MISSING_TOOTH_INDEX) ? 
                       (MISSING_TOOTH_ANGLE + MISSING_TOOTH_GAP) :
                       (REGULAR_TOOTH_ANGLE + REGULAR_TOOTH_GAP);
    
    float current_angle = tooth_base_angle + (position_in_tooth * tooth_span);
    
    return DEGREES_TO_RADIANS(current_angle);
}

/**
  * @brief  Apply distortion to base sine wave
  * @param  base_sine: Base sine wave value (-1.0 to 1.0)
  * @param  angle: Current angle in radians
  * @retval Distorted sine wave value
  */
static float VR_Emulator_ApplyDistortion(float base_sine, float angle)
{
    // Add harmonic distortion to make signal more realistic
    float harmonic2 = sinf(2.0f * angle) * VR_DISTORTION_FACTOR;
    float harmonic3 = sinf(3.0f * angle) * (VR_DISTORTION_FACTOR * 0.5f);
    
    // Add some asymmetry
    float asymmetry = (base_sine > 0) ? 0.1f * VR_DISTORTION_FACTOR : -0.05f * VR_DISTORTION_FACTOR;
    
    return base_sine + harmonic2 + harmonic3 + asymmetry;
}

/**
  * @brief  Update timer period based on current RPM
  * @retval None
  */
static void VR_Emulator_UpdateTimerPeriod(void)
{
    if (vr_state.target_rpm == 0) {
        return;
    }
    
    // Calculate required timer frequency for good resolution
    // We want at least 10 samples per tooth for good waveform quality
    float tooth_freq = RPM_TO_TOOTH_FREQ(vr_state.target_rpm);
    uint32_t required_timer_freq = (uint32_t)(tooth_freq * 10.0f * TRIGGER_WHEEL_TEETH);
    
    // Timer 6 runs at 108MHz with current prescaler (1079)
    // This gives us ~100kHz base frequency
    // Adjust ARR to get desired frequency
    uint32_t timer_base_freq = 100000; // 100kHz
    uint32_t arr_value = timer_base_freq / required_timer_freq;
    
    if (arr_value < 1) arr_value = 1;
    if (arr_value > 65535) arr_value = 65535;
    
    // Update timer period
    __HAL_TIM_SET_AUTORELOAD(&htim6, arr_value - 1);
}

/* USER CODE END 0 */
