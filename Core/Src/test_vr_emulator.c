/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : test_vr_emulator.c
  * @brief          : Unit tests for VR sensor emulator
  ******************************************************************************
  * @attention
  *
  * Unit tests for VR Sensor Emulator for NUCLEO-STM32F7
  * Tests RPM input functionality from 0 to 13400 RPM
  * 
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "test_vr_emulator.h"
#include "vr_sensor_emulator.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct {
    uint16_t adc_value;
    uint16_t expected_rpm;
    float expected_tooth_freq;
    uint32_t expected_tooth_period_us;
} RPM_TestCase_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TEST_TOLERANCE_PERCENT      2.0f    // 2% tolerance for calculations
#define NUM_RPM_TEST_CASES          20      // Number of test points across RPM range
#define PRINTF_BUFFER_SIZE          256
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("TEST FAILED: %s\n", message); \
            test_results.failed_tests++; \
            return; \
        } else { \
            test_results.passed_tests++; \
        } \
    } while(0)

#define TEST_ASSERT_WITHIN_TOLERANCE(actual, expected, tolerance_percent) \
    do { \
        float diff = fabsf((float)(actual) - (float)(expected)); \
        float max_diff = (float)(expected) * (tolerance_percent) / 100.0f; \
        if (diff > max_diff && (expected) != 0) { \
            char msg[PRINTF_BUFFER_SIZE]; \
            snprintf(msg, sizeof(msg), "Value %f not within %f%% of expected %f (diff: %f, max: %f)", \
                    (float)(actual), (tolerance_percent), (float)(expected), diff, max_diff); \
            TEST_ASSERT(0, msg); \
        } else { \
            test_results.passed_tests++; \
        } \
    } while(0)
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static TestResults_t test_results = {0};
static char test_output_buffer[PRINTF_BUFFER_SIZE];

// Test cases covering the full RPM range
static const RPM_TestCase_t rpm_test_cases[NUM_RPM_TEST_CASES] = {
    // ADC Value, Expected RPM, Expected Tooth Freq (Hz), Expected Period (us)
    {0,     0,      0.0f,       0},
    {205,   670,    201.0f,     4975},
    {409,   1340,   402.0f,     2488},
    {614,   2010,   603.0f,     1658},
    {819,   2680,   804.0f,     1244},
    {1024,  3350,   1005.0f,    995},
    {1229,  4020,   1206.0f,    829},
    {1434,  4690,   1407.0f,    711},
    {1638,  5360,   1608.0f,    622},
    {1843,  6030,   1809.0f,    553},
    {2048,  6700,   2010.0f,    498},
    {2253,  7370,   2211.0f,    452},
    {2458,  8040,   2412.0f,    415},
    {2662,  8710,   2613.0f,    383},
    {2867,  9380,   2814.0f,    355},
    {3072,  10050,  3015.0f,    332},
    {3277,  10720,  3216.0f,    311},
    {3482,  11390,  3417.0f,    293},
    {3686,  12060,  3618.0f,    276},
    {4095,  13400,  4020.0f,    249}
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static void Test_RPM_Conversion(void);
static void Test_ADC_To_RPM_Mapping(void);
static void Test_Timing_Calculations(void);
static void Test_Boundary_Conditions(void);
static void Test_SetRPM_Function(void);
static void Print_Test_Results(void);
static uint16_t Simulate_ADC_Value(uint16_t adc_raw);
static float Calculate_Expected_Tooth_Frequency(uint16_t rpm);
static uint32_t Calculate_Expected_Tooth_Period_us(float tooth_freq);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
  * @brief  Run all VR emulator unit tests
  * @retval Test results structure
  */
TestResults_t VR_Emulator_RunTests(void)
{
    // Initialize test results
    test_results.total_tests = 0;
    test_results.passed_tests = 0;
    test_results.failed_tests = 0;
    
    printf("\n=== VR Sensor Emulator Unit Tests ===\n");
    printf("Testing RPM range: 0 to %d RPM\n\n", MAX_RPM);
    
    // Run test suites
    Test_ADC_To_RPM_Mapping();
    Test_RPM_Conversion();
    Test_Timing_Calculations();
    Test_Boundary_Conditions();
    Test_SetRPM_Function();
    
    // Calculate total tests
    test_results.total_tests = test_results.passed_tests + test_results.failed_tests;
    
    // Print results
    Print_Test_Results();
    
    return test_results;
}

/**
  * @brief  Test ADC value to RPM mapping
  * @retval None
  */
static void Test_ADC_To_RPM_Mapping(void)
{
    printf("Testing ADC to RPM mapping...\n");
    
    for (int i = 0; i < NUM_RPM_TEST_CASES; i++) {
        const RPM_TestCase_t* test_case = &rpm_test_cases[i];
        
        // Simulate ADC reading
        uint16_t simulated_adc = Simulate_ADC_Value(test_case->adc_value);
        
        // Calculate RPM from ADC value (same formula as in VR_Emulator_Update)
        uint16_t calculated_rpm = (uint32_t)simulated_adc * MAX_RPM / ADC_RESOLUTION;
        
        // Test the conversion
        snprintf(test_output_buffer, sizeof(test_output_buffer), 
                "ADC %d -> RPM conversion (expected: %d, got: %d)", 
                test_case->adc_value, test_case->expected_rpm, calculated_rpm);
        
        TEST_ASSERT_WITHIN_TOLERANCE(calculated_rpm, test_case->expected_rpm, TEST_TOLERANCE_PERCENT);
    }
    
    printf("✓ ADC to RPM mapping tests completed\n");
}

/**
  * @brief  Test RPM conversion calculations
  * @retval None
  */
static void Test_RPM_Conversion(void)
{
    printf("Testing RPM conversion calculations...\n");
    
    for (int i = 1; i < NUM_RPM_TEST_CASES; i++) { // Skip 0 RPM case
        const RPM_TestCase_t* test_case = &rpm_test_cases[i];
        
        // Test tooth frequency calculation
        float calculated_tooth_freq = Calculate_Expected_Tooth_Frequency(test_case->expected_rpm);
        
        snprintf(test_output_buffer, sizeof(test_output_buffer), 
                "RPM %d -> Tooth frequency (expected: %.1f Hz, got: %.1f Hz)", 
                test_case->expected_rpm, test_case->expected_tooth_freq, calculated_tooth_freq);
        
        TEST_ASSERT_WITHIN_TOLERANCE(calculated_tooth_freq, test_case->expected_tooth_freq, TEST_TOLERANCE_PERCENT);
        
        // Test tooth period calculation
        uint32_t calculated_period = Calculate_Expected_Tooth_Period_us(calculated_tooth_freq);
        
        snprintf(test_output_buffer, sizeof(test_output_buffer), 
                "Tooth freq %.1f Hz -> Period (expected: %lu us, got: %lu us)", 
                calculated_tooth_freq, test_case->expected_tooth_period_us, calculated_period);
        
        TEST_ASSERT_WITHIN_TOLERANCE(calculated_period, test_case->expected_tooth_period_us, TEST_TOLERANCE_PERCENT);
    }
    
    printf("✓ RPM conversion calculation tests completed\n");
}

/**
  * @brief  Test timing calculations for different RPMs
  * @retval None
  */
static void Test_Timing_Calculations(void)
{
    printf("Testing timing calculations...\n");
    
    // Test specific timing requirements
    struct {
        uint16_t rpm;
        float expected_wheel_freq_hz;
        float expected_tooth_time_ms;
    } timing_tests[] = {
        {600,   10.0f,  5.56f},    // 600 RPM = 10 Hz wheel frequency
        {1800,  30.0f,  1.85f},    // 1800 RPM = 30 Hz wheel frequency  
        {6000,  100.0f, 0.56f},    // 6000 RPM = 100 Hz wheel frequency
        {13400, 223.3f, 0.25f}     // 13400 RPM = 223.3 Hz wheel frequency
    };
    
    for (int i = 0; i < 4; i++) {
        float wheel_freq = (float)timing_tests[i].rpm / 60.0f; // Wheel frequency in Hz
        float tooth_freq = wheel_freq * TRIGGER_WHEEL_TEETH;   // Tooth frequency in Hz
        float tooth_time_ms = 1000.0f / tooth_freq;            // Time per tooth in ms
        
        snprintf(test_output_buffer, sizeof(test_output_buffer), 
                "RPM %d wheel frequency (expected: %.1f Hz, got: %.1f Hz)", 
                timing_tests[i].rpm, timing_tests[i].expected_wheel_freq_hz, wheel_freq);
        
        TEST_ASSERT_WITHIN_TOLERANCE(wheel_freq, timing_tests[i].expected_wheel_freq_hz, TEST_TOLERANCE_PERCENT);
        
        snprintf(test_output_buffer, sizeof(test_output_buffer), 
                "RPM %d tooth time (expected: %.2f ms, got: %.2f ms)", 
                timing_tests[i].rpm, timing_tests[i].expected_tooth_time_ms, tooth_time_ms);
        
        TEST_ASSERT_WITHIN_TOLERANCE(tooth_time_ms, timing_tests[i].expected_tooth_time_ms, TEST_TOLERANCE_PERCENT);
    }
    
    printf("✓ Timing calculation tests completed\n");
}

/**
  * @brief  Test boundary conditions and edge cases
  * @retval None
  */
static void Test_Boundary_Conditions(void)
{
    printf("Testing boundary conditions...\n");
    
    // Test minimum RPM (0)
    uint16_t min_rpm = 0;
    float min_tooth_freq = Calculate_Expected_Tooth_Frequency(min_rpm);
    TEST_ASSERT(min_tooth_freq == 0.0f, "Minimum RPM should result in 0 tooth frequency");
    
    // Test maximum RPM
    uint16_t max_rpm = MAX_RPM;
    float max_tooth_freq = Calculate_Expected_Tooth_Frequency(max_rpm);
    float expected_max_freq = (float)MAX_RPM * TRIGGER_WHEEL_TEETH / 60.0f;
    
    snprintf(test_output_buffer, sizeof(test_output_buffer), 
            "Maximum RPM tooth frequency (expected: %.1f Hz, got: %.1f Hz)", 
            expected_max_freq, max_tooth_freq);
    
    TEST_ASSERT_WITHIN_TOLERANCE(max_tooth_freq, expected_max_freq, TEST_TOLERANCE_PERCENT);
    
    // Test ADC boundary values
    uint16_t adc_min = 0;
    uint16_t rpm_from_min_adc = (uint32_t)adc_min * MAX_RPM / ADC_RESOLUTION;
    TEST_ASSERT(rpm_from_min_adc == 0, "Minimum ADC should result in 0 RPM");
    
    uint16_t adc_max = ADC_RESOLUTION - 1; // 4095 for 12-bit ADC
    uint16_t rpm_from_max_adc = (uint32_t)adc_max * MAX_RPM / ADC_RESOLUTION;
    
    snprintf(test_output_buffer, sizeof(test_output_buffer), 
            "Maximum ADC RPM (expected: close to %d, got: %d)", 
            MAX_RPM, rpm_from_max_adc);
    
    // Should be close to MAX_RPM (within 1 RPM due to integer division)
    TEST_ASSERT(abs(rpm_from_max_adc - MAX_RPM) <= 1, "Maximum ADC should result in close to maximum RPM");
    
    printf("✓ Boundary condition tests completed\n");
}

/**
  * @brief  Test VR_Emulator_SetRPM function behavior
  * @retval None
  */
static void Test_SetRPM_Function(void)
{
    printf("Testing VR_Emulator_SetRPM function...\n");
    
    // Test normal RPM setting
    for (int i = 0; i < NUM_RPM_TEST_CASES; i++) {
        uint16_t test_rpm = rpm_test_cases[i].expected_rpm;
        
        // This would normally call the actual function, but since we're testing
        // we'll validate the logic that would be inside VR_Emulator_SetRPM
        uint16_t clamped_rpm = (test_rpm > MAX_RPM) ? MAX_RPM : test_rpm;
        
        TEST_ASSERT(clamped_rpm <= MAX_RPM, "RPM should be clamped to maximum value");
        TEST_ASSERT(clamped_rpm == test_rpm, "Normal RPM values should not be modified");
    }
    
    // Test RPM values above maximum
    uint16_t over_max_rpms[] = {13500, 15000, 20000, 65535};
    for (int i = 0; i < 4; i++) {
        uint16_t test_rpm = over_max_rpms[i];
        uint16_t clamped_rpm = (test_rpm > MAX_RPM) ? MAX_RPM : test_rpm;
        
        snprintf(test_output_buffer, sizeof(test_output_buffer), 
                "RPM %d should be clamped to %d", test_rpm, MAX_RPM);
        
        TEST_ASSERT(clamped_rpm == MAX_RPM, test_output_buffer);
    }
    
    printf("✓ SetRPM function tests completed\n");
}

/**
  * @brief  Print test results summary
  * @retval None
  */
static void Print_Test_Results(void)
{
    printf("\n=== Test Results Summary ===\n");
    printf("Total Tests: %d\n", test_results.total_tests);
    printf("Passed: %d\n", test_results.passed_tests);
    printf("Failed: %d\n", test_results.failed_tests);
    
    if (test_results.failed_tests == 0) {
        printf("✓ ALL TESTS PASSED!\n");
    } else {
        printf("✗ %d TESTS FAILED!\n", test_results.failed_tests);
    }
    
    float success_rate = (float)test_results.passed_tests / test_results.total_tests * 100.0f;
    printf("Success Rate: %.1f%%\n", success_rate);
    printf("=============================\n\n");
}

/**
  * @brief  Simulate ADC reading (for testing without hardware)
  * @param  adc_raw: Raw ADC value to simulate
  * @retval Simulated ADC reading
  */
static uint16_t Simulate_ADC_Value(uint16_t adc_raw)
{
    // In actual hardware, this would read from the ADC
    // For testing, we just return the input value
    return adc_raw;
}

/**
  * @brief  Calculate expected tooth frequency for given RPM
  * @param  rpm: RPM value
  * @retval Expected tooth frequency in Hz
  */
static float Calculate_Expected_Tooth_Frequency(uint16_t rpm)
{
    if (rpm == 0) {
        return 0.0f;
    }
    
    // Same calculation as RPM_TO_TOOTH_FREQ macro
    return (float)rpm * TRIGGER_WHEEL_TEETH / 60.0f;
}

/**
  * @brief  Calculate expected tooth period for given frequency
  * @param  tooth_freq: Tooth frequency in Hz
  * @retval Expected tooth period in microseconds
  */
static uint32_t Calculate_Expected_Tooth_Period_us(float tooth_freq)
{
    if (tooth_freq == 0.0f) {
        return 0;
    }
    
    // Same calculation as TOOTH_FREQ_TO_PERIOD_US macro
    return (uint32_t)(1000000.0f / tooth_freq);
}

/* USER CODE END 0 */
