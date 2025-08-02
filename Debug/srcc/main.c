#define GLOBLE_VAR
#include "includes.h"      
#include "string.h"

#include "light.h"
#include "lightsensor.h" 
#include "button.h"
#include "buzz.h"
#include "OLED.h"
#include "OLED_Data.h"
#include "rtc.h"
#include "buzz.h"
#include "NumToStr.h"

extern uint16_t flag;
double a ;
double b ;
double num1;
double num2;
int main(void)
{
    uint8_t mK1[32];	 
    uint8_t mK2[32];
    char *p;
    uint8_t Flag_1 = 'Y',Flag_2 = 'Y';
    uint16_t num_AD,tmpAD;
    DISABLE_INTERRUPTS;
    uint8_t hour,min,sec;
    flash_erase(49);   
    Light_Init();
    LightSensor_Init();
    Buzz_Init();
    RTC_Init();         //RTC��ʼ��
    RTC_Set_Time(3,59,45);         //����ʱ��Ϊ3:59:45
    RTC_Set_Date(0,0,0,0);  //���������꣬�£���
    uart_init(UART_User,115200);     //��ʼ������ģ�� 
    //��ʼ��OLED
    OLED_Init();
    OLED_Update();
    //ʹ��
    RTC_PeriodWKUP_Enable_Int(); 
    uart_enable_re_int(UART_User);  //ʹ��UART_USERģ������жϹ���
    ENABLE_INTERRUPTS;
    RTC_Set_PeriodWakeUp(1); 
    while(1)
    
    {
        if(g_RTC_Flag==1) //���ݴ��ڽ��յ��������û�׼ʱ��
        {
            g_RTC_Flag=0;
            gcRTC_Date_Time.Year=(uint8_t)((gcRTCBuf[1]-'0')*10+(gcRTCBuf[2]-'0'));
            gcRTC_Date_Time.Month=(uint8_t)((gcRTCBuf[4]-'0')*10+(gcRTCBuf[5]-'0'));
            gcRTC_Date_Time.Date=(uint8_t)((gcRTCBuf[7]-'0')*10+(gcRTCBuf[8]-'0'));
            gcRTC_Date_Time.Hours=(uint8_t)((gcRTCBuf[10]-'0')*10+(gcRTCBuf[11]-'0'));
            gcRTC_Date_Time.Minutes=(uint8_t)((gcRTCBuf[13]-'0')*10+(gcRTCBuf[14]-'0'));
            gcRTC_Date_Time.Seconds=(uint8_t)((gcRTCBuf[16]-'0')*10+(gcRTCBuf[17]-'0'));
            gcRTC_Date_Time.Weekday=(uint8_t)((gcRTCBuf[23]-'0'));   
            RTC_Set_Time(gcRTC_Date_Time.Hours,gcRTC_Date_Time.Minutes,gcRTC_Date_Time.Seconds);         //����ʱ��
            RTC_Set_Date(gcRTC_Date_Time.Year,gcRTC_Date_Time.Month,gcRTC_Date_Time.Date,gcRTC_Date_Time.Weekday);  //��������
        }
        RTC_Get_Time(&hour,&min,&sec); 
        OLED_ClearArea(0,0,120,16);
        OLED_ShowString(0,0,"Time:",OLED_8X16);
        OLED_ShowNum(40,0,hour,2,OLED_8X16);
        OLED_ShowNum(60,0,min,2,OLED_8X16);
        OLED_ShowNum(80,0,sec,2,OLED_8X16);
        OLED_UpdateArea(0,0,120,16);
        //��flash�ĵ�49�������뵱ǰ����
        if(LightSensor_Get() == 0 && Flag_1 == 'Y')
        {
            flash_erase(49);  
            flash_write(49,0,32,(uint8_t *) "light"); 	
            Flag_1 = 'N';
        }
        else if(LightSensor_Get() == 1 && Flag_1 == 'N')
        {
            flash_erase(49);
            flash_write(49,0,32,(uint8_t *) "no light");   
            Flag_1 = 'Y';
        }  
        //ÿ��5�뿪ʼ����oled
        if(flag % 5 == 0)
        { 
            //��ȡ����
            flash_read_logic(mK1,49,0,32);
            if(LightSensor_Get() == 0)
            {
                Light_1_Off();
                Buzz_Off();
                a = a+1;
            }
            else 
            {
                Light_1_On();
                Buzz_On();
                b =b+1;
            }
            OLED_ClearArea(0,20,120,16);
            OLED_ShowString(0,20,mK1,OLED_8X16);
            OLED_UpdateArea(0,20,120,16);
            if(hour == 4 && min == 0 && sec == 0)
            {
                num1 = a/(a+b);
                num2 = b/(a+b);
                OLED_ClearArea(0,40,120,16);
                OLED_ShowFloatNum(0,40,num1*100,2,2,OLED_8X16);
                OLED_ShowFloatNum(60,40,num2*100,2,2,OLED_8X16);
                OLED_ShowString(50,40,"%",OLED_8X16);
                OLED_ShowString(110,40,"%",OLED_8X16);
                OLED_UpdateArea(0,40,120,16);
                p=NumToStr("�����й��հٷֱȣ�%.2f \n",num1*100);
                uart_send_string(UART_User,p);
                p=NumToStr("�����޹��հٷֱȣ�%.2f \n",num2*100);
                uart_send_string(UART_User,p);
                Delay_ms(200);
            }
        }
    }   
}        






