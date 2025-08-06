/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : test_vr_emulator.h
  * @brief          : Header for VR sensor emulator unit tests
  ******************************************************************************
  * @attention
  *
  * Unit tests for VR Sensor Emulator for NUCLEO-STM32F7
  * Tests RPM input functionality from 0 to 13400 RPM
  * 
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TEST_VR_EMULATOR_H
#define __TEST_VR_EMULATOR_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/
typedef struct {
    uint16_t total_tests;
    uint16_t passed_tests;
    uint16_t failed_tests;
} TestResults_t;

/* Exported constants --------------------------------------------------------*/
/* Test configuration */
#define TEST_MODE_ENABLED           1       // Set to 1 to enable test mode
#define TEST_VERBOSE_OUTPUT         1       // Set to 1 for detailed test output

/* Test tolerance settings */
#define RPM_TEST_TOLERANCE          1       // RPM tolerance in units
#define FREQUENCY_TEST_TOLERANCE    0.5f    // Frequency tolerance in Hz
#define TIMING_TEST_TOLERANCE       2.0f    // Timing tolerance in percent

/* Exported macro ------------------------------------------------------------*/
/* Test assertion macros */
#define TEST_ENABLE_ASSERTIONS      1

#if TEST_ENABLE_ASSERTIONS
    #define TEST_ASSERT_TRUE(condition) \
        do { \
            if (!(condition)) { \
                printf("ASSERTION FAILED: %s at line %d\n", #condition, __LINE__); \
                return; \
            } \
        } while(0)
    
    #define TEST_ASSERT_FALSE(condition) \
        do { \
            if (condition) { \
                printf("ASSERTION FAILED: %s should be false at line %d\n", #condition, __LINE__); \
                return; \
            } \
        } while(0)
    
    #define TEST_ASSERT_EQUAL(expected, actual) \
        do { \
            if ((expected) != (actual)) { \
                printf("ASSERTION FAILED: Expected %d, got %d at line %d\n", \
                       (int)(expected), (int)(actual), __LINE__); \
                return; \
            } \
        } while(0)
#else
    #define TEST_ASSERT_TRUE(condition)         ((void)0)
    #define TEST_ASSERT_FALSE(condition)        ((void)0)
    #define TEST_ASSERT_EQUAL(expected, actual) ((void)0)
#endif

/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief  Run comprehensive VR emulator unit tests
  * @retval Test results structure containing pass/fail statistics
  */
TestResults_t VR_Emulator_RunTests(void);

/**
  * @brief  Test RPM calculation accuracy across full range
  * @param  start_rpm: Starting RPM for test range
  * @param  end_rpm: Ending RPM for test range
  * @param  step_rpm: RPM increment for each test point
  * @retval Test results for RPM range testing
  */
TestResults_t VR_Emulator_TestRPMRange(uint16_t start_rpm, uint16_t end_rpm, uint16_t step_rpm);

/**
  * @brief  Test ADC to RPM conversion accuracy
  * @param  num_test_points: Number of test points across ADC range
  * @retval Test results for ADC conversion testing
  */
TestResults_t VR_Emulator_TestADCConversion(uint16_t num_test_points);

/**
  * @brief  Test timing calculations for tooth generation
  * @param  test_rpms: Array of RPM values to test
  * @param  num_rpms: Number of RPM values in array
  * @retval Test results for timing calculations
  */
TestResults_t VR_Emulator_TestTimingCalculations(const uint16_t* test_rpms, uint16_t num_rpms);

/**
  * @brief  Validate tooth pattern timing at specific RPM
  * @param  rpm: RPM to test
  * @param  expected_tooth_period_us: Expected tooth period in microseconds
  * @retval True if timing is within tolerance, false otherwise
  */
bool VR_Emulator_ValidateToothTiming(uint16_t rpm, uint32_t expected_tooth_period_us);

/**
  * @brief  Test boundary conditions and edge cases
  * @retval Test results for boundary condition testing
  */
TestResults_t VR_Emulator_TestBoundaryConditions(void);

/**
  * @brief  Performance test for real-time operation
  * @param  test_duration_ms: Duration of performance test in milliseconds
  * @retval Test results for performance testing
  */
TestResults_t VR_Emulator_TestPerformance(uint32_t test_duration_ms);

/**
  * @brief  Test signal quality and distortion characteristics
  * @param  test_rpm: RPM to test signal quality
  * @retval Test results for signal quality testing
  */
TestResults_t VR_Emulator_TestSignalQuality(uint16_t test_rpm);

/**
  * @brief  Print detailed test report
  * @param  results: Test results to print
  * @retval None
  */
void VR_Emulator_PrintTestReport(const TestResults_t* results);

/**
  * @brief  Initialize test environment
  * @retval True if initialization successful, false otherwise
  */
bool VR_Emulator_InitTestEnvironment(void);

/**
  * @brief  Clean up test environment
  * @retval None
  */
void VR_Emulator_CleanupTestEnvironment(void);

/* Test utility functions */

/**
  * @brief  Calculate expected tooth frequency for given RPM
  * @param  rpm: RPM value
  * @retval Expected tooth frequency in Hz
  */
float VR_Test_CalculateToothFrequency(uint16_t rpm);

/**
  * @brief  Calculate expected tooth period for given RPM
  * @param  rpm: RPM value  
  * @retval Expected tooth period in microseconds
  */
uint32_t VR_Test_CalculateToothPeriod(uint16_t rpm);

/**
  * @brief  Convert ADC value to RPM using emulator formula
  * @param  adc_value: ADC reading (0-4095)
  * @retval Calculated RPM value
  */
uint16_t VR_Test_ADCToRPM(uint16_t adc_value);

/**
  * @brief  Convert RPM to expected ADC value
  * @param  rpm: RPM value
  * @retval Expected ADC reading
  */
uint16_t VR_Test_RPMToADC(uint16_t rpm);

/**
  * @brief  Validate if value is within tolerance
  * @param  actual: Actual measured value
  * @param  expected: Expected value
  * @param  tolerance_percent: Tolerance as percentage
  * @retval True if within tolerance, false otherwise
  */
bool VR_Test_IsWithinTolerance(float actual, float expected, float tolerance_percent);

/**
  * @brief  Get current system timestamp for performance testing
  * @retval Current timestamp in microseconds
  */
uint32_t VR_Test_GetTimestamp_us(void);

/* Test data arrays for comprehensive testing */
extern const uint16_t VR_Test_StandardRPMs[];
extern const uint16_t VR_Test_NumStandardRPMs;
extern const uint16_t VR_Test_BoundaryRPMs[];
extern const uint16_t VR_Test_NumBoundaryRPMs;

#ifdef __cplusplus
}
#endif

#endif /* __TEST_VR_EMULATOR_H */
