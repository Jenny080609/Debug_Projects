/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app.h"
#include "key.h"
#include "led.h"
#include "oled.h"
#include "uart.h"
#include "line.h"
#include "motor.h"
#include "beep.h"
#include <stdio.h>
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
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
  * @brief printf 重定向：所有 printf 输出通过 USART1 发送
  * @note  Keil 需勾选 Use MicroLIB
  */
int fputc(int ch, FILE *f)
{
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}

/* ================= 巡线全局状态 ================= */
uint8_t  line_running     = 0;   /* 0=停车，1=跑圈中 */
uint32_t line_start_tick  = 0;   /* 发车时刻（HAL_GetTick） */
uint32_t line_sample_tick = 0;   /* 10ms 采样计时 */
uint32_t line_ui_tick     = 0;   /* 100ms 显示计时 */
uint32_t line_lost_tick   = 0;   /* 脱线（全白）起始时刻，0 表示在线 */
float    last_time        = 0.0f;/* 本圈用时（秒，保留 1 位小数） */
uint8_t  line_black_cnt   = 0;   /* 全黑连续采样次数（消抖用） */

/* 上一次动作：脱线（全白）时沿用，给小车"找线"的机会 */
#define LINE_ACT_FORWARD  0
#define LINE_ACT_LEFT     1
#define LINE_ACT_RIGHT    2
uint8_t  last_error_action = LINE_ACT_FORWARD;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  uint8_t key;                  /* 本周期扫描到的按键值，0 表示没按键 */
  uint32_t led_ui_tick = 0;     /* LED 状态界面上次刷新时间 */
  char line_spaced[12];         /* "1 1 0 1 1" 格式的传感器状态 */
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
  MX_USART1_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  OLED_Init();                  /* OLED 初始化：软件 I2C，PB6=SCL，PB7=SDA */
  Refresh_Display();            /* 上电先显示主菜单 */
  UART_Start();                 /* 启动第一次串口接收中断 */
	
	LED_Force_Off();
  Motor_Init();                 /* 电机方向引脚 + TIM3 双路 PWM */
  Beep_Init();                  /* 蜂鸣器 PB8 */
  Line_Init();                  /* 5 路传感器 + 关闭 JTAG（PB3/PB4）*/
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		
    key = Key_Scan();                      /* 1. 扫描按键（内部 20ms 消抖）*/

    if (key != 0)                          /* 2. 有按键才处理 */
    {
      if (sys_state == MENU_MAIN)
      {
        /* ---------- 主菜单：移动光标或进入子菜单 ---------- */
        if (key == 1)                       /* 按键1：光标上移（0 的上一个是 3）*/
        {
          menu_cursor = (menu_cursor + MENU_ITEMS - 1) % MENU_ITEMS;
          Refresh_Display();
        }
        else if (key == 2)                  /* 按键2：光标下移（3 的下一个是 0）*/
        {
          menu_cursor = (menu_cursor + 1) % MENU_ITEMS;
          Refresh_Display();
        }
        else if (key == 3)                  /* 按键3：进入光标选中的子菜单 */
        {
          switch (menu_cursor)
          {
            case 0: sys_state = MENU_LED;  break;
            case 1: sys_state = MENU_INFO; break;
            case 2: Motor_Stop();          /* 进巡线界面先确保电机停转 */
                    sys_state = MODE_LINE; break;
            case 3: sys_state = MENU_EXT;  break;
            default: sys_state = MENU_MAIN; break;
          }
          Refresh_Display();        /* LED 子菜单的 Refresh_Display 内部会显示状态 */
        }
      }
      else if (sys_state == MENU_LED)
      {
        /* ---------- LED 子菜单 ---------- */
        if (key == 1)
        {
          LED_Toggle_All();                 /* 常亮 / 常灭切换，并打断闪烁 */
        }
        else if (key == 2)
        {
          LED_Start_Blink();                /* 开始交替闪烁，重新计时 30 秒 */
        }
        else if (key == 4)
        {
          LED_Force_Off();                  /* 关灯、清除闪烁状态 */
          sys_state = MENU_MAIN;
          Refresh_Display();
        }
      }
      else if (sys_state == MODE_LINE)
      {
        /* ---------- 巡线子菜单 ---------- */
        if (key == 1 && line_running == 0)      /* 按键1：开始巡线 */
        {
          Beep_Once();                          /* 先响一声，约 100ms */
          line_running      = 1;
          line_start_tick   = HAL_GetTick();    /* 发车瞬间开始计时 */
          line_sample_tick  = HAL_GetTick();
          line_ui_tick      = 0;                /* 立即刷新界面 */
          line_black_cnt    = 0;
          line_lost_tick    = 0;                /* 清除脱线状态 */
          last_error_action = LINE_ACT_FORWARD; /* 起步默认直行 */
          last_time         = 0.0f;             /* 清空上一圈成绩 */
          printf("LINE START\r\n");
        }
        else if (key == 4)                      /* 按键4：强制停车并退出 */
        {
          Motor_Stop();

          if (line_running)                     /* 中途手动停止，记录已用时 */
          {
            last_time = (HAL_GetTick() - line_start_tick) / 1000.0f;
            printf("LINE ABORT: %.1f s\r\n", last_time);
          }

          line_running = 0;
          sys_state = MENU_MAIN;
          Refresh_Display();
        }
      }
      else
      {
        /* ---------- 其它子菜单：只响应返回键 ---------- */
        if (key == 4)
        {
          sys_state = MENU_MAIN;
          Refresh_Display();
        }
      }
    }

    /* 3. 串口任务："111" 修改 var_a，"GET A" 返回 var_a */
    UART_Task();

    /* 4. var_a 被修改且正在信息界面时，刷新屏幕 */
    if (uart_refresh_flag)
    {
      uart_refresh_flag = 0;
      if (sys_state == MENU_INFO)
      {
        Refresh_Display();
      }
    }

    /* 5. LED 闪烁计时任务：每个循环都调用，内部非阻塞 */
    LED_Task();

    /* 6. LED 子菜单：按键后立即刷新，闪烁期间每 200ms 刷新一次 */
    if (sys_state == MENU_LED &&
        (key != 0 || (HAL_GetTick() - led_ui_tick) >= 200))
    {
      led_ui_tick = HAL_GetTick();
      Refresh_LED_Status();
    }
		
    /* 7. 巡线任务：10ms 采样控制，100ms 刷新显示 */
    if (sys_state == MODE_LINE)
    {
      /* --- 10ms：读传感器并控制电机 --- */
      if (line_running && (HAL_GetTick() - line_sample_tick) >= 10)
      {
        const char *s;
        uint8_t left, right, mid;

        line_sample_tick = HAL_GetTick();
        s = Line_ReadSensors();

        /* --- 判圈：连续 5 次全黑（约 50ms）+ 已跑 ≥3 秒 --- */
        if (Line_IsAllBlack())
        {
          if (line_black_cnt < 5) line_black_cnt++;
        }
        else
        {
          line_black_cnt = 0;
        }

        if (line_black_cnt >= 5 &&
            (HAL_GetTick() - line_start_tick) >= 3000)
        {
          Motor_Stop();                              /* 自动停车 */
          last_time = (HAL_GetTick() - line_start_tick) / 1000.0f;
          line_running = 0;
          line_black_cnt = 0;
          line_lost_tick = 0;
          line_ui_tick = 0;                          /* 立即刷新最终画面 */
          printf("LAP DONE: %.1f s\r\n", last_time);
        }
        else
        {
          left  = (s[0] == '0' || s[1] == '0');
          right = (s[3] == '0' || s[4] == '0');
          mid   = (s[2] == '0');

          if (left && right)                    /* 宽线 / 两侧都压：居中直行 */
          {
            Motor_Forward();
            last_error_action = LINE_ACT_FORWARD;
            line_lost_tick = 0;
          }
          else if (left)                        /* 左侧压线：急左转 */
          {
            Motor_TurnLeft();
            last_error_action = LINE_ACT_LEFT;
            line_lost_tick = 0;
          }
          else if (right)                       /* 右侧压线：急右转 */
          {
            Motor_TurnRight();
            last_error_action = LINE_ACT_RIGHT;
            line_lost_tick = 0;
          }
          else if (mid)                         /* 中间压线：直行 */
          {
            Motor_Forward();
            last_error_action = LINE_ACT_FORWARD;
            line_lost_tick = 0;
          }
          else                                  /* 全白：脱线找线，绝不停车 */
          {
            if (line_lost_tick == 0) line_lost_tick = HAL_GetTick();

            if ((HAL_GetTick() - line_lost_tick) <= 300)
            {
              /* 300ms 内：保持上一次的转向继续找线 */
              if (last_error_action == LINE_ACT_LEFT)       Motor_TurnLeft();
              else if (last_error_action == LINE_ACT_RIGHT) Motor_TurnRight();
              else                                          Motor_Forward();
            }
            else
            {
              /* 超过 300ms 仍无线：改直行继续找，仍然不停车 */
              Motor_Forward();
              last_error_action = LINE_ACT_FORWARD;
            }
          }
        }
      }

      /* --- 100ms：刷新 OLED 和串口 --- */
      if ((HAL_GetTick() - line_ui_tick) >= 100)
      {
        const char *s;
        char buf[24];

        line_ui_tick = HAL_GetTick();
        s = Line_ReadSensors();

        sprintf(line_spaced, "%c %c %c %c %c",
                s[0], s[1], s[2], s[3], s[4]);

        OLED_ShowString(0, 0, "==== LINE ====");

        if (line_running)
        {
          /* 跑圈中：实时累加显示已用时间 */
          sprintf(buf, "Time: %.1f s",
                  (HAL_GetTick() - line_start_tick) / 1000.0f);
          OLED_ShowString(0, 7, "4:Stop");
        }
        else if (last_time > 0.0f)
        {
          /* 跑完一圈：固定显示本圈成绩（6x8 字库无中文，用 Last 代替“本圈用时”） */
          sprintf(buf, "Last: %.1f s", last_time);
          OLED_ShowString(0, 7, "1:Start 4:Back");
        }
        else
        {
          sprintf(buf, "1:Start 4:Back     ");
          OLED_ShowString(0, 7, "4:Back");
        }
        OLED_ShowString(0, 1, buf);
        OLED_ShowString(0, 2, line_spaced);

        printf("Sensor: %s  %s\r\n", line_spaced, line_running ? "RUN" : "STOP");
      }
    }

    HAL_Delay(5);
		
  /* USER CODE END 3 */
}
}
/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 71;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 99;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4|GPIO_PIN_5, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /* PB8 蜂鸣器为低电平触发：先写高电平，确保一上电绝对不响 */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_SET);

  /*Configure GPIO pins : PA0 PA1 PA2 PA3 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA4 PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB3 PB4
                           PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_3|GPIO_PIN_4
                          |GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB12 PB13 PB14 PB15 电机方向控制 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB8 蜂鸣器，单独初始化，推挽输出，默认高电平（不响） */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);   /* 配置完再确认一次高电平 */

  /*Configure GPIO pins : PB6 PB7 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* PA5：LED2，推挽输出，初始高电平（灭） */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);

  /* PA0~PA3：四个按键，输入 + 内部上拉（按下为低电平） */
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
