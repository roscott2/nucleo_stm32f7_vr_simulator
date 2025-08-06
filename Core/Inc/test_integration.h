/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : test_integration.h
  * @brief          : Header for test integration and runner
  ******************************************************************************
  * @attention
  *
  * Test integration for VR Sensor Emulator for NUCLEO-STM32F7
  * Provides easy integration with main application
  * 
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TEST_INTEGRATION_H
#define __TEST_INTEGRATION_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "test_vr_emulator.h"
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/
/* Test modes */
typedef enum {
    VR_TEST_MODE_BASIC = 0,
    VR_TEST_MODE_COMPREHENSIVE,
    VR_TEST_MODE_RPM_SWEEP,
    VR_TEST_MODE_DEMO,
    VR_TEST_MODE_CUSTOM
} VR_TestMode_t;

/* Exported constants --------------------------------------------------------*/
/* Test configuration */
#define VR_TEST_ENABLED             1       // Set to 1 to enable testing
#define VR_TEST_AUTO_RUN            0       // Set to 1 to auto-run tests on startup
#define VR_TEST_DEFAULT_MODE        VR_TEST_MODE_BASIC

/* Exported macro ------------------------------------------------------------*/
/* Test control macros */
#define VR_TEST_ENABLE()            VR_Test_Init()
#define VR_TEST_RUN()               VR_Test_RunBasic()
#define VR_TEST_RUN_ALL()           VR_Test_RunComprehensive()

/* Conditional compilation for test mode */
#if VR_TEST_ENABLED
    #define VR_TEST_CALL(func)      func
    #define VR_TEST_PRINTF(...)     printf(__VA_ARGS__)
#else
    #define VR_TEST_CALL(func)      ((void)0)
    #define VR_TEST_PRINTF(...)     ((void)0)
#endif

/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief  Initialize test environment and enable test mode
  * @retval None
  */
void VR_Test_Init(void);

/**
  * @brief  Run basic functionality tests
  * @retval Overall test results
  */
TestResults_t VR_Test_RunBasic(void);

/**
  * @brief  Run comprehensive test suite (all tests)
  * @retval Overall test results  
  */
TestResults_t VR_Test_RunComprehensive(void);

/**
  * @brief  Run RPM sweep test from 0 to max RPM
  * @param  step_size: RPM increment for each test point
  * @retval Test results
  */
TestResults_t VR_Test_RunRPMSweep(uint16_t step_size);

/**
  * @brief  Run specific RPM validation test
  * @param  rpm: Specific RPM to test
  * @retval True if test passed, false otherwise
  */
bool VR_Test_ValidateRPM(uint16_t rpm);

/**
  * @brief  Demo mode - shows VR emulator functionality
  * @retval None
  */
void VR_Test_RunDemo(void);

/**
  * @brief  Quick health check test
  * @retval True if basic functionality works, false otherwise
  */
bool VR_Test_HealthCheck(void);

/**
  * @brief  Test specific ADC value and its RPM conversion
  * @param  adc_value: ADC value to test (0-4095)
  * @retval True if conversion is accurate, false otherwise
  */
bool VR_Test_ValidateADCValue(uint16_t adc_value);

/**
  * @brief  Run automated test sequence based on button press
  * @retval Test results
  */
TestResults_t VR_Test_RunAutomated(void);

/* Test utility functions for main application */

/**
  * @brief  Check if test mode is currently enabled
  * @retval True if test mode is active, false otherwise
  */
bool VR_Test_IsEnabled(void);

/**
  * @brief  Enable or disable test output
  * @param  enable: True to enable test output, false to disable
  * @retval None
  */
void VR_Test_SetOutputEnabled(bool enable);

/**
  * @brief  Get current test mode
  * @retval Current test mode
  */
VR_TestMode_t VR_Test_GetMode(void);

/**
  * @brief  Set test mode
  * @param  mode: Test mode to set
  * @retval None
  */
void VR_Test_SetMode(VR_TestMode_t mode);

/* Integration with main application */

/**
  * @brief  Main test entry point - call from main()
  * @retval None
  */
void VR_Test_Main(void);

/**
  * @brief  Periodic test update - call from main loop
  * @retval None
  */
void VR_Test_Update(void);

/**
  * @brief  Button press handler for test control
  * @retval None
  */
void VR_Test_OnButtonPress(void);

/**
  * @brief  Test completion callback
  * @param  results: Test results
  * @retval None
  */
void VR_Test_OnComplete(const TestResults_t* results);

#ifdef __cplusplus
}
#endif

#endif /* __TEST_INTEGRATION_H */
