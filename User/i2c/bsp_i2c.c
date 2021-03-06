#include "bsp_i2c.h"               	

/**
  * @brief  I2C初始化
  * @param  无
  * @retval 无
  */
void I2C_1_Init(void)
{	
	GPIO_InitTypeDef  GPIO_InitStructure;
	I2C_InitTypeDef  I2C_InitStructure; 

	/* 使能与 I2C1 有关的时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1,ENABLE);  
    
	/* PB6-I2C1_SCL、PB7-I2C1_SDA*/
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;	// 开漏输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;	//I2C模式
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;	//高电平数据稳定，低电平数据变化 SCL 时钟线的占空比 2:1
	I2C_InitStructure.I2C_OwnAddress1 = 0X0A;	//这个地址只要与STM32外挂的I2C器件地址不一样即可
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable ;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;	//I2C的寻址模式
	I2C_InitStructure.I2C_ClockSpeed = 400000;	//通信速率
	I2C_Init(I2C1, &I2C_InitStructure);	//I2C1初始化
	
	I2C_Cmd(I2C1, ENABLE);	//使能 I2C1
}
