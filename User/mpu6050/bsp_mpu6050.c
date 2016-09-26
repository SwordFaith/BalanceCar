#include "bsp_mpu6050.h"
#include "bsp_i2c.h"
#include "scheduler.h"

#define MPU6050_SLAVE_ADDRESS 0xD0	//AD0�ӵ�
//#define MPU6050_SLAVE_ADDRESS 0xD2	//AD0�Ӹ�

/**
  * @brief   дһ���ֽڵ�MPU6050��
  *		@arg pBuffer:	1Byte��Ϣ
  *		@arg WriteAddr:	д��ַ 
  */
void I2C_MPU6050_ByteWrite(u8 pBuffer, u8 WriteAddr)
{
	I2C_GenerateSTART(I2C1, ENABLE);	//д��ʼ�ź� Send STRAT condition 
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));	//���I2C�����¼� EV5 ��������¼� 
																//�ɹ�������ʼ�źź������ź�EV5
	I2C_Send7bitAddress(I2C1, MPU6050_SLAVE_ADDRESS, I2C_Direction_Transmitter);	//��I2C�����Ϸ���Ҫд����Ϣ���豸��ַ
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));	//������I2C�豸Ѱַ���õ�Ӧ��� ����EV6
	I2C_SendData(I2C1, WriteAddr);	//����MPU6050��Ŀ��Ĵ�����ַ
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));	//��鷢�ͳɹ��¼�
	I2C_SendData(I2C1, pBuffer); 	//����Ҫд������ݣ�1�ֽڣ�
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));	//��鷢�ͳɹ��¼�
	I2C_GenerateSTOP(I2C1, ENABLE);	//����ֹͣ�ź�
}


/**
  * @brief   ��MPU6050�����ȡ n Byte ����
  *		@arg pBuffer:��Ŵ�MPU6050��ȡ�����ݵĻ�����ָ�루�����׵�ַ��
  *		@arg WriteAddr:MPU6050��Ŀ�����ݴ�ŵĵ�ַ
  *     @arg NumByteToWrite:Ҫ��MPU6050��ȡ���ֽ���
  *     
  *     ��ʵ������������Ǵ�EEPROM����Ĺ����ģ�ӵ�ж�ȡn���ֽڵ���������MPU6050�ļĴ����ձ�ֻ��1�ֽ�
  *     �ֽ�������ƽʱ����д1��Ȼ�󻺳���ָ�����һ��������int������ȡ��ַ
  */
void I2C_MPU6050_BufferRead(u8* pBuffer, u8 ReadAddr, u16 NumByteToRead)
{  
	//��ȡ����Ӧ���������ģ�����д�ķ�ʽ����Ҫ��ȡ�ĵ�ַд��ȥ��Ȼ�����ö��ķ�ʽ���ն������Ϣ��
	//��Ϣ������Ϻ�����ֹͣͨѶ
	
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY)); 	//���SDA�Ƿ���У�Ӧ����Ϊ����Ӧ������ͨ�ţ�
													//Added by Najoua 27/08/2008    
	
	I2C_GenerateSTART(I2C1, ENABLE);	//������ʵ�ź� Send START condition */
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));	//�����ʼ�źŵĻ�Ӧ
	I2C_Send7bitAddress(I2C1, MPU6050_SLAVE_ADDRESS, I2C_Direction_Transmitter);	//����MPU6050��ַ
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	I2C_Cmd(I2C1, ENABLE);	/* Clear EV6 by setting again the PE bit */
	I2C_SendData(I2C1, ReadAddr);	//����Ҫ��ȡ�ļĴ����ĵ�ַ
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));	//����Ƿ��ͳɹ�
  
	I2C_GenerateSTART(I2C1, ENABLE);	//���·�����ʼ�ź�
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));	//�����ʼ�źŷ���
	I2C_Send7bitAddress(I2C1, MPU6050_SLAVE_ADDRESS, I2C_Direction_Receiver);	//����MPU6050��ַ��ͬʱ��ʾ���ö��Ĺ�����ʽ
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));	//��鷴��

	while(NumByteToRead)  // While there is data to be read ��ѭ���������
	{
		//�൱��˵���ڽ����굹���ڶ����ֽں�������I2C_ReceiveData���������˽��ܳɹ�Ӧ��Ȼ��ӻ����������һ���ֽڣ
		//Ȼ�����������Źر������һ���ֽڵĽ��ճɹ�������ֱ�ӷ�����ͨ�Ž�����־
		//ֻ����Ϊ���ڽ��յ������ڶ��ֽڵ�Ӧ��ɹ��źź󣬴ӻ��������������һ���ֽڣ���ʱֹͣ�źŻ�û��������
		//Ҳ����˵���������յ����һ���ֽڵ�һ˲�䣨Ӳ���Զ����գ����������������ֹͣ���
		//����ֹͣ�����������ȥ������һ���ֽ��յ���ʲô����û�յ���
		if(NumByteToRead == 1)
		{
			/* Disable Acknowledgement */
			I2C_AcknowledgeConfig(I2C1, DISABLE);

			/* Send STOP Condition */
			I2C_GenerateSTOP(I2C1, ENABLE);
		}

		/* Test on EV7 and clear it */
		if(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED))  
		{      
			/* Read a byte from the EEPROM */
			*pBuffer = I2C_ReceiveData(I2C1);	//�����ݴ���洢��

			/* Point to the next location where the byte read will be saved */
			pBuffer++; //ָ��ָ����һ���洢�ռ䣨������һλ��

			/* Decrement the read bytes counter */
			NumByteToRead--;        
		}   
  }

  /* Enable Acknowledgement to be ready for another reception */
  I2C_AcknowledgeConfig(I2C1, ENABLE);	//�ָ�I2C���Զ�Ӧ��
}


//**********************************************�캽��mpu6050������ֲ**********************************************888
u8 mpu6050_ok;	// =1 ����
				// =0 ����

//����д����ת�Ӻ�����û��ʹ��SlaveAddress��Σ��˲����ڱ��ļ�ͷ���Ժ궨����ʽԤ��
//IIC��n�ֽ�����ת�Ӻ���
u8 IIC_Read_nByte(u8 SlaveAddress, u8 REG_Address, u8 len, u8 *buf)
{
	I2C_MPU6050_BufferRead(buf, REG_Address, len);
	return 0;
}

//I2C�ӿ�д��ת�Ӻ���
u8 IIC_Write_1Byte(u8 SlaveAddress,u8 REG_Address,u8 REG_data)
{
	I2C_MPU6050_ByteWrite(REG_data, REG_Address);
	return 0;
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:		u8 IICwriteBit(u8 dev, u8 reg, u8 bitNum, u8 data)
*��������:	  �� �޸� д ָ���豸 ָ���Ĵ���һ���ֽ� �е�1��λ
����	dev  Ŀ���豸��ַ
reg	   �Ĵ�����ַ
bitNum  Ҫ�޸�Ŀ���ֽڵ�bitNumλ
data  Ϊ0 ʱ��Ŀ��λ������0 ���򽫱���λ
����   �ɹ� Ϊ1 
ʧ��Ϊ0
*******************************************************************************/ 
void IICwriteBit(u8 dev, u8 reg, u8 bitNum, u8 data){
	u8 b;
	IIC_Read_nByte(dev, reg, 1, &b);
	b = (data != 0) ? (b | (1 << bitNum)) : (b & ~(1 << bitNum));
	mpu6050_ok = !( IIC_Write_1Byte(dev, reg, b) );	//IIC_Write_1Byte�����������0������ȥ��mpu6050_ok = 1
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:		u8 IICwriteBits(u8 dev,u8 reg,u8 bitStart,u8 length,u8 data)
*��������:	    �� �޸� д ָ���豸 ָ���Ĵ���һ���ֽ� �еĶ��λ
����	dev  Ŀ���豸��ַ
reg	   �Ĵ�����ַ
bitStart  Ŀ���ֽڵ���ʼλ
length   λ����
data    ��Ÿı�Ŀ���ֽ�λ��ֵ
����   �ɹ� Ϊ1 
ʧ��Ϊ0
*******************************************************************************/ 
void IICwriteBits(u8 dev,u8 reg,u8 bitStart,u8 length,u8 data)
{
	
	u8 b,mask;
	IIC_Read_nByte(dev, reg, 1, &b);
	mask = (0xFF << (bitStart + 1)) | 0xFF >> ((8 - bitStart) + length - 1);
	data <<= (8 - length);
	data >>= (7 - bitStart);
	b &= mask;
	b |= data;
	IIC_Write_1Byte(dev, reg, b);
}

/***********************************************************************************************************
												����ʵ�ֺ���
***********************************************************************************************************/

/**************************ʵ�ֺ���********************************************
*����ԭ��:		void MPU6050_setSleepEnabled(uint8_t enabled)
*��������:	    MPU6050 ��˯��ģʽ����
				0 ��
				1 ��
*******************************************************************************/
void MPU6050_setSleepEnabled(uint8_t enabled) {
	IICwriteBit(MPU6050_ADDRESS, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_SLEEP_BIT, enabled);
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:		void MPU6050_setClockSource(uint8_t source)
*��������:	    ����  MPU6050 ��ʱ��Դ
* CLK_SEL | Clock Source
* --------+--------------------------------------
* 0       | Internal oscillator
* 1       | PLL with X Gyro reference
* 2       | PLL with Y Gyro reference
* 3       | PLL with Z Gyro reference
* 4       | PLL with external 32.768kHz reference
* 5       | PLL with external 19.2MHz reference
* 6       | Reserved
* 7       | Stops the clock and keeps the timing generator in reset
*******************************************************************************/
void MPU6050_setClockSource(uint8_t source)
{
	IICwriteBits(MPU6050_ADDRESS, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_CLKSEL_BIT, MPU6050_PWR1_CLKSEL_LENGTH, source);
	
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:		void MPU6050_set_SMPLRT_DIV(uint16_t hz)
*��������:	    ���� MPU6050 �����������Ƶ��
*******************************************************************************/
void MPU6050_set_SMPLRT_DIV(uint16_t hz)
{
	IIC_Write_1Byte(MPU6050_ADDRESS, MPU6050_RA_SMPLRT_DIV,1000/hz - 1);
//	I2C_Single_Write(MPU6050_ADDRESS,MPU_RA_SMPLRT_DIV, (1000/sample_rate - 1));
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:		void MPU6050_setFullScaleGyroRange(uint8_t range)
*��������:	    ����  MPU6050 ������ ���������
*******************************************************************************/
void MPU6050_setFullScaleGyroRange(uint8_t range) {
	IICwriteBits(MPU6050_ADDRESS, MPU6050_RA_GYRO_CONFIG, MPU6050_GCONFIG_FS_SEL_BIT, MPU6050_GCONFIG_FS_SEL_LENGTH, range);
	IICwriteBits(MPU6050_ADDRESS, MPU6050_RA_GYRO_CONFIG,7, 3, 0x00);   //���Լ�
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:		void MPU6050_setFullScaleAccelRange(uint8_t range)
*��������:	    ����  MPU6050 ���ٶȼƵ��������
*******************************************************************************/
void MPU6050_setFullScaleAccelRange(uint8_t range) {
	IICwriteBits(MPU6050_ADDRESS, MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_AFS_SEL_BIT, MPU6050_ACONFIG_AFS_SEL_LENGTH, range);
	IICwriteBits(MPU6050_ADDRESS, MPU6050_RA_ACCEL_CONFIG,7, 3, 0x00);   //���Լ�
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:		void MPU6050_setDLPF(uint8_t mode)
*��������:	    ����  MPU6050 ���ֵ�ͨ�˲���Ƶ�����
*******************************************************************************/
void MPU6050_setDLPF(uint8_t mode)
{
	IICwriteBits(MPU6050_ADDRESS, MPU6050_RA_CONFIG, MPU6050_CFG_DLPF_CFG_BIT, MPU6050_CFG_DLPF_CFG_LENGTH, mode);
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:		void MPU6050_setI2CMasterModeEnabled(uint8_t enabled)
*��������:	    ���� MPU6050 �Ƿ�ΪAUX I2C�ߵ�����
*******************************************************************************/
void MPU6050_setI2CMasterModeEnabled(uint8_t enabled) {
	IICwriteBit(MPU6050_ADDRESS, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_I2C_MST_EN_BIT, enabled);
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:		void MPU6050_setI2CBypassEnabled(uint8_t enabled)
*��������:	    ���� MPU6050 �Ƿ�ΪAUX I2C�ߵ�����
*******************************************************************************/
void MPU6050_setI2CBypassEnabled(uint8_t enabled) {
	IICwriteBit(MPU6050_ADDRESS, MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_I2C_BYPASS_EN_BIT, enabled);
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:		void MPU6050_initialize(void)
*��������:	    ��ʼ�� 	MPU6050 �Խ������״̬�����������ɿ��캽��mpu6050������
*******************************************************************************/
int MPU6050_Init(u16 lpf)
{ 
	u16 default_filter = 1;
	
	//ѡ��mpu6050�ڲ����ֵ�Ͳ�˲�������
	//�������ڲ���ͨ�˲��������ǲ����� 8MHz
	//  �����ڲ���ͨ�˲��������ǲ����� 1MHz
	//���ٶȼƲ�����1MHz
	switch(lpf)
	{
	case 5:
		default_filter = MPU6050_DLPF_BW_5;
		break;
	case 10:
		default_filter = MPU6050_DLPF_BW_10;
		break;
	case 20:
		default_filter = MPU6050_DLPF_BW_20;
		break;
	case 42:
		default_filter = MPU6050_DLPF_BW_42;
		break;
	case 98:
		default_filter = MPU6050_DLPF_BW_98;
		break;
	case 188:
		default_filter = MPU6050_DLPF_BW_188;
		break;
	case 256:
		default_filter = MPU6050_DLPF_BW_256;
		break;
	default:
		default_filter = MPU6050_DLPF_BW_42;
		break;
	}

	//�豸��λ
//	IIC_Write_1Byte(MPU6050_SLAVE_ADDRESS,MPU6050_RA_PWR_MGMT_1, 0x80);
	
	//����ʹ�õ�Delay()ֻ���ڳ�ʼ���׶�ʹ�ã����������ʹ������Delay()���Ῠ����������
	MPU6050_setSleepEnabled(0); //���빤��״̬
	Delay_ms(10);
	MPU6050_setClockSource(MPU6050_CLOCK_PLL_ZGYRO);	//����ʱ��  0x6b   0x03
														//ʱ��Դѡ��MPU6050_CLOCK_INTERNAL��ʾ�ڲ�8M����
	Delay_ms(10);
	MPU6050_set_SMPLRT_DIV(1000);  //1000hz
	Delay_ms(10);
	MPU6050_setFullScaleGyroRange(MPU6050_GYRO_FS_2000);//������������� +-2000��ÿ��
	Delay_ms(10);
	MPU6050_setFullScaleAccelRange(MPU6050_ACCEL_FS_8);	//���ٶȶ�������� +-8G
	Delay_ms(10);
	MPU6050_setDLPF(default_filter);  //42hz
	Delay_ms(10);
	MPU6050_setI2CMasterModeEnabled(0);	 //����MPU6050 ����AUXI2C
	Delay_ms(10);
	MPU6050_setI2CBypassEnabled(1);	 //����������I2C��	MPU6050��AUXI2C	ֱͨ������������ֱ�ӷ���HMC5883L
	Delay_ms(10);
	
	return (mpu6050_ok == 0);
}



//��ȡMPU6050����Ĵ�����ֵ
void MPU6050_Read(MPU6050_STRUCT * mpu6050)
{
	IIC_Read_nByte(MPU6050_SLAVE_ADDRESS,MPU6050_RA_ACCEL_XOUT_H,14,mpu6050->mpu6050_buffer);
	
	/*ƴ��bufferԭʼ����*/
	mpu6050->Acc_I16.x = ((((int16_t)mpu6050->mpu6050_buffer[0]) << 8) | mpu6050->mpu6050_buffer[1]) ;
	mpu6050->Acc_I16.y = ((((int16_t)mpu6050->mpu6050_buffer[2]) << 8) | mpu6050->mpu6050_buffer[3]) ;
	mpu6050->Acc_I16.z = ((((int16_t)mpu6050->mpu6050_buffer[4]) << 8) | mpu6050->mpu6050_buffer[5]) ;
 
	mpu6050->Gyro_I16.x = ((((int16_t)mpu6050->mpu6050_buffer[ 8]) << 8) | mpu6050->mpu6050_buffer[ 9]) ;
	mpu6050->Gyro_I16.y = ((((int16_t)mpu6050->mpu6050_buffer[10]) << 8) | mpu6050->mpu6050_buffer[11]) ;
	mpu6050->Gyro_I16.z = ((((int16_t)mpu6050->mpu6050_buffer[12]) << 8) | mpu6050->mpu6050_buffer[13]) ;
	
	mpu6050->Tempreature = ((((int16_t)mpu6050->mpu6050_buffer[6]) << 8) | mpu6050->mpu6050_buffer[7]); //tempreature
	
}


////д��ʽת������
//void MPU6050_WriteReg(u8 reg_add,u8 reg_dat)
//{
//	I2C_MPU6050_ByteWrite(reg_dat,reg_add);
//}

////����ʽת������
//void MPU6050_ReadData(u8 reg_add,unsigned char* Read,u8 num)
//{
//	I2C_MPU6050_BufferRead(Read,reg_add,num);
//}

////�˶κ����ǿ������ģ�ֻ�ܱ�֤���������������ܱ�֤����׼ȷ��
////����ʱû���Լ죬ȱ��У׼
//void MPU6050_Init(void)
//{
//	//�ڳ�ʼ��֮ǰҪ��ʱһ��ʱ�䣬��û����ʱ����ϵ�����ϵ����ݿ��ܻ����
//	Delay_ms(500);
//		
//	MPU6050_WriteReg(MPU6050_RA_PWR_MGMT_1, 0x00);	     //�������״̬ �ر�ѭ��ģʽ �����¶ȴ����� ʹ���ڲ�8M����
//	MPU6050_WriteReg(MPU6050_RA_SMPLRT_DIV , 0x07);	    //�����ǲ����� ���������������Ƶ���Ӷ��õ������� 
//														//������ = ���������Ƶ�� / (1 + SMPLRT_DIV)
//	MPU6050_WriteReg(MPU6050_RA_CONFIG , 0x06);			//����5Hz��������ֵ�ͨ�˲�
//	MPU6050_WriteReg(MPU6050_RA_ACCEL_CONFIG , 0x01);	  //���ü��ٶȴ�����������2Gģʽ
//	MPU6050_WriteReg(MPU6050_RA_GYRO_CONFIG, 0x18);     //�������Լ켰������Χ������ֵ��0x18(���Լ죬2000deg/s)
//}

////��������ַ
//unsigned char MPU6050ReadID(void)
//{
//	unsigned char Re = 0;
//    MPU6050_ReadData(MPU6050_RA_WHO_AM_I,&Re,1);    //��������ַ
////     printf("%d\r\n",Re);
//	return Re;
//}

////�����x��y��z����д��ȥ�ģ�û�к�ʵ����ʹ��ǰһ��Ҫ��ʵ������
//void MPU6050ReadAcc(mpu6050_3D *accData)
//{
//    u8 buf[6];
//    MPU6050_ReadData(MPU6050_ACC_OUT, buf, 6);
//	accData->x = (buf[0] << 8) | buf[1];
//	accData->y = (buf[2] << 8) | buf[3];
//	accData->z = (buf[4] << 8) | buf[5];
//}
//void MPU6050ReadGyro(mpu6050_3D *gyroData)
//{
//    u8 buf[6];
//    MPU6050_ReadData(MPU6050_GYRO_OUT,buf,6);
//	gyroData->x = (buf[0] << 8) | buf[1];
//	gyroData->y = (buf[2] << 8) | buf[3];
//	gyroData->z = (buf[4] << 8) | buf[5];
//}

//void MPU6050ReadTemp(short *tempData)	//short 16λ
//{
//	u8 buf[2];
//    MPU6050_ReadData(MPU6050_RA_TEMP_OUT_H,buf,2);     //��ȡ�¶�ֵ
//	
//    *tempData = (buf[0] << 8) | buf[1];		//�򵥵İ�2��8λƴ��16λ
//}

//void MPU6050_ReturnTemp(short*Temperature)	
//{
//	short temp3;
//	u8 buf[2];
//	
//	MPU6050_ReadData(MPU6050_RA_TEMP_OUT_H,buf,2);     //��ȡ�¶�ֵ
//	temp3= (buf[0] << 8) | buf[1];
//	
//	*Temperature=(((double) (temp3 + 13200)) / 280)-13;	//��ȡ�¶Ⱥ������һ����ʽ���㣬��ʱ��ȷ������������������
//}


//void MPU6050ReadAcc(short *accData)
//{
//    u8 buf[6];
//    MPU6050_ReadData(MPU6050_ACC_OUT, buf, 6);
//    accData[0] = (buf[0] << 8) | buf[1];
//    accData[1] = (buf[2] << 8) | buf[3];
//    accData[2] = (buf[4] << 8) | buf[5];
//}
//void MPU6050ReadGyro(short *gyroData)
//{
//    u8 buf[6];
//    MPU6050_ReadData(MPU6050_GYRO_OUT,buf,6);
//    gyroData[0] = (buf[0] << 8) | buf[1];
//    gyroData[1] = (buf[2] << 8) | buf[3];
//    gyroData[2] = (buf[4] << 8) | buf[5];
//}
