#include"AT24C02.h"


/*******************************************************************************
* 函 数 名         : Delay1us()
* 函数功能		   : 延时
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/

void I2C_Delay10us()
{
	uchar a, b;
	for(b=1; b>0; b--)
	{
		for(a=2; a>0; a--);
	}
}

/*******************************************************************************
* 函 数 名         : I2C_Start()
* 函数功能		   : 起始信号：在I2C_SCL时钟信号在高电平期间I2C_SDA信号产生一个下降沿
* 输    入         : 无
* 输    出         : 无
* 备    注         : 起始之后I2C_SDA和I2C_SCL都为0
*******************************************************************************/

void I2C_Start()
{
	I2C_SDA = 1;
	I2C_Delay10us();
	I2C_SCL = 1;
	I2C_Delay10us();//建立时间是I2C_SDA保持时间>4.7us
	I2C_SDA = 0;
	I2C_Delay10us();//保持时间是>4us
	I2C_SCL = 0;			
	I2C_Delay10us();		
}
/*******************************************************************************
* 函 数 名           : I2C_Stop()
* 函数功能	         : 终止信号：在I2C_SCL时钟信号高电平期间I2C_SDA信号产生一个上升沿
* 输    入           : 无
* 输    出         	 : 无
* 备    注           : 结束之后保持I2C_SDA和I2C_SCL都为1；表示总线空闲
*******************************************************************************/

void I2C_Stop()
{
	I2C_SDA = 0;
	I2C_Delay10us();
	I2C_SCL = 1;
	I2C_Delay10us();//建立时间大于4.7us
	I2C_SDA = 1;
	I2C_Delay10us();		
}
/*******************************************************************************
* 函 数 名           : I2cSendByte(uchar num)
* 函数功能 	         : 通过I2C发送一个字节。在I2C_SCL时钟信号高电平期间，
*                    * 保持发送信号I2C_SDA保持稳定
* 输    入           : num ,ack
* 输    出         	 : 0或1。发送成功返回1，发送失败返回0
* 备    注           : 发送完一个字节I2C_SCL=0, 需要应答则应答设置为1，否则为0
*******************************************************************************/

uchar I2C_SendByte(uchar dat, uchar ack)
{
	uchar a = 0,b = 0;//最大255，一个机器周期为1us，最大延时255us。
			
	for(a=0; a<8; a++)//要发送8位，从最高位开始
	{
		I2C_SDA = dat >> 7;	 //起始信号之后I2C_SCL=0，所以可以直接改变I2C_SDA信号
		dat = dat << 1;
		I2C_Delay10us();
		I2C_SCL = 1;
		I2C_Delay10us();//建立时间>4.7us
		I2C_SCL = 0;
		I2C_Delay10us();//时间大于4us		
	}

	I2C_SDA = 1;
	I2C_Delay10us();
	I2C_SCL = 1;
	while(I2C_SDA && (ack == 1))//等待应答，也就是等待从设备把I2C_SDA拉低
	{
		b++;
		if(b > 200)	 //如果超过200us没有应答发送失败，或者为非应答，表示接收结束
		{
			I2C_SCL = 0;
			I2C_Delay10us();
			return 0;
		}
	}

	I2C_SCL = 0;
	I2C_Delay10us();
 	return 1;		
}
/*******************************************************************************
* 函 数 名           : I2cReadByte()
* 函数功能	    	 : 使用I2c读取一个字节
* 输    入           : 无
* 输    出         	 : dat
* 备    注           : 接收完一个字节I2C_SCL=0
*******************************************************************************/

uchar I2C_ReadByte()
{
	uchar a = 0,dat = 0;
	I2C_SDA = 1;			//起始和发送一个字节之后I2C_SCL都是0
	I2C_Delay10us();
	for(a=0; a<8; a++)      //接收8个字节
	{
		I2C_SCL = 1;
		I2C_Delay10us();
		dat <<= 1;
		dat |= I2C_SDA;
		I2C_Delay10us();
		I2C_SCL = 0;
		I2C_Delay10us();
	}
	return dat;		
}
/*******************************************************************************
* 函 数 名         : void At24c02Write(uchar addr,uchar *Ptr)
* 函数功能		   : 依次往AT24C02中写入8个数据
* 输    入         : 写入的起始位置地址addr,要写入的数组名
* 输    出         : 无
*******************************************************************************/

void At24c02Write(uchar addr,uchar *Ptr)
{
    
	for(;addr<8;addr++)
	{
		I2C_Start();
		I2C_SendByte(0xa0, 1); //发送写器件地址
		I2C_SendByte(addr, 1); //发送要写入内存地址
		I2C_SendByte(*Ptr, 0); //发送数据
		I2C_Stop();
		Ptr++;
		Delayms(200);   //延时必要
	}
}
/*******************************************************************************
* 函 数 名         : unsigned char At24c02Read(uchar addr,uchar *Ptr)
* 函数功能		   : 依次读取AT24C02的8个数据
* 输    入         : 读AT24C02的起始位置地址addr,读出的数据要存入的数组名
* 输    出         : 无
*******************************************************************************/

void At24c02Read(uchar addr,uchar *Ptr)
{
	for(;addr<8;addr++)
	{
		I2C_Start();
		I2C_SendByte(0xa0, 1); //发送写器件地址
		I2C_SendByte(addr, 1); //发送要读取的地址
		I2C_Start();
		I2C_SendByte(0xa1, 1); //发送读器件地址
		*Ptr=I2C_ReadByte();    //读取数据
		I2C_Stop();
		Ptr++;
		Delayms(200);  //延时必要
	}
}


/*******************************************************************************
* 函 数 名         : SerialInit()
* 函数功能		   : 串口初始化
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/
//void SerialInit()
//{	 
//	SCON=0X50;			//设置为工作方式1
//	TMOD=0X20;			//设置计数器工作方式2
//	PCON=0X80;			//波特率加倍
//	TH1=0XF3;		    //计数器初始值设置，注意波特率是4800的
//	TL1=0XF3;
//	TR1=1;					    //打开计数器
//}

/*******************************************************************************
* 函 数 名         : SendToPc(uchar data)
* 函数功能		   : 串口发送数据到上位机
* 输    入         : 要发送的数据
* 输    出         : 无
*******************************************************************************/
//void SendToPc(uchar dat)
//{
//	SBUF=dat;
//	while(!TI);	 //当串口发送中断标志位TI置一说明一帧信息已经发送完毕，可以准备下一次发送了
//	TI=0;
//}

