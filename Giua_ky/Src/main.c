#include <stdint.h>
#include <string.h>
#include <stm32f401re_gpio.h>
#include <stm32f401re_rcc.h>
#include <stdio.h>
#include <stm32f401re_i2c.h>
#include <stm32f401re_spi.h>
#include <timer.h>
#include <temhumsensor.h>
#include <Ucglib.h>
#include <kalman_filter.h>
#include <lightsensor.h>
#include <stm32f401re_adc.h>
#include <stm32f401re_tim.h>
/***********************************************************************************/
#define CYCLE_SEND_DATA_1 1000 // ms

#define CYCLE_SEND_DATA_2 2000 // ms

#define ADC_PORT				    		GPIOC
#define ADC_PIN             			    GPIO_Pin_5
#define Tim_Period      			  		8399

/*Defined SPI **********************************************************************/

#define SPI1_CS_PORT           				 GPIOB
#define SPI1_CS_PIN            				 GPIO_Pin_6

#define SPI1_RST_PORT          				 GPIOC
#define SPI1_RST_PIN           				 GPIO_Pin_7

#define SPI1_MOSI_PORT         				 GPIOA
#define SPI1_MOSI_PIN          				 GPIO_Pin_7

#define SPI1_SCK_PORT          				 GPIOA
#define SPI1_SCK_PIN           				 GPIO_Pin_5

#define SPI1_RS_PORT           				 GPIOA
#define SPI1_RS_PIN            				 GPIO_Pin_9

#define SPI1_ENABLE_PORT       				 GPIOB
#define SPI1_ENABLE_PIN        				 GPIO_Pin_10

#define SPI1_MODE_PORT          			 GPIOA
#define SPI1_MODE_PIN           			 GPIO_Pin_8

/*Defined I2C ******************************************************************/

#define I2Cx_RCC							 RCC_APB1Periph_I2C1
#define I2Cx_SENSOR							 I2C1
#define I2C_GPIO_RCC		    			 RCC_AHB1Periph_GPIOB
#define I2C_GPIO							 GPIOB
#define I2C_PIN_SDA			    		 	 GPIO_Pin_9
#define I2C_PIN_SCL			    			 GPIO_Pin_8

/*Defined SENSOR ***************************************************************/

#define SI7020_ADDR                          0x40
#define CMDR_MEASURE_VALUE                   0xE0

// yeu cau 2
#define GPIO_PIN_SET 	1
#define GPIO_PIN_RESET 	0
#define GPIO_PIN_HIGH 	1
#define GPIO_PIN_LOW 	0

#define LEDREDA1_GPIO_PORT				GPIOA
#define LEDREDA1_GPIO_PIN				GPIO_Pin_1
#define LEDREDA1_PIN					1
#define LEDREDA1Control_SetClock		RCC_AHB1Periph_GPIOA

#define LEDREDB13_GPIO_PORT				GPIOB
#define LEDREDB13_GPIO_PIN				GPIO_Pin_13
#define LEDREDB13_PIN					13
#define LEDREDB13Control_SetClock		RCC_AHB1Periph_GPIOB

#define LEDREDA3_GPIO_PORT				GPIOA
#define LEDREDA3_GPIO_PIN				GPIO_Pin_3
#define LEDREDA3_PIN					3
#define LEDREDA3Control_SetClock		RCC_AHB1Periph_GPIOA

#define LEDREDA10_GPIO_PORT				GPIOA
#define LEDREDA10_GPIO_PIN				GPIO_Pin_10
#define LEDREDA10_PIN					10
#define LEDREDA10Control_SetClock		RCC_AHB1Periph_GPIOA

#define BUZZER_GPIO_PORT		GPIOC
#define BUZZER_GPIO_PIN			GPIO_Pin_9
#define BUZZER_PIN9				9
#define BUZZERControl_SetClock	RCC_AHB1Periph_GPIOC

#define BUTTON_GPIO_PORT		GPIOB
#define BUTTON_GPIO_PIN			GPIO_Pin_3
#define BUTTON_PIN3				3
#define BUTTONControl_SetClock	RCC_AHB1Periph_GPIOB

#define BUTTONA4_GPIO_PORT			GPIOA
#define BUTTONA4_GPIO_PIN			GPIO_Pin_4
#define BUTTONA4_PIN				4
#define BUTTONA4Control_SetClock	RCC_AHB1Periph_GPIOA

#define BUTTONB0_GPIO_PORT			GPIOB
#define BUTTONB0_GPIO_PIN			GPIO_Pin_0
#define BUTTONB0_PIN				0
#define BUTTONB0Control_SetClock	RCC_AHB1Periph_GPIOB

/*Privated Function ************************************************************/
static void AppCommon();
static void SPI1_Init(void);

static void I2C_Init_temphumi(void);
void I2C_start(void);
void I2C_address_direction(uint8_t address, uint8_t direction);
void I2C_transmit(uint8_t byte);
void I2C_stop(void);
uint8_t I2C_receive_nack(void);
uint8_t I2C_receive_ack(void);
/* function update--------------------------------------------------------------*/
static void AppInitCommon(void);
static void LightSensor_AdcInit(void);
static void LedControl_TimerOCInit(void);
uint16_t LightSensor_AdcPollingRead(void);
uint16_t Kanman_Light(uint16_t lightLevel);
static void LedControl_TimerOCSetPwm(uint32_t Compare);
static void ABL_Process(void);
static void TemHumSensor_readRegister(uint8_t address,
		uint8_t *pAddressRegister, uint8_t *pDataRegister, uint8_t Length_Data,
		uint16_t delay);
void processGetValueSensor(void);
uint32_t GetTemp_Sensor(void);
uint32_t GetHumi_Sensor(void);
void delay_ms(uint32_t ms);
uint32_t CalculatorTime(uint32_t dwTimeInit, uint32_t dwTimeCurrent);
void Scan_SensorLCD(void);
void Scan_TimeSensor(uint32_t byRepeats);
/* Private variables ---------------------------------------------------------*/
static ucg_t ucg;
uint32_t time_initial = 0;
uint32_t time_current, time_initial;
uint32_t idTimer = 0;
static char src1[20] = "";
static char src2[20] = "";
static char src3[20] = "";
static char src4[20] = "";
static char src5[20] = "";
static uint32_t time_total;
static uint8_t temperature, humidity;
static uint8_t temperature1, humidity1;
//static uint8_t temperature2, humidity2;
/* ----------------------------------modified 0604-----------------------------------------------*/
static uint16_t Kanman_light = 0;
uint16_t AdcValue = 0;
static uint16_t lightLevelAfterFilter = 0;
static void Led_Init_red1(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(LEDREDA1Control_SetClock, ENABLE);
	GPIO_InitStructure.GPIO_Pin = LEDREDA1_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(LEDREDA1_GPIO_PORT, &GPIO_InitStructure);
}

static void Led_Init_red2(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(LEDREDB13Control_SetClock, ENABLE);
	GPIO_InitStructure.GPIO_Pin = LEDREDB13_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(LEDREDB13_GPIO_PORT, &GPIO_InitStructure);
}

static void Led_Init_blue1(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(LEDREDA3Control_SetClock, ENABLE);
	GPIO_InitStructure.GPIO_Pin = LEDREDA3_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(LEDREDA3_GPIO_PORT, &GPIO_InitStructure);
}

static void Led_Init_blue2(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(LEDREDA10Control_SetClock, ENABLE);
	GPIO_InitStructure.GPIO_Pin = LEDREDA10_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(LEDREDA10_GPIO_PORT, &GPIO_InitStructure);
}

static void Button_Init(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(BUTTONControl_SetClock, ENABLE);
	GPIO_InitStructure.GPIO_Pin = BUTTON_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(BUTTON_GPIO_PORT, &GPIO_InitStructure);
}

static void Button_Init_b3(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(BUTTONA4Control_SetClock, ENABLE);
	GPIO_InitStructure.GPIO_Pin = BUTTONA4_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(BUTTONA4_GPIO_PORT, &GPIO_InitStructure);
}

static void Button_Init_b4(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(BUTTONB0Control_SetClock, ENABLE);
	GPIO_InitStructure.GPIO_Pin = BUTTONB0_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(BUTTONB0_GPIO_PORT, &GPIO_InitStructure);
}

static void Buzzer_Init(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(BUZZERControl_SetClock, ENABLE);
	GPIO_InitStructure.GPIO_Pin = BUZZER_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(BUZZER_GPIO_PORT, &GPIO_InitStructure);
}

static void LEDControl_SetStatus(GPIO_TypeDef *GPIOx, uint8_t GPIO_PIN,
		uint8_t Status) {
	if (Status == GPIO_PIN_SET) {
		GPIOx->BSRRL |= 1 << GPIO_PIN;
	}
	if (Status == GPIO_PIN_RESET) {
		GPIOx->BSRRH |= 1 << GPIO_PIN;
	}
}

static uint8_t ButtonRead_Status(GPIO_TypeDef *GPIOx, uint32_t GPIO_PIN) {
	uint32_t Read_Pin;
	Read_Pin = (GPIOx->IDR) >> GPIO_PIN;
	Read_Pin = Read_Pin & 0x01;

	return Read_Pin;
}

static void BuzzerControl_SetStatus(GPIO_TypeDef *GPIOx, uint8_t GPIO_PIN,
		uint8_t Status) {
	if (Status == GPIO_PIN_SET) {
		GPIOx->BSRRL |= 1 << GPIO_PIN;
	}
	if (Status == GPIO_PIN_RESET) {
		GPIOx->BSRRH |= 1 << GPIO_PIN;
	}
}

int main(void) {
	AppInitCommon();

	Led_Init_red1();
	Led_Init_red2();
	Led_Init_blue1();
	Led_Init_blue2();
	Buzzer_Init();
	Button_Init();
	Button_Init_b3();
	Button_Init_b4();
	int timeStart = 0; // Thời điểm bắt đầu nhấn nút
	int isButtonPressed = 0; // Cờ kiểm tra xem nút đã được nhấn hay chưa
	int cnt = 0;
	int cnt1 = 0;
	while (1) {
		processGetValueSensor();
		processTimerScheduler();
		ABL_Process();

//		while (ButtonRead_Status(BUTTON_GPIO_PORT, BUTTON_PIN3) == 0) {
//			BuzzerControl_SetStatus(BUZZER_GPIO_PORT, BUZZER_PIN9, 1);
//			LEDControl_SetStatus(LEDREDA1_GPIO_PORT, LEDREDA1_PIN, 1);
//			LEDControl_SetStatus(LEDREDB13_GPIO_PORT, LEDREDB13_PIN, 1);
//		}
//
//		BuzzerControl_SetStatus(BUZZER_GPIO_PORT, BUZZER_PIN9, 0);
//		LEDControl_SetStatus(LEDREDA1_GPIO_PORT, LEDREDA1_PIN, 0);
//		LEDControl_SetStatus(LEDREDB13_GPIO_PORT, LEDREDB13_PIN, 0);

		// yeu cau 2
		if (ButtonRead_Status(BUTTON_GPIO_PORT, BUTTON_PIN3) == 0) {
			for (int i = 0; i < 2; i++) {
				BuzzerControl_SetStatus(BUZZER_GPIO_PORT, BUZZER_PIN9, 1);
				LEDControl_SetStatus(LEDREDA1_GPIO_PORT, LEDREDA1_PIN, 1);
				LEDControl_SetStatus(LEDREDB13_GPIO_PORT, LEDREDB13_PIN, 1);
				delay_ms(500);
				BuzzerControl_SetStatus(BUZZER_GPIO_PORT, BUZZER_PIN9, 0);
				LEDControl_SetStatus(LEDREDA1_GPIO_PORT, LEDREDA1_PIN, 0);
				LEDControl_SetStatus(LEDREDB13_GPIO_PORT, LEDREDB13_PIN, 0);
				delay_ms(500);
			}
		}

		// yeu cau 3
//		if (ButtonRead_Status(BUTTONA4_GPIO_PORT, BUTTONA4_PIN) == 0) {
//			LEDControl_SetStatus(LEDREDA3_GPIO_PORT, LEDREDA3_PIN, 1);
//			LEDControl_SetStatus(LEDREDA10_GPIO_PORT, LEDREDA10_PIN, 1);
//			delay_ms(4000);
//		}
//		while(ButtonRead_Status(BUTTONA4_GPIO_PORT, BUTTONA4_PIN) == 0){
//			int time=GetMilSecTick(), timeCurrent=0;
//			if(time-timeCurrent>=4000){
//				timeCurrent=GetMilSecTick();
//				LEDControl_SetStatus(LEDREDA3_GPIO_PORT, LEDREDA3_PIN, 1);
//				LEDControl_SetStatus(LEDREDA10_GPIO_PORT, LEDREDA10_PIN, 1);
//			}
//		}
//		 if(ButtonRead_Status(BUTTONA4_GPIO_PORT, BUTTONA4_PIN) == 0) { // Nếu nút được nhấn
//		        if(!isButtonPressed) { // Nếu đây là lần đầu nút được nhấn
//		            isButtonPressed = 1; // Đặt cờ nhấn nút
//		            timeStart = GetMilSecTick(); // Lấy thời gian hiện tại
//		        }
//
//		        if((GetMilSecTick() - timeStart >= 4000) && isButtonPressed) { // Sau 4 giây
//		            // Bật đèn LED
//		            LEDControl_SetStatus(LEDREDA3_GPIO_PORT, LEDREDA3_PIN, 1);
//		            LEDControl_SetStatus(LEDREDA10_GPIO_PORT, LEDREDA10_PIN, 1);
//		            break; // Thoát khỏi vòng lặp sau khi bật đèn
//		        }
//		    } else {
//		        isButtonPressed = 0; // Nếu nút không được nhấn, reset cờ nhấn nút
//		    }
//		// yeu cau 4
//		 if(ButtonRead_Status(BUTTONB0_GPIO_PORT, BUTTONB0_PIN) == 1) { // Nếu nút được nhấn
//		 		        if(!isButtonPressed) { // Nếu đây là lần đầu nút được nhấn
//		 		            isButtonPressed = 1; // Đặt cờ nhấn nút
//		 		            timeStart = GetMilSecTick(); // Lấy thời gian hiện tại
//		 		        }
//
//		 		        if((GetMilSecTick() - timeStart >= 4000) && isButtonPressed) { // Sau 4 giây
//		 		            // Bật đèn LED
//		 		            LEDControl_SetStatus(LEDREDA3_GPIO_PORT, LEDREDA3_PIN, 0);
//		 		            LEDControl_SetStatus(LEDREDA10_GPIO_PORT, LEDREDA10_PIN, 0);
//		 		            break; // Thoát khỏi vòng lặp sau khi bật đèn
//		 		        }
//		 		    } else {
//		 		        isButtonPressed = 1; // Nếu nút không được nhấn, reset cờ nhấn nút
//		 		    }
//	}
		while (ButtonRead_Status(BUTTONA4_GPIO_PORT, BUTTONA4_PIN) == 0) {
			delay_ms(1000);
			cnt++;
			if (cnt >= 3) {
				break;
			}
		}
		if (cnt >= 3) {
			LEDControl_SetStatus(LEDREDA3_GPIO_PORT, LEDREDA3_PIN, 1);
			LEDControl_SetStatus(LEDREDA10_GPIO_PORT, LEDREDA10_PIN, 1);
			cnt = 0;
		} else {
			cnt = 0;
		}

		while (ButtonRead_Status(BUTTONB0_GPIO_PORT, BUTTONB0_PIN) == 0) {
			delay_ms(1000);
			cnt1++;
			if (cnt1 >= 3) {
				break;
			}
		}
		if (cnt1 >= 3) {
			LEDControl_SetStatus(LEDREDA3_GPIO_PORT, LEDREDA3_PIN, 0);
			LEDControl_SetStatus(LEDREDA10_GPIO_PORT, LEDREDA10_PIN, 0);
			cnt1 = 0;
		} else {
			cnt1 = 0;
		}
	}
}

/**
 * @func   delay_ms
 * @brief  delay milisecon
 * @param  None
 * @retval None
 */
void delay_ms(uint32_t ms) {

	uint32_t startTime = GetMilSecTick(); //Lưu lại thời điểm hiện tại.
	while (CalculatorTime(startTime, GetMilSecTick()) <= ms)
		;

//	Đợi cho đến khi hết khoảng time cài đặt

}

/**
 * @func   CalculatorTime
 * @brief  Caculator time button
 * @param  None
 * @retval None
 */
uint32_t CalculatorTime(uint32_t dwTimeInit, uint32_t dwTimeCurrent) {
	uint32_t dwTimeTotal;
	if (dwTimeCurrent >= dwTimeInit) {
		dwTimeTotal = dwTimeCurrent - dwTimeInit;
	} else {
		dwTimeTotal = 0xFFFFFFFFU + dwTimeCurrent - dwTimeInit;
	}
	return dwTimeTotal;

}

/**
 * @func   AppCommon
 * @brief  Init everything
 * @param  None
 * @retval None
 */
static void AppCommon() {
	SystemCoreClockUpdate();
	TimerInit();

	SPI1_Init();
	I2C_Init_temphumi();
	TemHumSensor_Init();
	Ucglib4WireSWSPI_begin(&ucg, UCG_FONT_MODE_SOLID); //là hàm khởi tạo LCD.
	ucg_ClearScreen(&ucg);
	ucg_SetFont(&ucg, ucg_font_ncenR12_hr);
	ucg_SetColor(&ucg, 0, 255, 255, 255);
	ucg_SetColor(&ucg, 1, 0, 0, 0);
	ucg_SetRotate180(&ucg);
	Scan_TimeSensor(5000);
	time_initial = GetMilSecTick();
}

/**
 * @func   SPI1_Init
 * @brief  Init SPI
 * @param  None
 * @retval None
 */
static void SPI1_Init(void) {
	GPIO_InitTypeDef GPIO_InitStructure;

	/* GPIOA, GPIOB and GPIOC Clocks enable */
	RCC_AHB1PeriphClockCmd(
			RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC,
			ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_InitStructure.GPIO_Pin = SPI1_SCK_PIN | SPI1_MOSI_PIN | SPI1_RS_PIN
			| SPI1_MODE_PIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = SPI1_CS_PIN | SPI1_ENABLE_PIN;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = SPI1_RST_PIN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

/**
 * @func   I2C_Init_temphumi
 * @brief  Init I2C temp and humi
 * @param  None
 * @retval None
 */
static void I2C_Init_temphumi(void) {
// Initialization struct
	I2C_InitTypeDef I2C_InitStruct;
	GPIO_InitTypeDef GPIO_InitStruct;

	// Step 1: Initialize I2C
	RCC_APB1PeriphClockCmd(I2Cx_RCC, ENABLE);
	I2C_InitStruct.I2C_ClockSpeed = 400000;
	I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStruct.I2C_OwnAddress1 = 0x00;
	I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_Init(I2Cx_SENSOR, &I2C_InitStruct);
	I2C_Cmd(I2Cx_SENSOR, ENABLE);

	// Step 2: Initialize GPIO as open drain alternate function
	RCC_AHB1PeriphClockCmd(I2C_GPIO_RCC, ENABLE);
	GPIO_InitStruct.GPIO_Pin = I2C_PIN_SCL | I2C_PIN_SDA;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(I2C_GPIO, &GPIO_InitStruct);

	/* Connect PXx to I2C_SCL */
	GPIO_PinAFConfig(I2C_GPIO, GPIO_PinSource8, GPIO_AF_I2C1);

	/* Connect PXx to I2C_SDA */
	GPIO_PinAFConfig(I2C_GPIO, GPIO_PinSource9, GPIO_AF_I2C1);
}

/*
 * @func    I2C_start
 * @brief   Generate I2C start condition
 * @param   None
 * @retval  None
 */
void I2C_start(void) {
	// Wait until I2Cx is not busy anymore
	while (I2C_GetFlagStatus(I2Cx_SENSOR, I2C_FLAG_BUSY))
		;

	// Generate start condition
	I2C_GenerateSTART(I2Cx_SENSOR, ENABLE);

	// Wait for I2C EV5.
	// It means that the start condition has been correctly released
	// on the I2C bus (the bus is free, no other devices is communicating))
	while (!I2C_CheckEvent(I2Cx_SENSOR, I2C_EVENT_MASTER_MODE_SELECT))
		;
}

/**
 * @func    I2C_address_direction
 * @brief   e
 * @param   None
 * @retval  None
 */
void I2C_address_direction(uint8_t address, uint8_t direction) {
	// Send slave address
	I2C_Send7bitAddress(I2Cx_SENSOR, address, direction);

	// Wait for I2C EV6
	// It means that a slave acknowledges his address
	if (direction == I2C_Direction_Transmitter)		// truyền
	{
		while (!I2C_CheckEvent(I2Cx_SENSOR,
				I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
			;
	} else if (direction == I2C_Direction_Receiver)  // nhận
	{
		while (!I2C_CheckEvent(I2Cx_SENSOR,
				I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
			;
	}
}

/**
 * @func    I2C_transmit
 * @brief   Transmit one byte to I2C bus
 * @param   None
 * @retval  None
 */
void I2C_transmit(uint8_t byte) {
	// Send data byte
	I2C_SendData(I2Cx_SENSOR, byte);
	// Wait for I2C EV8_2.
	// It means that the data has been physically shifted out and
	// output on the bus)
	while (!I2C_CheckEvent(I2Cx_SENSOR, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
		;
}

/**
 * @func    I2C_stop
 * @brief   Generate I2C stop condition
 * @param   None
 * @retval  None
 */
void I2C_stop(void) {
	// Generate I2C stop condition
	I2C_GenerateSTOP(I2Cx_SENSOR, ENABLE);
}

/**
 * @func    I2C_receive_nack
 * @brief   receive data
 * @param   None
 * @retval  None
 */
uint8_t I2C_receive_nack(void) {
	// Disable ACK of received data
	I2C_AcknowledgeConfig(I2Cx_SENSOR, DISABLE);
	// Wait for I2C EV7
	// It means that the data has been received in I2C data register
	while (!I2C_CheckEvent(I2Cx_SENSOR, I2C_EVENT_MASTER_BYTE_RECEIVED))
		;

	// Read and return data byte from I2C data register
	return I2C_ReceiveData(I2Cx_SENSOR);
}

/**
 * @func    I2C_receive_ack
 * @brief   receive data
 * @param   None
 * @retval  None
 */
uint8_t I2C_receive_ack(void) {
	// Enable ACK of received data
	I2C_AcknowledgeConfig(I2Cx_SENSOR, ENABLE);
	// Wait for I2C EV7
	// It means that the data has been received in I2C data register
	while (!I2C_CheckEvent(I2Cx_SENSOR, I2C_EVENT_MASTER_BYTE_RECEIVED))
		;

	// Read and return data byte from I2C data register
	return I2C_ReceiveData(I2Cx_SENSOR);
}

/**
 * @func    TemHumSensor_readRegister
 * @brief   communicate register in sensor
 * @param   None
 * @retval  None
 */
static void TemHumSensor_readRegister(uint8_t address,	// Địa chỉ cảm biến.
		uint8_t *pAddressRegister, // Địa chỉ của thanh ghi chứa dữ liệu nhiệt độ, độ ẩm.
		uint8_t *pDataRegister, // Dữ liệu đọc được từ thanh ghi tương ứng.
		uint8_t Length_Data, //Độ dài Dữ liệu đọc được từ thanh ghi tương ứng.
		uint16_t delay)	//Dữ liệu đọc được từ thanh ghi tương ứng.
{
	uint8_t LengthCmd = pAddressRegister[0];

	I2C_start();	// conditon start
	I2C_address_direction(address << 1, I2C_Direction_Transmitter); // send slave address - Transmit

	for (uint8_t i = 1; i < LengthCmd; i++) {
		I2C_transmit(pAddressRegister[i]);		// Send Data from register
	}

	if (delay > 0) {
		delay_ms(delay);
	}

	I2C_stop();		// condition stop

	I2C_start(); // condition start
	I2C_address_direction(address << 1, I2C_Direction_Receiver); // send slave address - Received

	for (uint8_t i = 0; i < Length_Data; i++) {
		if (i == (Length_Data - 1)) {
			pDataRegister[i] = I2C_receive_nack();  // 	NA
		} else {
			pDataRegister[i] = I2C_receive_ack();   // A
		}
	}
	I2C_stop();

}

/**
 * @func    GetTemp_Sensor
 * @brief   Get value temperature
 * @param   None
 * @retval  Temperature
 */
uint32_t GetTemp_Sensor(void) {
	uint32_t RT;
	uint8_t pRT[3] = { 0 };	// pRT[0]: MSB		pRT[1]: LSB

	uint8_t CMD_MEASURE_TEMP[2] = { 2, 0xE3 }; // gửi độ dài byte cần truyền và CMD_MEASURE chế độ HOLD MASTER MODE

	TemHumSensor_readRegister(SI7020_ADDR, CMD_MEASURE_TEMP, pRT, 3, 4);

	RT = (pRT[0] << 8) + pRT[1];// RT_CODE (MSB << 8) + LSB		// Temp = ((17572* RT)/(0xFFu +1) - 4685)/100
	RT = ((RT * 17572) >> 16) - 4685;
	RT = RT / 100;
	return RT;
}

/**
 * @func    GetHumi_Sensor
 * @brief   Get value humidity
 * @param   None
 * @retval  Humidity
 */
uint32_t GetHumi_Sensor(void) {
	uint32_t RH;
	uint8_t pRH[3] = { 0 };

	uint8_t CMD_MEASURE_HUMI[2] = { 2, 0xE5 }; // gửi độ dài byte cần truyền và CMD_MEASURE chế độ HOLD MASTER MODE

	TemHumSensor_readRegister(SI7020_ADDR, CMD_MEASURE_HUMI, pRH, 3, 8);

	RH = (pRH[0] << 8) + pRH[1]; // RH_CODE (MSB << 8) + LSB
	RH = ((RH * 12500) >> 16) - 600;
	RH = RH / 100;

	return RH;
}

/**
 * @func 	processGetValueSensor
 * @brief	Get value sensor
 * @param	None
 * @retval	None
 */
void processGetValueSensor(void) {
	// Get timer
	time_current = GetMilSecTick();

	if (time_current >= time_initial) {
		time_total += time_current - time_initial;
	} else {
		time_total += 0xFFFFFFFFU - time_current + time_initial;
	}
	if (time_total == CYCLE_SEND_DATA_1) {
		//Dùng để đọc giá trị nhiệt độ trả về từ cảm biến thông qua giao tiếp I2C.
		temperature = (uint8_t) (GetTemp_Sensor());

		//Dùng để đọc giá trị độ ẩm trả về từ cảm biến thông qua giao tiếp I2C.
		humidity = (uint8_t) (GetHumi_Sensor());

	}
	if (time_total >= CYCLE_SEND_DATA_2) {
		temperature1 = (uint8_t) (GetTemp_Sensor());

		humidity1 = (uint8_t) (GetHumi_Sensor());

		// nếu thời gian chênh lệch giữa lần đo 1 và lần đo 2 là 2 thì sẽ cập nhật giá trị sensor
		if (((temperature1 > temperature) && (temperature1 - temperature >= 2))
				|| ((temperature1 < temperature)
						&& (temperature1 - temperature <= 2))
				|| ((humidity1 > humidity) && (humidity1 - humidity >= 2))
				|| ((humidity1 < humidity) && (humidity1 - humidity <= 2))) {
			ucg_DrawString(&ucg, 0, 32, 0, "Nhom 2");
			memset(src1, 0, sizeof(src1));
			sprintf(src1, " Temp = %d oC  ", temperature1);
			ucg_DrawString(&ucg, 0, 52, 0, src1);

			memset(src2, 0, sizeof(src2));
			sprintf(src2, " Humi = %3d %%   ", humidity1);
			ucg_DrawString(&ucg, 0, 72, 0, src2);
		}

		time_total = 0;
		// Reset lại biến thời gian sau quá trình đọc giá trị nhiệt độ - độ ẩm.

	}
	time_initial = time_current;
}

/**
 * @func 	Scan_SensorLCD
 * @brief	Scan display value sensor
 * @param	None
 * @retval	None
 */
void Scan_SensorLCD(void) {
	ucg_DrawString(&ucg, 0, 32, 0, "Nhom 2");
	memset(src3, 0, sizeof(src3));
	sprintf(src3, " Nhiet do = %d oC  ", temperature);
	ucg_DrawString(&ucg, 0, 52, 0, src3);

	memset(src4, 0, sizeof(src4));
	sprintf(src4, " Do am = %3d %%   ", humidity);
	ucg_DrawString(&ucg, 0, 72, 0, src4);
}
/**
 * @func 	Scan_1s
 * @brief	Scan period 1s
 * @param	None
 * @retval	None
 */
void Scan_TimeSensor(uint32_t byRepeats) {
	if (idTimer != NO_TIMER) {
		TimerStop(idTimer);
	}
	idTimer = TimerStart("Scan_sensor", byRepeats, TIMER_REPEAT_FOREVER,
			(void*) Scan_SensorLCD, NULL);
}
/* modified------------------------------------*/

/**
 * @func   AppInitCommon
 * @brief  Application initializer
 * @param  None
 * @retval None
 */
static void AppInitCommon(void) {
	SystemCoreClockUpdate(); 	// Cấp nguồn clock cho vi điều khiển 84 Mhz.
	TimerInit();
	SPI1_Init();// Khởi tạo TIMER sử dụng system tick.
	I2C_Init_temphumi();
	TemHumSensor_Init();
	LightSensor_AdcInit();		// Khởi tạo ngoại vi ADC ở chế độ Polling
	LedControl_TimerOCInit(); // Khởi tạo ngoại vi TIMER ở chế độ pawm xung output compare

	KalmanFilterInit(2, 2, 0.001); // Khởi tạo bộ lọc Kalman

	Ucglib4WireSWSPI_begin(&ucg, UCG_FONT_MODE_SOLID);
	ucg_SetFont(&ucg, ucg_font_ncenR10_hf);
	ucg_ClearScreen(&ucg);
	ucg_SetColor(&ucg, 0, 255, 255, 255);
	ucg_SetColor(&ucg, 1, 0, 0, 0);
	ucg_SetRotate180(&ucg);
	Scan_TimeSensor(5000);
	time_initial = GetMilSecTick();
}
/**
 * @func   LightSensor_AdcInit
 * @brief  Light Sensor ADC Polling initialize
 * @param  None
 * @retval None
 */
static void LightSensor_AdcInit(void) {
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable peripheral clocks ***********************************************/
	RCC_APB2PeriphClockCmd(ADCx_CLK, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = ADC_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(ADC_PORT, &GPIO_InitStructure);

	/* ADC Deinit ***************************************************/
	ADC_DeInit();

	/* ADC Common Init ********************************************************/
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);

	/* ADC1 Init **************************************************************/
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 1;
	ADC_Init(ADC1, &ADC_InitStructure);

	//Polling Configure
	ADC_EOCOnEachRegularChannelCmd(ADC1, ENABLE);
	ADC_ContinuousModeCmd(ADC1, DISABLE);
	ADC_DiscModeChannelCountConfig(ADC1, 1);
	ADC_DiscModeCmd(ADC1, ENABLE);

	/* ADC1 regular channel15 configuration ************************************/
	ADC_RegularChannelConfig(ADC1, ADC_Channel_15, 1, ADC_SampleTime_15Cycles);

	/* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE);
}
/**
 * @func   LedControl_TimerOCInit
 * @brief  Control Led with Timer Output Capture
 * @param  None
 * @retval None
 */
static void LedControl_TimerOCInit(void) {
	GPIO_InitTypeDef GPIO_InitStruct;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_OCInitTypeDef TIM_OC_InitStruct;

	// GPIO Configure
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_TIM1);

	//TimeBase Configure

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_Prescaler = 0;

	/************************************************************************
	 * 	FPwm = Fclk / (ARR+1) * (PSC + 1)																	*
	 ************************************************************************/
	TIM_TimeBaseInitStruct.TIM_Period = 8399; // ARR = 8399
	TIM_TimeBaseInitStruct.TIM_ClockDivision = 0;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStruct);

	//TimeIC Configure
	TIM_OC_InitStruct.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OC_InitStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OC_InitStruct.TIM_Pulse = 0;
	TIM_OC_InitStruct.TIM_OCPolarity = TIM_OCPolarity_Low;

	TIM_OC4Init(TIM1, &TIM_OC_InitStruct);

	TIM_Cmd(TIM1, ENABLE);

	TIM_CtrlPWMOutputs(TIM1, ENABLE);
}
/**
 * @func   LightSensor_AdcPollingRead
 * @brief  Read value from ADC
 * @param  None
 * @retval None
 */
uint16_t LightSensor_AdcPollingRead(void) {
	uint16_t result = 0;

	// Bắt đầu quá trình chuyển đổi ADC
	ADC_SoftwareStartConv(ADCx_SENSOR);

	// Đợi quá trình chuyển đổi được hoàn thành để đọc dữ liệu
	while (ADC_GetFlagStatus(ADCx_SENSOR, ADC_FLAG_EOC) == RESET)
		;

	// Read value
	result = ADC_GetConversionValue(ADCx_SENSOR);

	return result;
}

/**
 * @func   Kanman_Light
 * @brief  Use Kanman filter for Light Sensor
 * @param  None
 * @retval None
 */
uint16_t Kanman_Light(uint16_t lightLevel) {
	lightLevelAfterFilter = KalmanFilter_updateEstimate(lightLevel);

	return lightLevelAfterFilter;
}

/**
 * @func   LedControl_TimerOCSetPwm
 * @brief  Control Led with PWM
 * @param  None
 * @retval None
 */
static void LedControl_TimerOCSetPwm(uint32_t Compare) {
	TIM_SetCompare4(TIM1, Compare);
}
/**
 * @func   ABL_Process
 * @brief  AutoBrightness use Light Sensor
 * @param  None
 * @retval None
 */
static void ABL_Process(void) {
	uint32_t dwTimeCurrent;
	static uint32_t dwTimeTotal, dwTimeInit;

	dwTimeCurrent = GetMilSecTick();

	if (dwTimeCurrent >= dwTimeInit) {
		dwTimeTotal += dwTimeCurrent - dwTimeInit;
	} else {
		dwTimeTotal += 0xFFFFFFFFU - dwTimeCurrent + dwTimeInit;
	}

	if (dwTimeTotal >= 100) {
		dwTimeTotal = 0;
		AdcValue = LightSensor_AdcPollingRead(); // Đọc giá trị ADC đo được từ cảm biến ánh sáng
		Kanman_light = Kanman_Light(AdcValue);

		LedControl_TimerOCSetPwm(Kanman_light);
		memset(src5, 0, sizeof(src3));
		sprintf(src5, " Light = %d lux  ", Kanman_light);
		ucg_DrawString(&ucg, 0, 72, 0, src5);
	}
	dwTimeInit = dwTimeCurrent;
}
