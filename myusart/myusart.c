#include "myusart.h"
#include "string.h"
#include "myErrorAndWorning.h"

//���ݰ���ʽ��0xAA 0x55 length type data
enum myStatesz_Type
{
    NEEDAA = 100,
    NEED55,
    NEEDLEN
};

enum myControl_Type
{
	myControl_Type_Pause = 0,
	myControl_Type_Run,
	myControl_Type_Restart,
};

#define MY_USART_BUF_SIZE_TRANSMIT 256
#define MY_USART_BUF_SIZE_RECEIVE 256

static struct
{
    volatile u32 transmitHead;
    volatile u32 transmirTail;
    volatile u32 receiveHead;
    volatile u32 receiveTail;
    volatile u8 transmitBuf[MY_USART_BUF_SIZE_TRANSMIT];
    volatile u8 receiveBuf[MY_USART_BUF_SIZE_RECEIVE];
}myUsart;

/*********************************************************************
*Function: MyUsart_Config
*Description: ��ʼ������USART1 RX->PA9 TX->PA10
*Description: ������ rate�� ����λ 8bits�� У��λ none�� ֹͣλ 1bit
*Description: ����USART1�ж����ȼ�
*Input: int rate ���ڲ�����
*Output:
*Return: void
*Others:
*Author: Spacelan
*Date: 2014-2-25
*********************************************************************/
void MyUsart_Config(int rate)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO,ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	USART_StructInit(&USART_InitStructure);
	USART_InitStructure.USART_BaudRate = rate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1,&USART_InitStructure);
	USART_Cmd(USART1,ENABLE);
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);


	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	//myUsart�ṹ����
	memset(&myUsart,0,sizeof(myUsart));
}

/*********************************************************************
*Function: MyUsart_SendPacket
*Description: ���ڷ������ݰ�
*Description: ��ʽ 0xaa 0x55 length type data
*Input: char packet_type ���ݰ�����
*Input: const void *data ����ָ��
*Output:
*Return:
*Others:
*Author: Spacelan
*Date: 2014-2-25
*********************************************************************/
void MyUsart_SendPacket(char packet_type,const void *data)
{
//�������  ֱ��define��
#define buf myUsart.transmitBuf
	memset(buf,0,MY_USART_BUF_SIZE_TRANSMIT);

    uint8_t length;

    buf[0] = 0xaa;
    buf[1] = 0x55;

    if (packet_type == PACKET_TYPE_ACCEL || packet_type == PACKET_TYPE_GYRO)
    {
        short *sdata = (short*)data;
        buf[2] = 7;
        buf[3] = packet_type;
        buf[4] = (char)(sdata[0] >> 8);
        buf[5] = (char)sdata[0];
        buf[6] = (char)(sdata[1] >> 8);
        buf[7] = (char)sdata[1];
        buf[8] = (char)(sdata[2] >> 8);
        buf[9] = (char)sdata[2];
        length = 10;
    }
    else if (packet_type == PACKET_TYPE_QUAT)
    {
        long *ldata = (long*)data;
        buf[2] = 17;
        buf[3] = packet_type;
        buf[4] = (char)(ldata[0] >> 24);
        buf[5] = (char)(ldata[0] >> 16);
        buf[6] = (char)(ldata[0] >> 8);
        buf[7] = (char)ldata[0];
        buf[8] = (char)(ldata[1] >> 24);
        buf[9] = (char)(ldata[1] >> 16);
        buf[10] = (char)(ldata[1] >> 8);
        buf[11] = (char)ldata[1];
        buf[12] = (char)(ldata[2] >> 24);
        buf[13] = (char)(ldata[2] >> 16);
        buf[14] = (char)(ldata[2] >> 8);
        buf[15] = (char)ldata[2];
        buf[16] = (char)(ldata[3] >> 24);
        buf[17] = (char)(ldata[3] >> 16);
        buf[18] = (char)(ldata[3] >> 8);
        buf[19] = (char)ldata[3];
        length = 20;
    }
    MyUsart_Send(buf,length);
#undef buf
}

/*********************************************************************
*Function: MyUsart_Send
*Description: ����USART_SendData��������buf
*Input: const u8 *buf ���ݻ���
*Input: u8 len ���泤��
*Output:
*Return:
*Others:
*Author: Spacelan
*Date: 2014-2-25
*********************************************************************/
void MyUsart_Send(const u8 *buf,u8 len)
{
	u8 i;
	for(i=0;i<len;i++)
	{
		USART_SendData(USART1,buf[i]);
		while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	}
}

/*********************************************************************
*Function: HandleOneByte
*Description: ����һ�ֽ����ݣ��жϴ��ֽ���ʲô��ʾʲô
*Description: ��������һ����������֡ͬ������״̬��
*Input: u8 byte �账����ֽ�
*Output:
*Return:
*Others:
*Author: Spacelan
*Date: 2014-2-25
*********************************************************************/
void HandleOneByte(u8 byte)
{
#define _MAX_BUF_LENGTH_ (20)
	static u8 len,buf[_MAX_BUF_LENGTH_],state = NEEDAA;
	switch(state)
	{
	case NEEDAA:
		if(byte == 0xaa)
			state = NEED55;
		break;
	case NEED55:
		if(byte == 0x55)
			state = NEEDLEN;
		break;
	case NEEDLEN:
		len = byte;
		memset(buf,0,_MAX_BUF_LENGTH_);
		state = 0;
		break;
	}

	if(state < NEEDAA)
	{
		buf[state] = byte;
		state++;
		if(state == len)
		{
			FrameCompleted(buf,len);
			state = NEEDAA;
		}
	}
}

/*********************************************************************
*Function: FrameCompleted
*Description: һ֡���ݽ�����ɣ���������
*Input: const u8 *buf һ֡���ݣ�type+data
*Input: u8 len ���ݳ���
*Output:
*Return:
*Others:
*Author: Spacelan
*Date: 2014-2-25
*********************************************************************/
void FrameCompleted(const u8 *buf,u8 len)
{
	u8 type = buf[0];
	buf++;
	extern int myState;

	switch(type)
	{
	case PACKER_TYPE_RUN:
		myState = myControl_Type_Run;
		break;
	case PACKET_TYPE_PAUSE:
		myState = myControl_Type_Pause;
		break;
	case PACKET_TYPE_RESTART:
		myState = myControl_Type_Restart;
		break;
	}
}

/*********************************************************************
*Function: USART1_IRQHandler
*Description: USART1�жϴ��������������ջ��岻Ϊ�ա��ж�
*Description: �����յ���һ�ֽ����ݴ���receiveBuf
*Input:
*Output:
*Return:
*Others:
*Author: Spacelan
*Date: 2014-2-25
*********************************************************************/
void USART1_IRQHandler(void)
{
	u8 byte;
	if(USART_GetITStatus(USART1,USART_IT_RXNE) == SET)
	{
		USART_ITConfig(USART1,USART_IT_RXNE,DISABLE);
		byte = USART_ReceiveData(USART1);
/*		switch(byte)
		{
		case 'r':
			myState = myControl_Type_Run;
			break;
		case 'p':
			myState = myControl_Type_Pause;
			break;
		case 's':
			myState = myControl_Type_Restart;
			break;
		}
*/
		USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
	}
}


