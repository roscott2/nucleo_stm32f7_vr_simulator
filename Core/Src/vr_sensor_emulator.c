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
  * @brief  Update VR sensor emulator (deprecated - now handled by TIM2 interrupt)
  * @retval None
  */
void VR_Emulator_Update(void)
{
    // This function is deprecated in the new timer architecture
    // RPM updates are now handled automatically by TIM2 interrupt
    // calling VR_Emulator_UpdateRPM() every 50ms
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
        // Quantize RPM to 500 intervals for stable timing
        vr_state.quantized_rpm = (rpm / 500) * 500;
        if (vr_state.quantized_rpm == 0) vr_state.quantized_rpm = 500; // Minimum 500 RPM for quantization
        
        // Update TIM6 frequency for degree-based generation
        VR_Emulator_UpdateTIM6Frequency();
    } else {
        vr_state.quantized_rpm = 0;
        vr_state.current_degree = 0;
        
        // Set DAC to DC offset when stopped
        vr_state.dac_output = (uint16_t)(DAC_RESOLUTION * VR_DC_OFFSET);
        HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, vr_state.dac_output);
        
        // Stop TIM6
        HAL_TIM_Base_Stop_IT(&htim6);
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
  * @brief  Timer callback for VR sensor generation (called for each degree)
  * @retval None
  */
void VR_Emulator_TimerCallback(void)
{
    if (vr_state.target_rpm == 0) {
        return; // No output when stopped
    }
    
    // Generate waveform for current degree
    VR_Emulator_GenerateWaveformDegree();
    
    // Increment degree counter
    vr_state.current_degree++;
    if (vr_state.current_degree >= 360) {
        vr_state.current_degree = 0;
    }
}

/**
  * @brief  Update TIM6 frequency for quantized RPM
  * @retval None
  */
void VR_Emulator_UpdateTIM6Frequency(void)
{
    if (vr_state.quantized_rpm == 0) {
        HAL_TIM_Base_Stop_IT(&htim6);
        return;
    }
    
    // Calculate required frequency: RPM/60 * 360 (for 1 interrupt per degree)
    float required_freq = (float)vr_state.quantized_rpm * 6.0f; // RPM/60 * 360 = RPM * 6
    
    // Calculate prescaler for this frequency
    uint16_t new_prescaler = VR_Emulator_CalculatePrescaler(required_freq);
    
    if (new_prescaler != vr_state.tim6_prescaler) {
        vr_state.tim6_prescaler = new_prescaler;
        
        // Stop timer, update prescaler, restart
        HAL_TIM_Base_Stop_IT(&htim6);
        __HAL_TIM_SET_PRESCALER(&htim6, new_prescaler);
        HAL_TIM_Base_Start_IT(&htim6);
    } else if (!__HAL_TIM_GET_FLAG(&htim6, TIM_FLAG_UPDATE)) {
        // Timer not running, start it
        HAL_TIM_Base_Start_IT(&htim6);
    }
}

/**
  * @brief  Calculate prescaler for desired frequency
  * @param  frequency: Desired frequency in Hz
  * @retval Prescaler value
  */
uint16_t VR_Emulator_CalculatePrescaler(float frequency)
{
    // TIM6 clock = APB1 timer clock = 108MHz
    // Formula: frequency = timer_clock / ((prescaler + 1) * (ARR + 1))
    // With ARR = 9 (fixed), solve for prescaler
    
    const uint32_t timer_clock = 108000000; // 108MHz
    const uint32_t arr_value = 9;            // Fixed ARR for consistent timing
    
    if (frequency <= 0) {
        return 65535; // Maximum prescaler for very low frequencies
    }
    
    uint32_t prescaler = (timer_clock / (frequency * (arr_value + 1))) - 1;
    
    // Clamp to valid range
    if (prescaler > 65535) prescaler = 65535;
    if (prescaler < 1) prescaler = 1;
    
    return (uint16_t)prescaler;
}

/**
  * @brief  Generate waveform for current degree
  * @retval None
  */
void VR_Emulator_GenerateWaveformDegree(void)
{
    // Determine which tooth we're in based on current degree
    uint16_t tooth_number = vr_state.current_degree / 20; // 18 teeth * 20 degrees each
    uint16_t degree_in_tooth = vr_state.current_degree % 20;
    
    // Handle the case where we have 18 teeth (0-17)
    if (tooth_number >= TRIGGER_WHEEL_TEETH) {
        tooth_number = TRIGGER_WHEEL_TEETH - 1;
        degree_in_tooth = 19; // Last degree of last tooth
    }
    
    uint8_t is_tooth_active = 0;
    
    if (tooth_number == MISSING_TOOTH_INDEX) {
        // Missing tooth: 12° tooth, 8° gap
        is_tooth_active = (degree_in_tooth < 12) ? 1 : 0;
    } else {
        // Regular tooth: 4° tooth, 16° gap
        is_tooth_active = (degree_in_tooth < 4) ? 1 : 0;
    }
    
    // Calculate angle in radians for sine generation
    float angle_rad = DEGREES_TO_RADIANS((float)vr_state.current_degree);
    
    // Generate DAC output
    vr_state.dac_output = VR_Emulator_CalculateDAC_Value(angle_rad, is_tooth_active);
    
    // Output to DAC
    HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, vr_state.dac_output);
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
  * @brief  Update RPM from potentiometer (called by TIM2 interrupt)
  * @retval None
  */
void VR_Emulator_UpdateRPM(void)
{
    uint16_t pot_value = VR_Emulator_ReadPotentiometer();
    uint16_t new_rpm = (uint32_t)pot_value * MAX_RPM / ADC_RESOLUTION;
    
    if (new_rpm != vr_state.target_rpm) {
        VR_Emulator_SetRPM(new_rpm);
    }
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

/* USER CODE END 0 */
