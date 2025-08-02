//=====================================================================
//�ļ����ƣ�isr.c���жϴ������Դ�ļ���
//����ṩ��SD-ARM��sumcu.suda.edu.cn��
//�汾���£�20170801-20191020
//�����������ṩ�жϴ�������̿��
//��ֲ���򣺡��̶���
//=====================================================================
#include "includes.h"
#include "string.h"
#include "rtc.h"

uint8_t CreateFrame(uint8_t Data,uint8_t * buffer);		//��֡��������
void User_SysFun(uint8_t ch);


uint8_t buff1[32];
uint8_t buff2[32];
uint16_t flag = 0;
//======================================================================
//�������ƣ�UART_User_Handler
//����������UART_User�����յ�һ���ֽڴ���
//��    ע�����뱾����󣬿�ʹ��uart_get_re_int�������ٽ����жϱ�־�ж�
// ��1-��UART�����жϣ�0-û��UART�����жϣ�
//======================================================================
void UART_User_Handler(void)
{
    uint8_t ch,flag;
    DISABLE_INTERRUPTS;      
       ch = uart_re1(UART_User, &flag);  
    if(flag)   
    {
        uart_send1(UART_User,ch);
    }  

    //��4.1����������֡ʹ�ã���ʼ)��
    if(CreateFrame(ch,gcRTCBuf))
	{   
        g_RTC_Flag=1;
	}
       //    ��������֡ʹ�ã�����)��
    ENABLE_INTERRUPTS;       
}
//�ڲ�����
void User_SysFun(uint8_t ch)
{
    //��1���յ���һ���ֽڲ�����֡
    if(gcRecvLen == 0)  gcRecvLen =useremuart_frame(ch,(uint8_t*)gcRecvBuf);
    //��2���ֽڽ�����֡���ж�gcRecvLen=0����Ϊ0����ʾ��֡��δ��ɣ�
    //     �´��յ�һ���ֽڣ��ټ�����֡
    if(gcRecvLen == 0) goto User_SysFun_Exit;
    //��3�����ˣ�gcRecvLen��0,��ʾ��֡��ɣ�gcRecvLenΪ֡�ĳ���,У�����кź���
    //     ����Flash�е���һ������ʼ��16�ֽڽ��бȽϣ�
    //     gcRecvBuf[16]������ת
    if(strncmp((char *)(gcRecvBuf),(char *)((MCU_SECTOR_NUM-1)*MCU_SECTORSIZE+
       MCU_FLASH_ADDR_START),16) != 0)
    {
        gcRecvLen = 0;         //�ָ�����״̬
        goto User_SysFun_Exit;
    }
    //��4�����ˣ������յ�����֡������űȽ�Ҳһ�£� ���������ֽ�gcRecvBuf[16]������ת
    //��ΪUser���ڳ�������������г������
    switch(gcRecvBuf[16])  //֡��ʶ
    {
        case 0:
            SYSTEM_FUNCTION((uint8_t *)(gcRecvBuf+17));
            gcRecvLen = 0;         //�ָ�����״̬
        break;
        default:
        break;
    }
User_SysFun_Exit:
    return;
}
//======================================================================
//�������ƣ�RTC_WKUP_IRQHandler
//������������
//�ж����ͣ�RTC���ӻ����жϴ�����
//======================================================================
 void RTC_WKUP_IRQHandler(void)
 {
 	 uint8_t hour,min,sec;
 	 uint8_t  year,month,date,week;
 	 char *p;
	 if(RTC_PeriodWKUP_Get_Int())         //�����жϵı�־
	 {
	 	 RTC_PeriodWKUP_Clear(); //��������жϱ�־
	 	 flag++;
	 	 RTC_Get_Date(&year,&month,&date,&week); //��ȡRTC��¼������
		 RTC_Get_Time(&hour,&min,&sec);    //��ȡRTC��¼��ʱ��
		 p=NumToStr("%02d/%02d/%02d %02d:%02d:%02d %d\n",year,month,date,hour,min,sec,week);
		 uart_send_string(UART_User,p);
		 printf("%02d/%02d/%02d %02d:%02d:%02d %d\n",year,month,date,hour,min,sec,week);
	 }


 }
//======================================================================
//�������ƣ�RTC_Alarm_IRQHandler
//�ж����ͣ�RTC�����жϴ�����
//======================================================================
void RTC_Alarm_IRQHandler(void)
{

	if(RTC_Alarm_Get_Int(A))            //����A���жϱ�־λ
	{
		RTC_Alarm_Clear(A);       //������A���жϱ�־λ
		printf("This is ALARM_A!!!\n");
	}
	if(RTC_Alarm_Get_Int(B))            //����A���жϱ�־λ
	{
		RTC_Alarm_Clear(B);       //������A���жϱ�־λ
		printf("This is ALARM_B!!!\n");
	}
	
}
//�ڲ����ú���
//===========================================================================
//�������ƣ�CreateFrame
//���ܸ�Ҫ���齨����֡��������֡���ݼ��뵽����֡��
//����˵����Data������֡����
//       buffer:����֡����
//�������أ���֡״̬    0-��֡δ�ɹ���1-��֡�ɹ�
//===========================================================================
uint8_t CreateFrame(uint8_t Data,uint8_t * buffer)
{
    static uint8_t frameLen=0;    //֡�ļ�����
    uint8_t frameFlag;            //��֡״̬

    frameFlag=0;            //��֡״̬��ʼ��
    //���ݾ�̬����frameLen��֡
    switch(frameLen)
    {
        case 0:    //��һ������
        {
            if (Data=='?')    //�յ�������֡ͷFrameHead
            {
                buffer[0]=Data;
                frameLen++;
                frameFlag=0;        //��֡��ʼ
            }
            break;
        }
        default:    //�������
        {
        	
            //������յ��Ĳ���֡β
            if(frameLen>=1 && Data!='!')
            {
                buffer[frameLen]=Data;
                frameLen++;
                break;
            }

            //����ĩβ��������֡�ɹ�
            if(Data=='!')
            {
                buffer[frameLen]=Data;
            	frameFlag=1;    //��֡�ɹ�
                frameLen=0;     //������0��׼��������֡
                break;
            }
        }
    }
    return frameFlag;                 //������֡״̬
}





