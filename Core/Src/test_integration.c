/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : test_integration.c
  * @brief          : Test integration and runner for VR emulator tests
  ******************************************************************************
  * @attention
  *
  * Test integration for VR Sensor Emulator for NUCLEO-STM32F7
  * Provides easy integration with main application
  * 
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "test_integration.h"
#include "test_vr_emulator.h"
#include "vr_sensor_emulator.h"
#include <stdio.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TEST_DELAY_MS               1000    // Delay between test suites
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static bool test_mode_enabled = false;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static void Run_Quick_Tests(void);
static void Run_Comprehensive_Tests(void);
static void Print_Test_Header(void);
static void Print_Test_Footer(const TestResults_t* overall_results);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
  * @brief  Initialize test mode for VR emulator
  * @retval None
  */
void VR_Test_Init(void)
{
    test_mode_enabled = true;
    
    printf("\n");
    printf("======================================\n");
    printf("  VR Sensor Emulator Test Suite\n");
    printf("======================================\n");
    printf("Target: NUCLEO-STM32F7\n");
    printf("RPM Range: 0 - %d RPM\n", MAX_RPM);
    printf("Test Mode: ENABLED\n");
    printf("======================================\n\n");
}

/**
  * @brief  Run basic functionality tests
  * @retval Overall test results
  */
TestResults_t VR_Test_RunBasic(void)
{
    TestResults_t results = {0};
    
    if (!test_mode_enabled) {
        printf("Error: Test mode not initialized. Call VR_Test_Init() first.\n");
        return results;
    }
    
    Print_Test_Header();
    printf("Running BASIC test suite...\n\n");
    
    // Run basic tests
    results = VR_Emulator_RunTests();
    
    Print_Test_Footer(&results);
    return results;
}

/**
  * @brief  Run comprehensive test suite
  * @retval Overall test results
  */
TestResults_t VR_Test_RunComprehensive(void)
{
    TestResults_t overall_results = {0};
    TestResults_t suite_results = {0};
    
    if (!test_mode_enabled) {
        printf("Error: Test mode not initialized. Call VR_Test_Init() first.\n");
        return overall_results;
    }
    
    Print_Test_Header();
    printf("Running COMPREHENSIVE test suite...\n\n");
    
    // Test Suite 1: Basic functionality
    printf("=== Test Suite 1: Basic Functionality ===\n");
    suite_results = VR_Emulator_RunTests();
    overall_results.total_tests += suite_results.total_tests;
    overall_results.passed_tests += suite_results.passed_tests;
    overall_results.failed_tests += suite_results.failed_tests;
    
    HAL_Delay(TEST_DELAY_MS);
    
    // Test Suite 2: RPM Range Testing
    printf("\n=== Test Suite 2: RPM Range Testing ===\n");
    suite_results = VR_Emulator_TestRPMRange(0, MAX_RPM, 100);
    overall_results.total_tests += suite_results.total_tests;
    overall_results.passed_tests += suite_results.passed_tests;
    overall_results.failed_tests += suite_results.failed_tests;
    
    HAL_Delay(TEST_DELAY_MS);
    
    // Test Suite 3: ADC Conversion Testing
    printf("\n=== Test Suite 3: ADC Conversion Testing ===\n");
    suite_results = VR_Emulator_TestADCConversion(50);
    overall_results.total_tests += suite_results.total_tests;
    overall_results.passed_tests += suite_results.passed_tests;
    overall_results.failed_tests += suite_results.failed_tests;
    
    HAL_Delay(TEST_DELAY_MS);
    
    // Test Suite 4: Boundary Conditions
    printf("\n=== Test Suite 4: Boundary Conditions ===\n");
    suite_results = VR_Emulator_TestBoundaryConditions();
    overall_results.total_tests += suite_results.total_tests;
    overall_results.passed_tests += suite_results.passed_tests;
    overall_results.failed_tests += suite_results.failed_tests;
    
    HAL_Delay(TEST_DELAY_MS);
    
    // Test Suite 5: Performance Testing
    printf("\n=== Test Suite 5: Performance Testing ===\n");
    suite_results = VR_Emulator_TestPerformance(5000);
    overall_results.total_tests += suite_results.total_tests;
    overall_results.passed_tests += suite_results.passed_tests;
    overall_results.failed_tests += suite_results.failed_tests;
    
    Print_Test_Footer(&overall_results);
    return overall_results;
}

/**
  * @brief  Run RPM sweep test from 0 to max RPM
  * @param  step_size: RPM increment for each test point
  * @retval Test results
  */
TestResults_t VR_Test_RunRPMSweep(uint16_t step_size)
{
    TestResults_t results = {0};
    
    if (!test_mode_enabled) {
        printf("Error: Test mode not initialized. Call VR_Test_Init() first.\n");
        return results;
    }
    
    printf("\n=== RPM SWEEP TEST ===\n");
    printf("Testing RPM range: 0 to %d in steps of %d\n", MAX_RPM, step_size);
    printf("Expected test points: %d\n\n", (MAX_RPM / step_size) + 1);
    
    // Run RPM range test
    results = VR_Emulator_TestRPMRange(0, MAX_RPM, step_size);
    
    printf("\nRPM Sweep Test Results:\n");
    printf("- Test Points: %d\n", results.total_tests);
    printf("- Passed: %d\n", results.passed_tests);
    printf("- Failed: %d\n", results.failed_tests);
    printf("- Success Rate: %.1f%%\n", 
           (float)results.passed_tests / results.total_tests * 100.0f);
    
    return results;
}

/**
  * @brief  Run specific RPM validation test
  * @param  rpm: Specific RPM to test
  * @retval True if test passed, false otherwise
  */
bool VR_Test_ValidateRPM(uint16_t rpm)
{
    if (!test_mode_enabled) {
        printf("Error: Test mode not initialized. Call VR_Test_Init() first.\n");
        return false;
    }
    
    printf("Testing RPM: %d\n", rpm);
    
    // Calculate expected values
    float expected_tooth_freq = VR_Test_CalculateToothFrequency(rpm);
    uint32_t expected_period_us = VR_Test_CalculateToothPeriod(rpm);
    uint16_t expected_adc = VR_Test_RPMToADC(rpm);
    
    printf("- Expected tooth frequency: %.2f Hz\n", expected_tooth_freq);
    printf("- Expected tooth period: %lu us\n", expected_period_us);
    printf("- Expected ADC value: %d\n", expected_adc);
    
    // Validate timing
    bool timing_valid = VR_Emulator_ValidateToothTiming(rpm, expected_period_us);
    
    // Test ADC conversion
    uint16_t converted_rpm = VR_Test_ADCToRPM(expected_adc);
    bool adc_valid = VR_Test_IsWithinTolerance(converted_rpm, rpm, 2.0f);
    
    printf("- Timing validation: %s\n", timing_valid ? "PASS" : "FAIL");
    printf("- ADC conversion: %s (got %d RPM)\n", adc_valid ? "PASS" : "FAIL", converted_rpm);
    
    bool overall_pass = timing_valid && adc_valid;
    printf("- Overall result: %s\n", overall_pass ? "PASS" : "FAIL");
    
    return overall_pass;
}

/**
  * @brief  Print test header
  * @retval None
  */
static void Print_Test_Header(void)
{
    printf("\n┌─────────────────────────────────────┐\n");
    printf("│           Test Execution           │\n");  
    printf("└─────────────────────────────────────┘\n");
}

/**
  * @brief  Print test footer with results
  * @param  overall_results: Test results to display
  * @retval None
  */
static void Print_Test_Footer(const TestResults_t* overall_results)
{
    printf("\n┌─────────────────────────────────────┐\n");
    printf("│          Final Results              │\n");
    printf("├─────────────────────────────────────┤\n");
    printf("│ Total Tests: %-20d │\n", overall_results->total_tests);
    printf("│ Passed:      %-20d │\n", overall_results->passed_tests);
    printf("│ Failed:      %-20d │\n", overall_results->failed_tests);
    
    float success_rate = 0.0f;
    if (overall_results->total_tests > 0) {
        success_rate = (float)overall_results->passed_tests / overall_results->total_tests * 100.0f;
    }
    
    printf("│ Success Rate: %-18.1f%% │\n", success_rate);
    printf("├─────────────────────────────────────┤\n");
    
    if (overall_results->failed_tests == 0) {
        printf("│          ✓ ALL TESTS PASSED        │\n");
    } else {
        printf("│         ✗ SOME TESTS FAILED        │\n");
    }
    
    printf("└─────────────────────────────────────┘\n\n");
}

/**
  * @brief  Demo mode - shows VR emulator functionality
  * @retval None
  */
void VR_Test_RunDemo(void)
{
    if (!test_mode_enabled) {
        printf("Error: Test mode not initialized. Call VR_Test_Init() first.\n");
        return;
    }
    
    printf("\n=== VR EMULATOR DEMO MODE ===\n");
    printf("Demonstrating VR sensor output at various RPMs...\n\n");
    
    // Demo RPM values
    uint16_t demo_rpms[] = {0, 800, 1500, 3000, 6000, 9000, 13400};
    uint8_t num_demo_rpms = sizeof(demo_rpms) / sizeof(demo_rpms[0]);
    
    for (uint8_t i = 0; i < num_demo_rpms; i++) {
        uint16_t rpm = demo_rpms[i];
        
        printf("Setting RPM to: %d\n", rpm);
        
        // In a real scenario, this would actually set the RPM
        // VR_Emulator_SetRPM(rpm);
        
        // Calculate and display characteristics
        float tooth_freq = VR_Test_CalculateToothFrequency(rpm);
        uint32_t tooth_period = VR_Test_CalculateToothPeriod(rpm);
        
        printf("  - Tooth frequency: %.2f Hz\n", tooth_freq);
        printf("  - Tooth period: %lu us\n", tooth_period);
        
        if (rpm > 0) {
            float wheel_rps = (float)rpm / 60.0f;
            printf("  - Wheel speed: %.2f RPS\n", wheel_rps);
        }
        
        printf("\n");
        
        // Delay to simulate real operation
        HAL_Delay(2000);
    }
    
    printf("Demo completed.\n");
}

/* USER CODE END 0 */
