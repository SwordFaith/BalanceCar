#include "bsp_i2c.h"               	

/**
  * @brief  I2C��ʼ��
  * @param  ��
  * @retval ��
  */
void I2C_1_Init(void)
{	
	GPIO_InitTypeDef  GPIO_InitStructure;
	I2C_InitTypeDef  I2C_InitStructure; 

	/* ʹ���� I2C1 �йص�ʱ�� */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1,ENABLE);  
    
	/* PB6-I2C1_SCL��PB7-I2C1_SDA*/
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;	// ��©���
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;	//I2Cģʽ
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;	//�ߵ�ƽ�����ȶ����͵�ƽ���ݱ仯 SCL ʱ���ߵ�ռ�ձ� 2:1
	I2C_InitStructure.I2C_OwnAddress1 = 0X0A;	//�����ַֻҪ��STM32��ҵ�I2C������ַ��һ������
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable ;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;	//I2C��Ѱַģʽ
	I2C_InitStructure.I2C_ClockSpeed = 400000;	//ͨ������
	I2C_Init(I2C1, &I2C_InitStructure);	//I2C1��ʼ��
	
	I2C_Cmd(I2C1, ENABLE);	//ʹ�� I2C1
}
