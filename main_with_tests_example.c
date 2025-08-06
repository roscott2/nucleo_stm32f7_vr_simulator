/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main_with_tests.c
  * @brief          : Example main.c with integrated VR emulator tests
  ******************************************************************************
  * @attention
  *
  * Example showing how to integrate VR sensor emulator tests with main application
  * This is a reference implementation - copy relevant parts to your main.c
  * 
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "vr_sensor_emulator.h"
#include "test_integration.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TEST_MODE_ENABLED           1       // Set to 1 to enable test mode
#define AUTO_RUN_TESTS              0       // Set to 1 to auto-run tests on startup
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static bool button_pressed = false;
static uint32_t button_press_time = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void Handle_Button_Press(void);
static void Run_Test_Menu(void);
static void Print_Menu(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
  * @brief  The application entry point with test integration.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  // ... other peripheral initializations

  /* USER CODE BEGIN 2 */
  
  // Initialize VR sensor emulator
  VR_Emulator_Init();
  
#if TEST_MODE_ENABLED
  // Initialize test environment
  VR_Test_Init();
  
  printf("\n=== NUCLEO-STM32F7 VR Sensor Emulator ===\n");
  printf("Press User Button (Blue) to access test menu\n");
  printf("Test Mode: ENABLED\n\n");
  
#if AUTO_RUN_TESTS
  printf("Auto-running basic tests...\n");
  TestResults_t results = VR_Test_RunBasic();
  
  if (results.failed_tests == 0) {
    printf("✓ All tests passed! System ready.\n");
  } else {
    printf("✗ Some tests failed. Check system configuration.\n");
  }
#endif

#else
  printf("\n=== NUCLEO-STM32F7 VR Sensor Emulator ===\n");
  printf("Test Mode: DISABLED\n");
  printf("Normal operation mode\n\n");
#endif

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    
    // Update VR sensor emulator (read potentiometer, update RPM)
    VR_Emulator_Update();
    
    // Handle button press for test menu
#if TEST_MODE_ENABLED
    Handle_Button_Press();
#endif
    
    // Small delay to prevent overwhelming the system
    HAL_Delay(10);
    
    // Toggle LED to show system is alive
    HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
  }
  /* USER CODE END 3 */
}

/**
  * @brief  Handle button press for test menu access
  * @retval None
  */
static void Handle_Button_Press(void)
{
    // Check if button is pressed (active low)
    if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET) {
        if (!button_pressed) {
            button_pressed = true;
            button_press_time = HAL_GetTick();
        }
    } else {
        if (button_pressed) {
            // Button released - check press duration
            uint32_t press_duration = HAL_GetTick() - button_press_time;
            
            if (press_duration > 50 && press_duration < 3000) { // 50ms to 3s press
                Run_Test_Menu();
            }
            
            button_pressed = false;
        }
    }
}

/**
  * @brief  Run interactive test menu
  * @retval None
  */
static void Run_Test_Menu(void)
{
    printf("\n" );
    printf("┌─────────────────────────────────────┐\n");
    printf("│          VR Emulator Tests          │\n");
    printf("└─────────────────────────────────────┘\n");
    
    Print_Menu();
    
    // In a real implementation, you might want to:
    // 1. Wait for UART input to select test
    // 2. Use button combinations
    // 3. Or automatically run a default test
    
    // For this example, we'll run basic tests
    printf("Running basic functionality tests...\n\n");
    
    TestResults_t results = VR_Test_RunBasic();
    
    if (results.failed_tests == 0) {
        printf("✓ All tests completed successfully!\n");
        // Flash green LED pattern
        for (int i = 0; i < 3; i++) {
            HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
            HAL_Delay(200);
            HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
            HAL_Delay(200);
        }
    } else {
        printf("✗ %d tests failed!\n", results.failed_tests);
        // Flash red LED pattern
        for (int i = 0; i < 5; i++) {
            HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
            HAL_Delay(100);
            HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
            HAL_Delay(100);
        }
    }
    
    printf("Press button again for more tests.\n\n");
}

/**
  * @brief  Print test menu options
  * @retval None
  */
static void Print_Menu(void)
{
    printf("Available Tests:\n");
    printf("1. Basic Functionality Tests\n");
    printf("2. RPM Range Tests (0-%d RPM)\n", MAX_RPM);
    printf("3. ADC Conversion Tests\n");
    printf("4. Timing Validation Tests\n");
    printf("5. Demo Mode\n");
    printf("6. Comprehensive Test Suite\n");
    printf("\n");
}

/**
  * @brief  GPIO Initialization Function (example)
  * @param  None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD2_Pin|LD3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD1_Pin LD2_Pin LD3_Pin */
  GPIO_InitStruct.Pin = LD1_Pin|LD2_Pin|LD3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/* 
 * Additional functions to implement in test_vr_emulator.c:
 * - VR_Emulator_TestRPMRange()
 * - VR_Emulator_TestADCConversion()  
 * - VR_Emulator_TestBoundaryConditions()
 * - VR_Emulator_TestPerformance()
 * - VR_Emulator_ValidateToothTiming()
 * 
 * And utility functions:
 * - VR_Test_CalculateToothFrequency()
 * - VR_Test_CalculateToothPeriod()
 * - VR_Test_ADCToRPM()
 * - VR_Test_RPMToADC()
 * - VR_Test_IsWithinTolerance()
 * - VR_Test_GetTimestamp_us()
 */

/* USER CODE END 0 */
