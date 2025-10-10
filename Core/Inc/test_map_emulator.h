/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : test_map_emulator.h
  * @brief          : Header for MAP sensor emulator unit tests
  ******************************************************************************
  * @attention
  *
  * Unit tests for MAP Sensor Emulator for NUCLEO-STM32F7
  * Tests MAP sensor pressure calculations, TPS response, and DAC output
  * 
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TEST_MAP_EMULATOR_H
#define __TEST_MAP_EMULATOR_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "test_vr_emulator.h"  // For TestResults_t type
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/
/* Test results structure is already defined in test_vr_emulator.h */

/* Exported constants --------------------------------------------------------*/
/* Test configuration for MAP sensor */
#define MAP_TEST_ENABLED            1       // Set to 1 to enable MAP testing
#define MAP_TEST_VERBOSE            1       // Set to 1 for detailed output
#define MAP_TEST_BENCHMARK          0       // Set to 1 to run performance tests

/* Exported macro ------------------------------------------------------------*/
/* MAP Test control macros */
#define MAP_TEST_RUN()              MAP_Emulator_RunTests()
#define MAP_TEST_SWEEP()            MAP_Emulator_TestPressureSweep()

/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief  Run all MAP emulator tests
  * @retval Test results structure with pass/fail counts
  */
TestResults_t MAP_Emulator_RunTests(void);

/**
  * @brief  Test MAP sensor pressure sweep across TPS range
  * @retval Test results structure
  */
TestResults_t MAP_Emulator_TestPressureSweep(void);

/**
  * @brief  Run MAP sensor basic functionality tests
  * @retval Test results structure
  */
TestResults_t MAP_Emulator_TestBasicFunctionality(void);

/**
  * @brief  Run MAP sensor pressure calculation tests
  * @retval Test results structure
  */
TestResults_t MAP_Emulator_TestPressureCalculations(void);

/**
  * @brief  Run MAP sensor TPS response tests
  * @retval Test results structure
  */
TestResults_t MAP_Emulator_TestTPSResponse(void);

/**
  * @brief  Run MAP sensor voltage conversion tests
  * @retval Test results structure
  */
TestResults_t MAP_Emulator_TestVoltageConversion(void);

/**
  * @brief  Run MAP sensor DAC output tests
  * @retval Test results structure
  */
TestResults_t MAP_Emulator_TestDACOutput(void);

/**
  * @brief  Run MAP sensor boundary condition tests
  * @retval Test results structure
  */
TestResults_t MAP_Emulator_TestBoundaryConditions(void);

/**
  * @brief  Run MAP sensor coordinated operation tests
  * @retval Test results structure
  */
TestResults_t MAP_Emulator_TestCoordinatedOperation(void);

/**
  * @brief  Test MAP sensor performance characteristics
  * @retval Test results structure
  */
TestResults_t MAP_Emulator_TestPerformance(void);

/**
  * @brief  Print MAP sensor test configuration and status
  * @retval None
  */
void MAP_Emulator_PrintTestInfo(void);

#ifdef __cplusplus
}
#endif

#endif /* __TEST_MAP_EMULATOR_H */