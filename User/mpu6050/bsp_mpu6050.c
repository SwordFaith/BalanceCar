#include "bsp_mpu6050.h"
#include "bsp_i2c.h"
#include "scheduler.h"

#define MPU6050_SLAVE_ADDRESS 0xD0	//AD0接低
//#define MPU6050_SLAVE_ADDRESS 0xD2	//AD0接高

/**
  * @brief   写一个字节到MPU6050中
  *		@arg pBuffer:	1Byte信息
  *		@arg WriteAddr:	写地址 
  */
void I2C_MPU6050_ByteWrite(u8 pBuffer, u8 WriteAddr)
{
	I2C_GenerateSTART(I2C1, ENABLE);	//写起始信号 Send STRAT condition 
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));	//检测I2C外设事件 EV5 并清除该事件 
																//成功发送起始信号后会产生信号EV5
	I2C_Send7bitAddress(I2C1, MPU6050_SLAVE_ADDRESS, I2C_Direction_Transmitter);	//向I2C总线上发送要写入信息的设备地址
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));	//发送完I2C设备寻址并得到应答后 产生EV6
	I2C_SendData(I2C1, WriteAddr);	//发送MPU6050中目标寄存器地址
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));	//检查发送成功事件
	I2C_SendData(I2C1, pBuffer); 	//发送要写入的数据（1字节）
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));	//检查发送成功事件
	I2C_GenerateSTOP(I2C1, ENABLE);	//发送停止信号
}


/**
  * @brief   从MPU6050里面读取 n Byte 数据
  *		@arg pBuffer:存放从MPU6050读取的数据的缓冲区指针（数组首地址）
  *		@arg WriteAddr:MPU6050中目标数据存放的地址
  *     @arg NumByteToWrite:要从MPU6050读取的字节数
  *     
  *     其实啊，这个函数是从EEPROM那里改过来的，拥有读取n个字节的能力，但MPU6050的寄存器普遍只有1字节
  *     字节数那里平时就是写1，然后缓冲区指针就是一个变量（int）进行取地址
  */
void I2C_MPU6050_BufferRead(u8* pBuffer, u8 ReadAddr, u16 NumByteToRead)
{  
	//读取过程应该是这样的：先用写的方式，把要读取的地址写进去，然后再用读的方式接收对面的信息。
	//信息接收完毕后主动停止通讯
	
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY)); 	//检测SDA是否空闲（应该是为了适应多主机通信）
													//Added by Najoua 27/08/2008    
	
	I2C_GenerateSTART(I2C1, ENABLE);	//发送其实信号 Send START condition */
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));	//检查起始信号的回应
	I2C_Send7bitAddress(I2C1, MPU6050_SLAVE_ADDRESS, I2C_Direction_Transmitter);	//发送MPU6050地址
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	I2C_Cmd(I2C1, ENABLE);	/* Clear EV6 by setting again the PE bit */
	I2C_SendData(I2C1, ReadAddr);	//发送要读取的寄存器的地址
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));	//检查是否发送成功
  
	I2C_GenerateSTART(I2C1, ENABLE);	//重新发送起始信号
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));	//检查起始信号反馈
	I2C_Send7bitAddress(I2C1, MPU6050_SLAVE_ADDRESS, I2C_Direction_Receiver);	//发送MPU6050地址，同时表示采用读的工作方式
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));	//检查反馈

	while(NumByteToRead)  // While there is data to be read 再循环里读数据
	{
		//相当于说，在接受完倒数第二个字节后，主机的I2C_ReceiveData函数发出了接受成功应答，然后从机发出了最后一个字节�
		//然后主机紧接着关闭了最后一个字节的接收成功反馈，直接发送了通信结束标志
		//只能认为是在接收到倒数第二字节的应答成功信号后，从机立即发送了最后一个字节，这时停止信号还没发过来。
		//也就是说在主机接收到最后一个字节的一瞬间（硬件自动接收），主机软件发送了停止命令。
		//发完停止命令后，主机才去检查最后一个字节收到了什么（收没收到）
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
			*pBuffer = I2C_ReceiveData(I2C1);	//把数据存进存储区

			/* Point to the next location where the byte read will be saved */
			pBuffer++; //指针指向下一个存储空间（数组下一位）

			/* Decrement the read bytes counter */
			NumByteToRead--;        
		}   
  }

  /* Enable Acknowledgement to be ready for another reception */
  I2C_AcknowledgeConfig(I2C1, ENABLE);	//恢复I2C的自动应答
}


//**********************************************领航者mpu6050驱动移植**********************************************888
u8 mpu6050_ok;	// =1 正常
				// =0 错误

//读、写两个转接函数都没有使用SlaveAddress入参，此参数在本文件头部以宏定义形式预置
//IIC读n字节数据转接函数
u8 IIC_Read_nByte(u8 SlaveAddress, u8 REG_Address, u8 len, u8 *buf)
{
	I2C_MPU6050_BufferRead(buf, REG_Address, len);
	return 0;
}

//I2C接口写入转接函数
u8 IIC_Write_1Byte(u8 SlaveAddress,u8 REG_Address,u8 REG_data)
{
	I2C_MPU6050_ByteWrite(REG_data, REG_Address);
	return 0;
}

/**************************实现函数********************************************
*函数原型:		u8 IICwriteBit(u8 dev, u8 reg, u8 bitNum, u8 data)
*功　　能:	  读 修改 写 指定设备 指定寄存器一个字节 中的1个位
输入	dev  目标设备地址
reg	   寄存器地址
bitNum  要修改目标字节的bitNum位
data  为0 时，目标位将被清0 否则将被置位
返回   成功 为1 
失败为0
*******************************************************************************/ 
void IICwriteBit(u8 dev, u8 reg, u8 bitNum, u8 data){
	u8 b;
	IIC_Read_nByte(dev, reg, 1, &b);
	b = (data != 0) ? (b | (1 << bitNum)) : (b & ~(1 << bitNum));
	mpu6050_ok = !( IIC_Write_1Byte(dev, reg, b) );	//IIC_Write_1Byte正常情况返回0，经过去反mpu6050_ok = 1
}

/**************************实现函数********************************************
*函数原型:		u8 IICwriteBits(u8 dev,u8 reg,u8 bitStart,u8 length,u8 data)
*功　　能:	    读 修改 写 指定设备 指定寄存器一个字节 中的多个位
输入	dev  目标设备地址
reg	   寄存器地址
bitStart  目标字节的起始位
length   位长度
data    存放改变目标字节位的值
返回   成功 为1 
失败为0
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
												功能实现函数
***********************************************************************************************************/

/**************************实现函数********************************************
*函数原型:		void MPU6050_setSleepEnabled(uint8_t enabled)
*功　　能:	    MPU6050 的睡眠模式开关
				0 关
				1 开
*******************************************************************************/
void MPU6050_setSleepEnabled(uint8_t enabled) {
	IICwriteBit(MPU6050_ADDRESS, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_SLEEP_BIT, enabled);
}

/**************************实现函数********************************************
*函数原型:		void MPU6050_setClockSource(uint8_t source)
*功　　能:	    设置  MPU6050 的时钟源
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

/**************************实现函数********************************************
*函数原型:		void MPU6050_set_SMPLRT_DIV(uint16_t hz)
*功　　能:	    设置 MPU6050 的陀螺仪输出频率
*******************************************************************************/
void MPU6050_set_SMPLRT_DIV(uint16_t hz)
{
	IIC_Write_1Byte(MPU6050_ADDRESS, MPU6050_RA_SMPLRT_DIV,1000/hz - 1);
//	I2C_Single_Write(MPU6050_ADDRESS,MPU_RA_SMPLRT_DIV, (1000/sample_rate - 1));
}

/**************************实现函数********************************************
*函数原型:		void MPU6050_setFullScaleGyroRange(uint8_t range)
*功　　能:	    设置  MPU6050 陀螺仪 的最大量程
*******************************************************************************/
void MPU6050_setFullScaleGyroRange(uint8_t range) {
	IICwriteBits(MPU6050_ADDRESS, MPU6050_RA_GYRO_CONFIG, MPU6050_GCONFIG_FS_SEL_BIT, MPU6050_GCONFIG_FS_SEL_LENGTH, range);
	IICwriteBits(MPU6050_ADDRESS, MPU6050_RA_GYRO_CONFIG,7, 3, 0x00);   //不自检
}

/**************************实现函数********************************************
*函数原型:		void MPU6050_setFullScaleAccelRange(uint8_t range)
*功　　能:	    设置  MPU6050 加速度计的最大量程
*******************************************************************************/
void MPU6050_setFullScaleAccelRange(uint8_t range) {
	IICwriteBits(MPU6050_ADDRESS, MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_AFS_SEL_BIT, MPU6050_ACONFIG_AFS_SEL_LENGTH, range);
	IICwriteBits(MPU6050_ADDRESS, MPU6050_RA_ACCEL_CONFIG,7, 3, 0x00);   //不自检
}

/**************************实现函数********************************************
*函数原型:		void MPU6050_setDLPF(uint8_t mode)
*功　　能:	    设置  MPU6050 数字低通滤波的频带宽度
*******************************************************************************/
void MPU6050_setDLPF(uint8_t mode)
{
	IICwriteBits(MPU6050_ADDRESS, MPU6050_RA_CONFIG, MPU6050_CFG_DLPF_CFG_BIT, MPU6050_CFG_DLPF_CFG_LENGTH, mode);
}

/**************************实现函数********************************************
*函数原型:		void MPU6050_setI2CMasterModeEnabled(uint8_t enabled)
*功　　能:	    设置 MPU6050 是否为AUX I2C线的主机
*******************************************************************************/
void MPU6050_setI2CMasterModeEnabled(uint8_t enabled) {
	IICwriteBit(MPU6050_ADDRESS, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_I2C_MST_EN_BIT, enabled);
}

/**************************实现函数********************************************
*函数原型:		void MPU6050_setI2CBypassEnabled(uint8_t enabled)
*功　　能:	    设置 MPU6050 是否为AUX I2C线的主机
*******************************************************************************/
void MPU6050_setI2CBypassEnabled(uint8_t enabled) {
	IICwriteBit(MPU6050_ADDRESS, MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_I2C_BYPASS_EN_BIT, enabled);
}

/**************************实现函数********************************************
*函数原型:		void MPU6050_initialize(void)
*功　　能:	    初始化 	MPU6050 以进入可用状态。改自匿名飞控领航者mpu6050驱动。
*******************************************************************************/
int MPU6050_Init(u16 lpf)
{ 
	u16 default_filter = 1;
	
	//选择mpu6050内部数字低筒滤波器带宽
	//不开启内部低通滤波，陀螺仪采样率 8MHz
	//  开启内部低通滤波，陀螺仪采样率 1MHz
	//加速度计采样率1MHz
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

	//设备复位
//	IIC_Write_1Byte(MPU6050_SLAVE_ADDRESS,MPU6050_RA_PWR_MGMT_1, 0x80);
	
	//这里使用的Delay()只能在初始化阶段使用，任务调度中使用这种Delay()，会卡死整个调度
	MPU6050_setSleepEnabled(0); //进入工作状态
	Delay_ms(10);
	MPU6050_setClockSource(MPU6050_CLOCK_PLL_ZGYRO);	//设置时钟  0x6b   0x03
														//时钟源选择，MPU6050_CLOCK_INTERNAL表示内部8M晶振
	Delay_ms(10);
	MPU6050_set_SMPLRT_DIV(1000);  //1000hz
	Delay_ms(10);
	MPU6050_setFullScaleGyroRange(MPU6050_GYRO_FS_2000);//陀螺仪最大量程 +-2000度每秒
	Delay_ms(10);
	MPU6050_setFullScaleAccelRange(MPU6050_ACCEL_FS_8);	//加速度度最大量程 +-8G
	Delay_ms(10);
	MPU6050_setDLPF(default_filter);  //42hz
	Delay_ms(10);
	MPU6050_setI2CMasterModeEnabled(0);	 //不让MPU6050 控制AUXI2C
	Delay_ms(10);
	MPU6050_setI2CBypassEnabled(1);	 //主控制器的I2C与	MPU6050的AUXI2C	直通。控制器可以直接访问HMC5883L
	Delay_ms(10);
	
	return (mpu6050_ok == 0);
}



//读取MPU6050输出寄存器数值
void MPU6050_Read(MPU6050_STRUCT * mpu6050)
{
	IIC_Read_nByte(MPU6050_SLAVE_ADDRESS,MPU6050_RA_ACCEL_XOUT_H,14,mpu6050->mpu6050_buffer);
	
	/*拼接buffer原始数据*/
	mpu6050->Acc_I16.x = ((((int16_t)mpu6050->mpu6050_buffer[0]) << 8) | mpu6050->mpu6050_buffer[1]) ;
	mpu6050->Acc_I16.y = ((((int16_t)mpu6050->mpu6050_buffer[2]) << 8) | mpu6050->mpu6050_buffer[3]) ;
	mpu6050->Acc_I16.z = ((((int16_t)mpu6050->mpu6050_buffer[4]) << 8) | mpu6050->mpu6050_buffer[5]) ;
 
	mpu6050->Gyro_I16.x = ((((int16_t)mpu6050->mpu6050_buffer[ 8]) << 8) | mpu6050->mpu6050_buffer[ 9]) ;
	mpu6050->Gyro_I16.y = ((((int16_t)mpu6050->mpu6050_buffer[10]) << 8) | mpu6050->mpu6050_buffer[11]) ;
	mpu6050->Gyro_I16.z = ((((int16_t)mpu6050->mpu6050_buffer[12]) << 8) | mpu6050->mpu6050_buffer[13]) ;
	
	mpu6050->Tempreature = ((((int16_t)mpu6050->mpu6050_buffer[6]) << 8) | mpu6050->mpu6050_buffer[7]); //tempreature
	
}


////写格式转换函数
//void MPU6050_WriteReg(u8 reg_add,u8 reg_dat)
//{
//	I2C_MPU6050_ByteWrite(reg_dat,reg_add);
//}

////读格式转换函数
//void MPU6050_ReadData(u8 reg_add,unsigned char* Read,u8 num)
//{
//	I2C_MPU6050_BufferRead(Read,reg_add,num);
//}

////此段函数是拷贝来的，只能保证传感器工作，不能保证数据准确性
////启动时没有自检，缺乏校准
//void MPU6050_Init(void)
//{
//	//在初始化之前要延时一段时间，若没有延时，则断电后再上电数据可能会出错
//	Delay_ms(500);
//		
//	MPU6050_WriteReg(MPU6050_RA_PWR_MGMT_1, 0x00);	     //解除休眠状态 关闭循环模式 启用温度传感器 使用内部8M晶振
//	MPU6050_WriteReg(MPU6050_RA_SMPLRT_DIV , 0x07);	    //陀螺仪采样率 设置陀螺仪输出分频，从而得到采样率 
//														//采样率 = 陀螺仪输出频率 / (1 + SMPLRT_DIV)
//	MPU6050_WriteReg(MPU6050_RA_CONFIG , 0x06);			//配置5Hz带宽的数字低通滤波
//	MPU6050_WriteReg(MPU6050_RA_ACCEL_CONFIG , 0x01);	  //配置加速度传感器工作在2G模式
//	MPU6050_WriteReg(MPU6050_RA_GYRO_CONFIG, 0x18);     //陀螺仪自检及测量范围，典型值：0x18(不自检，2000deg/s)
//}

////读器件地址
//unsigned char MPU6050ReadID(void)
//{
//	unsigned char Re = 0;
//    MPU6050_ReadData(MPU6050_RA_WHO_AM_I,&Re,1);    //读器件地址
////     printf("%d\r\n",Re);
//	return Re;
//}

////这里的x，y，z是先写上去的，没有核实过，使用前一定要核实！！！
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

//void MPU6050ReadTemp(short *tempData)	//short 16位
//{
//	u8 buf[2];
//    MPU6050_ReadData(MPU6050_RA_TEMP_OUT_H,buf,2);     //读取温度值
//	
//    *tempData = (buf[0] << 8) | buf[1];		//简单的把2个8位拼成16位
//}

//void MPU6050_ReturnTemp(short*Temperature)	
//{
//	short temp3;
//	u8 buf[2];
//	
//	MPU6050_ReadData(MPU6050_RA_TEMP_OUT_H,buf,2);     //读取温度值
//	temp3= (buf[0] << 8) | buf[1];
//	
//	*Temperature=(((double) (temp3 + 13200)) / 280)-13;	//读取温度后进行了一个公式换算，暂时不确定换算结果的物理意义
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
