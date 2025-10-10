/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : test_map_emulator.c
  * @brief          : Unit tests for MAP sensor emulator
  ******************************************************************************
  * @attention
  *
  * Unit tests for MAP Sensor Emulator for NUCLEO-STM32F7
  * Tests MAP sensor pressure calculations, TPS response, and DAC output
  * 
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "test_map_emulator.h"
#include "map_sensor_emulator.h"
#include <stdio.h>
#include <math.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MAP_TEST_TOLERANCE_KPA      2.0f    // Tolerance for pressure tests (kPa)
#define MAP_TEST_TOLERANCE_VOLTAGE  0.1f    // Tolerance for voltage tests (V)
#define MAP_TEST_TOLERANCE_PERCENT  2.0f    // Tolerance for percentage tests (%)
#define MAP_TEST_DELAY_MS          10       // Delay between test steps

/* Test point definitions for MAP sensor */
#define MAP_TEST_POINTS            20       // Number of test points across range
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define MAP_TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("FAIL: %s\n", message); \
            results.failed_tests++; \
        } else { \
            printf("PASS: %s\n", message); \
            results.passed_tests++; \
        } \
        results.total_tests++; \
    } while(0)

#define MAP_TEST_ASSERT_NEAR_FLOAT(actual, expected, tolerance, message) \
    MAP_TEST_ASSERT(fabsf((actual) - (expected)) <= (tolerance), message)

#define MAP_TEST_ASSERT_RANGE_FLOAT(value, min_val, max_val, message) \
    MAP_TEST_ASSERT(((value) >= (min_val)) && ((value) <= (max_val)), message)
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static TestResults_t Test_MAP_Initialization(void);
static TestResults_t Test_MAP_PressureCalculations(void);
static TestResults_t Test_MAP_TPSResponse(void);
static TestResults_t Test_MAP_VoltageConversion(void);
static TestResults_t Test_MAP_DACOutput(void);
static TestResults_t Test_MAP_BoundaryConditions(void);
static TestResults_t Test_MAP_CoordinatedOperation(void);
static void Print_MAP_Test_Status(void);
/* USER CODE END PFP */

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  Run all MAP emulator tests
  * @retval Test results structure
  */
TestResults_t MAP_Emulator_RunTests(void)
{
    TestResults_t overall_results = {0};
    TestResults_t test_results = {0};
    
    printf("\n=== MAP SENSOR EMULATOR TESTS ===\n");
    
    // Initialize MAP emulator for testing
    MAP_Emulator_Init();
    HAL_Delay(MAP_TEST_DELAY_MS);
    
    // Test 1: Initialization
    printf("\n--- Test 1: MAP Initialization ---\n");
    test_results = Test_MAP_Initialization();
    overall_results.total_tests += test_results.total_tests;
    overall_results.passed_tests += test_results.passed_tests;
    overall_results.failed_tests += test_results.failed_tests;
    
    // Test 2: Pressure Calculations
    printf("\n--- Test 2: Pressure Calculations ---\n");
    test_results = Test_MAP_PressureCalculations();
    overall_results.total_tests += test_results.total_tests;
    overall_results.passed_tests += test_results.passed_tests;
    overall_results.failed_tests += test_results.failed_tests;
    
    // Test 3: TPS Response
    printf("\n--- Test 3: TPS Response ---\n");
    test_results = Test_MAP_TPSResponse();
    overall_results.total_tests += test_results.total_tests;
    overall_results.passed_tests += test_results.passed_tests;
    overall_results.failed_tests += test_results.failed_tests;
    
    // Test 4: Voltage Conversion
    printf("\n--- Test 4: Voltage Conversion ---\n");
    test_results = Test_MAP_VoltageConversion();
    overall_results.total_tests += test_results.total_tests;
    overall_results.passed_tests += test_results.passed_tests;
    overall_results.failed_tests += test_results.failed_tests;
    
    // Test 5: DAC Output
    printf("\n--- Test 5: DAC Output ---\n");
    test_results = Test_MAP_DACOutput();
    overall_results.total_tests += test_results.total_tests;
    overall_results.passed_tests += test_results.passed_tests;
    overall_results.failed_tests += test_results.failed_tests;
    
    // Test 6: Boundary Conditions
    printf("\n--- Test 6: Boundary Conditions ---\n");
    test_results = Test_MAP_BoundaryConditions();
    overall_results.total_tests += test_results.total_tests;
    overall_results.passed_tests += test_results.passed_tests;
    overall_results.failed_tests += test_results.failed_tests;
    
    // Test 7: Coordinated Operation
    printf("\n--- Test 7: Coordinated Operation ---\n");
    test_results = Test_MAP_CoordinatedOperation();
    overall_results.total_tests += test_results.total_tests;
    overall_results.passed_tests += test_results.passed_tests;
    overall_results.failed_tests += test_results.failed_tests;
    
    // Print overall results
    printf("\n=== MAP SENSOR TEST SUMMARY ===\n");
    printf("Total Tests: %d\n", overall_results.total_tests);
    printf("Passed: %d\n", overall_results.passed_tests);
    printf("Failed: %d\n", overall_results.failed_tests);
    printf("Success Rate: %.1f%%\n", 
           (float)overall_results.passed_tests * 100.0f / overall_results.total_tests);
    
    return overall_results;
}

/**
  * @brief  Test MAP sensor pressure sweep across TPS range
  * @retval Test results structure
  */
TestResults_t MAP_Emulator_TestPressureSweep(void)
{
    TestResults_t results = {0};
    
    printf("\n=== MAP PRESSURE SWEEP TEST ===\n");
    printf("Testing pressure response across TPS range (0-100%%)\n");
    printf("TPS%%\tPressure(kPa)\tPressure(PSI)\tVoltage(V)\tDAC\tCondition\n");
    printf("----\t-------------\t-------------\t----------\t---\t---------\n");
    
    for (int i = 0; i <= MAP_TEST_POINTS; i++) {
        float tps_percent = (float)i * 100.0f / MAP_TEST_POINTS;
        
        // Set TPS position
        MAP_Emulator_SetTPS_Percent(tps_percent);
        MAP_Emulator_Update();
        HAL_Delay(MAP_TEST_DELAY_MS);
        
        // Read results
        float pressure_kpa = MAP_Emulator_GetPressure_kPa();
        float pressure_psi = MAP_Emulator_GetPressure_PSI();
        float voltage = MAP_Emulator_GetVoltage();
        uint16_t dac_value = MAP_Emulator_GetDACOutput();
        const char* condition = MAP_Emulator_GetPressureDescription();
        
        printf("%.0f\t%.1f\t\t%.1f\t\t%.2f\t\t%u\t%s\n", 
               tps_percent, pressure_kpa, pressure_psi, voltage, dac_value, condition);
        
        // Validate pressure is within expected range
        MAP_TEST_ASSERT_RANGE_FLOAT(pressure_kpa, MAP_IDLE_VACUUM_KPA, MAP_ATMOSPHERIC_PRESSURE_KPA, 
                                    "Pressure within valid range");
        
        // Validate voltage is within expected range
        MAP_TEST_ASSERT_RANGE_FLOAT(voltage, MAP_MIN_VOLTAGE, MAP_MAX_VOLTAGE, 
                                    "Voltage within valid range");
    }
    
    printf("\nPressure Sweep Test Complete\n");
    return results;
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Test MAP emulator initialization
  * @retval Test results
  */
static TestResults_t Test_MAP_Initialization(void)
{
    TestResults_t results = {0};
    
    // Test initialization state
    MAP_TEST_ASSERT(MAP_Emulator_IsEnabled(), "MAP emulator is enabled after init");
    
    // Test initial values are reasonable
    float initial_pressure = MAP_Emulator_GetPressure_kPa();
    MAP_TEST_ASSERT_RANGE_FLOAT(initial_pressure, MAP_IDLE_VACUUM_KPA, MAP_ATMOSPHERIC_PRESSURE_KPA,
                                "Initial pressure within valid range");
    
    float initial_voltage = MAP_Emulator_GetVoltage();
    MAP_TEST_ASSERT_RANGE_FLOAT(initial_voltage, MAP_MIN_VOLTAGE, MAP_MAX_VOLTAGE,
                                "Initial voltage within valid range");
    
    uint16_t initial_dac = MAP_Emulator_GetDACOutput();
    MAP_TEST_ASSERT(initial_dac < MAP_DAC_RESOLUTION, "Initial DAC value within range");
    
    return results;
}

/**
  * @brief  Test MAP pressure calculation functions
  * @retval Test results
  */
static TestResults_t Test_MAP_PressureCalculations(void)
{
    TestResults_t results = {0};
    
    // Test idle conditions (0% TPS)
    float idle_pressure = MAP_Emulator_CalculatePressureFromTPS(0.0f);
    MAP_TEST_ASSERT_NEAR_FLOAT(idle_pressure, MAP_IDLE_VACUUM_KPA, MAP_TEST_TOLERANCE_KPA,
                               "Idle pressure calculation (0% TPS)");
    
    // Test WOT conditions (100% TPS)
    float wot_pressure = MAP_Emulator_CalculatePressureFromTPS(100.0f);
    MAP_TEST_ASSERT_NEAR_FLOAT(wot_pressure, MAP_ATMOSPHERIC_PRESSURE_KPA, MAP_TEST_TOLERANCE_KPA,
                               "WOT pressure calculation (100% TPS)");
    
    // Test mid-range (50% TPS)
    float mid_pressure = MAP_Emulator_CalculatePressureFromTPS(50.0f);
    MAP_TEST_ASSERT_RANGE_FLOAT(mid_pressure, idle_pressure, wot_pressure,
                                "Mid-range pressure between idle and WOT");
    
    // Test pressure conversion kPa to PSI
    float pressure_kpa = 75.0f;
    float pressure_psi_calculated = MAP_KPA_TO_PSI(pressure_kpa);
    float expected_psi = pressure_kpa * 0.145038f;
    MAP_TEST_ASSERT_NEAR_FLOAT(pressure_psi_calculated, expected_psi, 0.1f,
                               "kPa to PSI conversion accuracy");
    
    return results;
}

/**
  * @brief  Test TPS response and pressure mapping
  * @retval Test results
  */
static TestResults_t Test_MAP_TPSResponse(void)
{
    TestResults_t results = {0};
    
    // Test various TPS positions
    float tps_positions[] = {0.0f, 25.0f, 50.0f, 75.0f, 100.0f};
    int num_positions = sizeof(tps_positions) / sizeof(tps_positions[0]);
    
    for (int i = 0; i < num_positions; i++) {
        float tps = tps_positions[i];
        
        MAP_Emulator_SetTPS_Percent(tps);
        MAP_Emulator_Update();
        
        float current_tps = MAP_Emulator_GetTPS_Percent();
        MAP_TEST_ASSERT_NEAR_FLOAT(current_tps, tps, MAP_TEST_TOLERANCE_PERCENT,
                                   "TPS setting accuracy");
        
        float pressure = MAP_Emulator_GetPressure_kPa();
        MAP_TEST_ASSERT_RANGE_FLOAT(pressure, MAP_IDLE_VACUUM_KPA, MAP_ATMOSPHERIC_PRESSURE_KPA,
                                    "Pressure within range for TPS setting");
    }
    
    // Test TPS boundary clamping
    MAP_Emulator_SetTPS_Percent(-10.0f);  // Below minimum
    MAP_Emulator_Update();
    float clamped_low = MAP_Emulator_GetTPS_Percent();
    MAP_TEST_ASSERT_NEAR_FLOAT(clamped_low, MAP_TPS_MIN_PERCENT, MAP_TEST_TOLERANCE_PERCENT,
                               "TPS clamping at lower bound");
    
    MAP_Emulator_SetTPS_Percent(110.0f);  // Above maximum
    MAP_Emulator_Update();
    float clamped_high = MAP_Emulator_GetTPS_Percent();
    MAP_TEST_ASSERT_NEAR_FLOAT(clamped_high, MAP_TPS_MAX_PERCENT, MAP_TEST_TOLERANCE_PERCENT,
                               "TPS clamping at upper bound");
    
    return results;
}

/**
  * @brief  Test voltage conversion functions
  * @retval Test results
  */
static TestResults_t Test_MAP_VoltageConversion(void)
{
    TestResults_t results = {0};
    
    // Test pressure to voltage conversion
    float test_pressure = MAP_IDLE_VACUUM_KPA;
    float voltage_low = MAP_Emulator_PressureToVoltage(test_pressure);
    MAP_TEST_ASSERT_NEAR_FLOAT(voltage_low, MAP_MIN_VOLTAGE, MAP_TEST_TOLERANCE_VOLTAGE,
                               "Low pressure to voltage conversion");
    
    test_pressure = MAP_ATMOSPHERIC_PRESSURE_KPA;
    float voltage_high = MAP_Emulator_PressureToVoltage(test_pressure);
    MAP_TEST_ASSERT_NEAR_FLOAT(voltage_high, MAP_MAX_VOLTAGE, MAP_TEST_TOLERANCE_VOLTAGE,
                               "High pressure to voltage conversion");
    
    // Test voltage to pressure conversion (roundtrip)
    float test_voltage = 1.65f;  // Mid-range voltage
    float pressure_from_voltage = MAP_Emulator_VoltageToPressure(test_voltage);
    float voltage_roundtrip = MAP_Emulator_PressureToVoltage(pressure_from_voltage);
    MAP_TEST_ASSERT_NEAR_FLOAT(voltage_roundtrip, test_voltage, MAP_TEST_TOLERANCE_VOLTAGE,
                               "Voltage to pressure roundtrip conversion");
    
    // Test linear relationship
    float voltage_25 = MAP_Emulator_PressureToVoltage(MAP_IDLE_VACUUM_KPA + 0.25f * (MAP_ATMOSPHERIC_PRESSURE_KPA - MAP_IDLE_VACUUM_KPA));
    float voltage_75 = MAP_Emulator_PressureToVoltage(MAP_IDLE_VACUUM_KPA + 0.75f * (MAP_ATMOSPHERIC_PRESSURE_KPA - MAP_IDLE_VACUUM_KPA));
    float expected_25 = MAP_MIN_VOLTAGE + 0.25f * (MAP_MAX_VOLTAGE - MAP_MIN_VOLTAGE);
    float expected_75 = MAP_MIN_VOLTAGE + 0.75f * (MAP_MAX_VOLTAGE - MAP_MIN_VOLTAGE);
    
    MAP_TEST_ASSERT_NEAR_FLOAT(voltage_25, expected_25, MAP_TEST_TOLERANCE_VOLTAGE,
                               "Linear voltage response at 25% pressure");
    MAP_TEST_ASSERT_NEAR_FLOAT(voltage_75, expected_75, MAP_TEST_TOLERANCE_VOLTAGE,
                               "Linear voltage response at 75% pressure");
    
    return results;
}

/**
  * @brief  Test DAC output functions
  * @retval Test results
  */
static TestResults_t Test_MAP_DACOutput(void)
{
    TestResults_t results = {0};
    
    // Test DAC conversion from pressure
    uint16_t dac_min = MAP_Emulator_PressureToDAC(MAP_IDLE_VACUUM_KPA);
    MAP_TEST_ASSERT(dac_min < MAP_DAC_RESOLUTION, "DAC minimum value within range");
    MAP_TEST_ASSERT(dac_min < MAP_DAC_RESOLUTION / 4, "DAC minimum value reasonably low");
    
    uint16_t dac_max = MAP_Emulator_PressureToDAC(MAP_ATMOSPHERIC_PRESSURE_KPA);
    MAP_TEST_ASSERT(dac_max < MAP_DAC_RESOLUTION, "DAC maximum value within range");
    MAP_TEST_ASSERT(dac_max > MAP_DAC_RESOLUTION * 3 / 4, "DAC maximum value reasonably high");
    
    // Test DAC to pressure roundtrip
    uint16_t test_dac = 2048;  // Mid-range DAC value
    float pressure_from_dac = MAP_Emulator_DACToPressure(test_dac);
    uint16_t dac_roundtrip = MAP_Emulator_PressureToDAC(pressure_from_dac);
    MAP_TEST_ASSERT(abs((int)dac_roundtrip - (int)test_dac) <= 2, "DAC roundtrip conversion accuracy");
    
    // Test DAC monotonicity (increasing pressure should increase DAC)
    MAP_TEST_ASSERT(dac_max > dac_min, "DAC output increases with pressure");
    
    return results;
}

/**
  * @brief  Test boundary conditions and error handling
  * @retval Test results
  */
static TestResults_t Test_MAP_BoundaryConditions(void)
{
    TestResults_t results = {0};
    
    // Test enable/disable functionality
    MAP_Emulator_SetEnabled(false);
    MAP_TEST_ASSERT(!MAP_Emulator_IsEnabled(), "MAP emulator disable functionality");
    
    MAP_Emulator_SetEnabled(true);
    MAP_TEST_ASSERT(MAP_Emulator_IsEnabled(), "MAP emulator enable functionality");
    
    // Test extreme pressure values (should be clamped)
    MAP_Emulator_SetPressure_kPa(0.0f);  // Below minimum
    float pressure_low = MAP_Emulator_GetPressure_kPa();
    MAP_TEST_ASSERT(pressure_low >= MAP_IDLE_VACUUM_KPA, "Pressure clamping at lower bound");
    
    MAP_Emulator_SetPressure_kPa(200.0f);  // Above maximum
    float pressure_high = MAP_Emulator_GetPressure_kPa();
    MAP_TEST_ASSERT(pressure_high <= MAP_ATMOSPHERIC_PRESSURE_KPA, "Pressure clamping at upper bound");
    
    // Test extreme TPS values (already tested in TPS response, but verify here)
    MAP_Emulator_UpdateFromTPS(0);     // Minimum ADC
    MAP_Emulator_Update();
    float tps_min = MAP_Emulator_GetTPS_Percent();
    MAP_TEST_ASSERT_NEAR_FLOAT(tps_min, 0.0f, MAP_TEST_TOLERANCE_PERCENT, "Minimum TPS from ADC");
    
    MAP_Emulator_UpdateFromTPS(4095);  // Maximum ADC
    MAP_Emulator_Update();
    float tps_max = MAP_Emulator_GetTPS_Percent();
    MAP_TEST_ASSERT_NEAR_FLOAT(tps_max, 100.0f, MAP_TEST_TOLERANCE_PERCENT, "Maximum TPS from ADC");
    
    return results;
}

/**
  * @brief  Test coordinated operation with VR sensor
  * @retval Test results
  */
static TestResults_t Test_MAP_CoordinatedOperation(void)
{
    TestResults_t results = {0};
    
    // Test that MAP sensor responds to same ADC values as VR sensor
    uint16_t test_adc_values[] = {0, 1000, 2000, 3000, 4095};
    int num_values = sizeof(test_adc_values) / sizeof(test_adc_values[0]);
    
    for (int i = 0; i < num_values; i++) {
        uint16_t adc_value = test_adc_values[i];
        
        // Update MAP sensor with ADC value
        MAP_Emulator_UpdateFromTPS(adc_value);
        MAP_Emulator_Update();
        
        // Calculate expected values
        float expected_tps = MAP_ADC_TO_TPS_PERCENT(adc_value);
        uint16_t expected_rpm = MAP_ADC_TO_RPM(adc_value);
        
        // Verify calculations
        float actual_tps = MAP_Emulator_GetTPS_Percent();
        uint16_t actual_rpm = MAP_Emulator_GetRPM();
        
        MAP_TEST_ASSERT_NEAR_FLOAT(actual_tps, expected_tps, MAP_TEST_TOLERANCE_PERCENT,
                                   "TPS calculation matches ADC input");
        
        // RPM should be proportional to ADC value
        float rpm_ratio = (float)actual_rpm / MAP_MAX_RPM;
        float adc_ratio = (float)adc_value / 4095.0f;
        MAP_TEST_ASSERT_NEAR_FLOAT(rpm_ratio, adc_ratio, 0.05f, "RPM proportional to ADC");
        
        // Pressure should be realistic for the TPS position
        float pressure = MAP_Emulator_GetPressure_kPa();
        if (actual_tps < 10.0f) {
            // Low TPS should give high vacuum (low pressure)
            MAP_TEST_ASSERT(pressure < (MAP_IDLE_VACUUM_KPA + 10.0f), "Low TPS gives high vacuum");
        } else if (actual_tps > 90.0f) {
            // High TPS should give low vacuum (high pressure)
            MAP_TEST_ASSERT(pressure > (MAP_ATMOSPHERIC_PRESSURE_KPA - 10.0f), "High TPS gives low vacuum");
        }
    }
    
    return results;
}

/**
  * @brief  Print current MAP sensor test status
  * @retval None
  */
static void Print_MAP_Test_Status(void)
{
    printf("\n--- MAP Sensor Current Status ---\n");
    printf("Enabled: %s\n", MAP_Emulator_IsEnabled() ? "Yes" : "No");
    printf("TPS: %.1f%%\n", MAP_Emulator_GetTPS_Percent());
    printf("RPM: %u\n", MAP_Emulator_GetRPM());
    printf("Pressure: %.1f kPa (%.1f PSI)\n", 
           MAP_Emulator_GetPressure_kPa(), MAP_Emulator_GetPressure_PSI());
    printf("Voltage: %.2f V\n", MAP_Emulator_GetVoltage());
    printf("DAC: %u\n", MAP_Emulator_GetDACOutput());
    printf("Condition: %s\n", MAP_Emulator_GetPressureDescription());
    printf("------------------------------\n");
}