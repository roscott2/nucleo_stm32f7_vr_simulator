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
        // Calculate timing periods based on RPM
        vr_state.revolution_period_us = (60000000UL / rpm); // Revolution period in microseconds
        vr_state.tooth_period_us = vr_state.revolution_period_us / TRIGGER_WHEEL_TEETH; // Period per tooth
        
        // Start TIM6 at fixed 100kHz if not already running
        if (HAL_TIM_Base_GetState(&htim6) != HAL_TIM_STATE_BUSY) {
            HAL_TIM_Base_Start_IT(&htim6);
        }
    } else {
        vr_state.revolution_period_us = 0;
        vr_state.tooth_period_us = 0;
        vr_state.revolution_timer_us = 0;
        vr_state.tooth_timer_us = 0;
        vr_state.current_tooth = 0;
        
        // Set DAC to DC offset when stopped
        vr_state.dac_output = (uint16_t)(DAC_RESOLUTION * 0.5f); // 50% for 1.65V DC offset
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
  * @brief  Timer callback for VR sensor generation (called at fixed 100kHz rate)
  * @retval None
  */
void VR_Emulator_TimerCallback(void)
{
    if (vr_state.target_rpm == 0) {
        return; // No output when stopped
    }
    
    // Generate waveform at fixed 100kHz rate
    VR_Emulator_GenerateWaveformFixed();
    
    // Increment time counters (10μs per interrupt at 100kHz)
    vr_state.revolution_timer_us += 10;
    vr_state.tooth_timer_us += 10;
    
    // Check if tooth period is complete
    if (vr_state.tooth_timer_us >= vr_state.tooth_period_us) {
        vr_state.tooth_timer_us = 0;
        vr_state.current_tooth = (vr_state.current_tooth + 1) % TRIGGER_WHEEL_TEETH;
    }
    
    // Check if full revolution is complete  
    if (vr_state.revolution_timer_us >= vr_state.revolution_period_us) {
        vr_state.revolution_timer_us = 0;
        vr_state.current_tooth = 0; // Reset to first tooth
        vr_state.tooth_timer_us = 0;
    }
}

/**
  * @brief  Generate waveform at fixed 100kHz rate
  * @retval None
  */
void VR_Emulator_GenerateWaveformFixed(void)
{
    if (vr_state.tooth_period_us == 0) {
        // No RPM set, output DC offset
        vr_state.dac_output = (uint16_t)(DAC_RESOLUTION * 0.5f);
        HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, vr_state.dac_output);
        return;
    }
    
    // Determine tooth timing characteristics
    uint32_t tooth_active_us, tooth_gap_us;
    
    if (vr_state.current_tooth == MISSING_TOOTH_INDEX) {
        // Missing tooth: 12° tooth, 8° gap out of 20° total
        tooth_active_us = (vr_state.tooth_period_us * 12) / 20;   // 12/20 = 60%
        tooth_gap_us = (vr_state.tooth_period_us * 8) / 20;       // 8/20 = 40%  
    } else {
        // Regular tooth: 4° tooth, 16° gap out of 20° total
        tooth_active_us = (vr_state.tooth_period_us * 4) / 20;    // 4/20 = 20%
        tooth_gap_us = (vr_state.tooth_period_us * 16) / 20;      // 16/20 = 80%
    }
    
    uint8_t is_tooth_active = (vr_state.tooth_timer_us < tooth_active_us) ? 1 : 0;
    
    // Calculate DAC output based on position within tooth or gap
    if (is_tooth_active) {
        // In tooth active period - generate rising sine
        float progress = (float)vr_state.tooth_timer_us / (float)tooth_active_us;
        vr_state.dac_output = VR_Emulator_CalculateDAC_Value_Fixed(progress, 1);
    } else {
        // In gap period - generate falling sine
        uint32_t gap_time = vr_state.tooth_timer_us - tooth_active_us;
        float progress = (float)gap_time / (float)tooth_gap_us;
        vr_state.dac_output = VR_Emulator_CalculateDAC_Value_Fixed(progress, 0);
    }
    
    // Output to DAC
    HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, vr_state.dac_output);
}

/**
  * @brief  Calculate DAC output value for fixed-rate generation
  * @param  progress: Position progress within tooth/gap period (0.0 to 1.0)
  * @param  tooth_active: 1 for tooth active period, 0 for gap period
  * @retval DAC value (0 to DAC_RESOLUTION-1)
  */
uint16_t VR_Emulator_CalculateDAC_Value_Fixed(float progress, uint8_t tooth_active)
{
    float output_voltage = 0.5f; // Start with 50% DC offset (1.65V)
    
    // Clamp progress to valid range
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;
    
    if (tooth_active) {
        // Tooth active: Generate sine wave from DC to peak and back
        // Peak occurs at progress = 0.5 (middle of tooth active period)
        float sine_angle = progress * M_PI; // 0 to π
        float sine_value = sinf(sine_angle); // 0 to 1 to 0
        
        // Apply distortion for realism
        float distorted_sine = VR_Emulator_ApplyDistortion(sine_value, sine_angle);
        
        // Scale to upper half of DAC range (1.65V to 3.3V)
        output_voltage = 0.5f + (distorted_sine * 0.5f);
        
    } else {
        // Gap period: Generate inverted sine below DC offset
        // Minimum occurs at progress = 0.5 (middle of gap period)
        float sine_angle = progress * M_PI; // 0 to π  
        float sine_value = sinf(sine_angle); // 0 to 1 to 0
        
        // Apply distortion
        float distorted_sine = VR_Emulator_ApplyDistortion(sine_value, sine_angle);
        
        // Scale to lower half of DAC range (0V to 1.65V)
        output_voltage = 0.5f - (distorted_sine * 0.5f);
    }
    
    // Clamp to valid range
    if (output_voltage < 0.0f) output_voltage = 0.0f;
    if (output_voltage > 1.0f) output_voltage = 1.0f;
    
    // Convert to DAC value (0-4095 for 12-bit DAC)
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
  * @brief  Apply distortion to base sine wave for VR sensor realism
  * @param  base_sine: Base sine wave value (0.0 to 1.0)
  * @param  angle: Current angle in radians within tooth
  * @retval Distorted sine wave value
  */
static float VR_Emulator_ApplyDistortion(float base_sine, float angle)
{
    // Add subtle harmonic distortion for VR sensor realism
    float harmonic2 = sinf(2.0f * angle) * VR_DISTORTION_FACTOR * 0.3f;
    float harmonic3 = sinf(3.0f * angle) * VR_DISTORTION_FACTOR * 0.1f;
    
    // Add slight asymmetry to simulate real VR sensor characteristics
    float asymmetry = base_sine * 0.05f * VR_DISTORTION_FACTOR;
    
    // Combine base sine with distortions
    float distorted = base_sine + harmonic2 + harmonic3 + asymmetry;
    
    // Clamp to valid range (0.0 to 1.0)
    if (distorted < 0.0f) distorted = 0.0f;
    if (distorted > 1.0f) distorted = 1.0f;
    
    return distorted;
}

/* USER CODE END 0 */
