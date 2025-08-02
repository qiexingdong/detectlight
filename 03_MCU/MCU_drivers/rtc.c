//==========================================================================
//�ļ����ƣ�rtc.c
//���ܸ�Ҫ��STM32L431RC RTC�ײ���������Դ�ļ�
//��Ȩ���У����ݴ�ѧ��˼����Ƕ��ʽ����(sumcu.suda.edu.cn)
//==========================================================================
#include "rtc.h"

// ==============================�ڲ���������=================================
void Delay_ms(uint16_t u16ms);
uint8_t RTC_DEC2BCD(uint8_t val);
uint8_t RTC_BCD2DEC(uint8_t val);
uint8_t RTC_Init_Mode(void);
// ==========================================================================
// �������ƣ�RTC_Init
// ������������
// �������أ�0,��ʼ���ɹ���1,�����ʼ��ʧ��
// ���ܸ�Ҫ��ѡ��LSEʱ��,Ƶ��Ϊ32.768kHz;��7λ�첽Ԥ��Ƶ��Ϊ128,15λͬ��Ԥ��Ƶ��Ϊ256;����ʼ��ʱ��
// ==========================================================================
uint8_t RTC_Init(void)
{
	RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN_Msk;		    //ʹ�ܵ�Դ�ӿ�ʱ��
	PWR->CR1 |= PWR_CR1_DBP_Msk;     //���������ʹ��(RTC+SRAM)
	RCC->CSR |= RCC_CSR_LSION_Msk;				//LSI����ʹ��
	while(!(RCC->CSR&0x02)) ;	    //�ȴ�LSI����
	RCC->BDCR &= ~(RCC_BDCR_RTCSEL_Msk);             //����8/9λ
	RCC->BDCR |= 1<<9;              //ѡ��LSIʱ��
	RCC->BDCR |= RCC_BDCR_RTCEN_Msk;             //ʹ��RTCʱ��
	RTC->WPR = 0xCA;                //�ر�RTC�Ĵ���д����
	RTC->WPR = 0x53;
	RTC->CR = 0;
	if(RTC_Init_Mode())              //����RTC��ʼ��ģʽʧ��
	{
		RCC->BDCR = RCC_BDCR_BDRST_Msk;//��λBDCR
		Delay_ms(10);
		RCC->BDCR = 0;			    //������λ
		return 1;
	}
	RTC->PRER = 0XFF;			    //RTCͬ����Ƶϵ��(0~7FFF),����������ͬ����Ƶ,�������첽��Ƶ
	RTC->PRER |= 0X7F<<16;		    //RTC�첽��Ƶϵ��(1~0X7F)
	RTC->CR &= ~(RTC_CR_FMT_Msk);   //24h��
	RTC->ISR &= ~(RTC_ISR_INIT_Msk);//�˳�RTC��ʼ��ģʽ
	RTC->WPR = 0XFF;                //ʹ��RTC�Ĵ���д����
	return 0;
}

// ===========================================================================
// �������ƣ�RTC_Set_Date
// ����������year:���;month:�·�;date:����;week:���ڼ�
// �������أ�1:�������ڳɹ�;0:��������ʧ��
// ���ܸ�Ҫ������RTCʱ�ӵ�����
// ===========================================================================
uint8_t RTC_Set_Date(uint8_t year,uint8_t month,uint8_t date,uint8_t week)
{
	uint32_t temp = 0;
	RTC->WPR = 0xCA;			                   //�ر�RTC�Ĵ���д����
	RTC->WPR = 0x53;
	if(RTC_Init_Mode())                            //����RTC��ʼ��ģʽʧ��
		return 1;
	temp = (((uint32_t)week&0X07)<<13)|((uint32_t)RTC_DEC2BCD(year)<<16)|((uint32_t)RTC_DEC2BCD(month)<<8)|(RTC_DEC2BCD(date));
	RTC->DR = temp;                                //����RTC�����ڼĴ�����ֵ
	RTC->ISR &= ~(RTC_ISR_INIT_Msk);			               //�˳�RTC��ʼ��ģʽ
	return 0;
}

// ===========================================================================
// �������ƣ�RTC_Set_Time
// ����������hour:Сʱ;min:����;sec:����;
// �������أ�1:����ʱ��ɹ�;0:����ʱ��ʧ��
// ���ܸ�Ҫ������RTCʱ�ӵ�ʱ��
// ===========================================================================
uint8_t RTC_Set_Time(uint8_t hour,uint8_t min,uint8_t sec)
{
	uint32_t temp = 0;
	RTC->WPR = 0xCA;                               //�ر�RTC�Ĵ���д����
	RTC->WPR = 0x53;
	if(RTC_Init_Mode())                           //����RTC��ʼ��ģʽʧ��
		return 1;
	temp = (((uint32_t)0&0X01)<<22)|((uint32_t)RTC_DEC2BCD(hour)<<16)|((uint32_t)RTC_DEC2BCD(min)<<8)|(RTC_DEC2BCD(sec));
	RTC->TR = temp;                                //����RTC��ʱ��Ĵ�����ֵ
	RTC->ISR &= ~(RTC_ISR_INIT_Msk);			   //�˳�RTC��ʼ��ģʽ
	return 0;
}

// ===========================================================================
// �������ƣ�RTC_Get_Date
// ����������year:���;month:�·�;date:����;week:���ڼ�
// �������أ���
// ���ܸ�Ҫ����ȡRTCʱ�ӵ�����
// ===========================================================================
void RTC_Get_Date(uint8_t *year,uint8_t *month,uint8_t *date,uint8_t *week)
{
	uint32_t temp = 0;
	while(RTC_Wait_Synchro());                      //�ȴ�ͬ�������ݼĴ�����
	temp = RTC->DR;
	*year = RTC_BCD2DEC((temp>>16)&0XFF);
	*month = RTC_BCD2DEC((temp>>8)&0X1F);
	*date = RTC_BCD2DEC(temp&0X3F);
	*week = (temp>>13)&0X07;
}

// ===========================================================================
// �������ƣ�RTC_Get_Time
// ����������hour:Сʱ;min:����;sec:����;
// �������أ���
// ���ܸ�Ҫ����ȡRTCʱ�ӵ�ʱ��
// ===========================================================================
void RTC_Get_Time(uint8_t *hour,uint8_t *min,uint8_t *sec)
{
	uint32_t temp = 0;
	while(RTC_Wait_Synchro());                      //�ȴ�ͬ�������ݼĴ�����
	temp = RTC->TR;
	*hour = RTC_BCD2DEC((temp>>16)&0X3F);
	*min = RTC_BCD2DEC((temp>>8)&0X7F);
	*sec = RTC_BCD2DEC(temp&0X7F);
	//*ampm = temp>>22;
}

// ==========================================================================
// �������ƣ�RTC_Set_Alarm
// ����������SelAlarm��0������A��1������B;week:������;hour:Сʱ;min:����;sec:����
// �������أ���
// ���ܸ�Ҫ���������ӵ�ʱ��
// ==========================================================================
void RTC_Set_Alarm(uint8_t SelAlarm,uint8_t week,uint8_t hour,uint8_t min,uint8_t sec)
{
    if(SelAlarm!=0&&SelAlarm!=1) SelAlarm=0;                 //������Ĭ��ѡ������A
    RTC->WPR = 0xCA;                                         //�ر�RTC�Ĵ���д����
	RTC->WPR = 0x53;
    if(SelAlarm==0) //����A
    {
		RTC->CR &= ~(RTC_CR_ALRAE_Msk);                          //�ر�����A
		while((RTC->ISR & 0x01)==0) ;                            //�ȴ�����A�޸�����
		RTC->ALRMAR = 0;                                         //�������A�Ĵ���������
		RTC->ALRMAR |= RTC_ALRMAR_WDSEL_Msk;                     //����������
		RTC->ALRMAR |= 0<<RTC_ALRMAR_PM_Pos;                      //24Сʱ��ʽ
		RTC->ALRMAR |= (uint32_t)RTC_DEC2BCD(week)<<RTC_ALRMAR_DU_Pos;       //��������
		RTC->ALRMAR |= (uint32_t)RTC_DEC2BCD(hour)<<RTC_ALRMAR_HU_Pos;       //Сʱ����
		RTC->ALRMAR |= (uint32_t)RTC_DEC2BCD(min)<<RTC_ALRMAR_MNU_Pos;	     //��������
		RTC->ALRMAR |= (uint32_t)RTC_DEC2BCD(sec);		         //��������
		RTC->ALRMASSR = 0;						                 //��ʹ��SUB SEC
	
		RTC->CR |= RTC_CR_ALRAE_Msk;                             //��������A
		RTC->WPR = 0XFF;		                                 //��ֹ�޸�RTC�Ĵ���
		RTC->ISR &= ~(RTC_ISR_ALRAF_Msk);                        //���RTC����A�ı�־
		EXTI->PR1 = EXTI_PR1_PIF18_Msk;                          //���LINE18�ϵ��жϱ�־λ ,д1��0
		EXTI->IMR1 |= EXTI_IMR1_IM18_Msk;                        //����LINE18�ϵ��ж�
		EXTI->RTSR1 |= EXTI_RTSR1_RT18_Msk;                      //�����ش���
	}
	else if(SelAlarm==1) //����B
	{
		RTC->CR &= ~(RTC_CR_ALRBE_Msk);                          //�ر�����B
		while((RTC->ISR & 0x02)==0) ;                            //�ȴ�����B�޸�����
		RTC->ALRMBR = 0;                                         //�������B�Ĵ���������
		RTC->ALRMBR |= RTC_ALRMBR_WDSEL_Msk;                     //����������
		RTC->ALRMBR |= 0<<RTC_ALRMBR_PM_Pos;                      //24Сʱ��ʽ
		RTC->ALRMBR |= (uint32_t)RTC_DEC2BCD(week)<<RTC_ALRMBR_DU_Pos;       //��������
		RTC->ALRMBR |= (uint32_t)RTC_DEC2BCD(hour)<<RTC_ALRMBR_HU_Pos;       //Сʱ����
		RTC->ALRMBR |= (uint32_t)RTC_DEC2BCD(min)<<RTC_ALRMBR_MNU_Pos;	     //��������
		RTC->ALRMBR |= (uint32_t)RTC_DEC2BCD(sec);		         //��������
		RTC->ALRMBSSR = 0;						                 //��ʹ��SUB SEC
	
		RTC->CR |= RTC_CR_ALRBE_Msk;                             //��������B
		RTC->WPR = 0XFF;		                                 //��ֹ�޸�RTC�Ĵ���
		RTC->ISR &= ~(RTC_ISR_ALRBF_Msk);                        //���RTC����B�ı�־
		EXTI->PR1 = EXTI_PR1_PIF18_Msk;                          //���LINE18�ϵ��жϱ�־λ ,д1��0
		EXTI->IMR1 |= EXTI_IMR1_IM18_Msk;                        //����LINE18�ϵ��ж�
		EXTI->RTSR1 |= EXTI_RTSR1_RT18_Msk;                      //�����ش���
	}
}

// ==========================================================================
// �������ƣ�RTC_Set_PeriodWakeUp
// ����������rtc_s:�Զ����ѵ����ڣ���λΪ��
// �������أ���
// ���ܸ�Ҫ�������Զ����ѵ�����
// ==========================================================================
void RTC_Set_PeriodWakeUp(uint8_t rtc_s)
{
	RTC->WPR = 0xCA;                                          //�ر�RTC�Ĵ�������
	RTC->WPR = 0x53;
	RTC->CR &= ~(RTC_CR_WUTE_Msk);                            //�ر�WAKE UP
	while((RTC->ISR&0x04)==0);                                //�ȴ������޸Ķ�ʱ������
	RTC->CR &= ~(RTC_CR_WUCKSEL_Msk);                         //���ԭ����ʱ��ѡ��
	RTC->CR |=RTC_CR_WUCKSEL_2;                               //ѡ�� ck_spre ʱ�ӣ�ͨ��Ϊ 1 Hz��
	if(rtc_s>0)
	{
	    RTC->WUTR = rtc_s-1;                                    //�����Զ���װ�ؼĴ�����ֵ
	}
	else 
		RTC->WUTR=0;
	RTC->ISR &= ~(RTC_ISR_WUTF_Msk);		                   //���RTC WAKE UP�ı�־
	RTC->CR |= RTC_CR_WUTE_Msk;                                //ʹ�ܶ�ʱ��
	RTC->WPR = 0xFF;                                           //��ֹ�޸�RTC�Ĵ���
	EXTI->PR1 = EXTI_PR1_PIF20_Msk;                            //���LINE20�ϵ��жϱ�־λ��д1��0
	EXTI->IMR1 |= EXTI_IMR1_IM20_Msk;                          //����LINE20�ϵ��ж�
	EXTI->RTSR1 |= EXTI_RTSR1_RT20_Msk;                        //�����ش����ж�
}

// ==========================================================================
// �������ƣ�RTC_Alarm_Enable_Int
// ����������SelAlarm��0������A��1������B
// �������أ���
// ���ܸ�Ҫ��ʹ�������ж�
// ==========================================================================
void RTC_Alarm_Enable_Int(uint8_t SelAlarm)
{
    if(SelAlarm==0) //����A
    {
		RTC->CR |= RTC_CR_ALRAIE_Msk;                             //��������A�ж�
	}
	else if(SelAlarm==1) //����B
	{
		RTC->CR |= RTC_CR_ALRBIE_Msk;                             //��������B�ж�
	}
	NVIC_EnableIRQ(RTC_Alarm_IRQn);                          //ʹ������ģ���ж�
}

// ==========================================================================
// �������ƣ�RTC_PeriodWKUP_Enable_Int
// ������������
// �������أ���
// ���ܸ�Ҫ��ʹ���Զ������ж�
// ==========================================================================
void RTC_PeriodWKUP_Enable_Int()
{
	RTC->CR |= RTC_CR_WUTIE_Msk;                             //ʹ��WAKE UP��ʱ���ж�
	NVIC_EnableIRQ(RTC_WKUP_IRQn);                           //ʹ���Զ�����ģ���ж�
}

// ==========================================================================
// �������ƣ�RTC_PeriodWKUP_Disable_Int
// ������������
// �������أ���
// ���ܸ�Ҫ����ֹ�Զ������ж�
// ==========================================================================
void RTC_PeriodWKUP_Disable_Int()
{
	RTC->CR &= ~(RTC_CR_WUTIE_Msk);                           //�ر�WAKE UP��ʱ���ж�
	NVIC_DisableIRQ(RTC_WKUP_IRQn);                           //��ֹ�Զ������ж�
}

// ==========================================================================
// �������ƣ�RTC_Alarm_Disable_Int
// ����������SelAlarm��0������A��1������B;
// �������أ���
// ���ܸ�Ҫ����ֹ�����ж�
// ==========================================================================
void RTC_Alarm_Disable_Int(uint8_t SelAlarm)
{
	if(SelAlarm==0) //����A
    {
		RTC->CR &= ~(RTC_CR_ALRAIE_Msk);                           //�ر�����A�ж�
	}
	else if(SelAlarm==1) //����B
	{
		RTC->CR &= ~(RTC_CR_ALRBIE_Msk);                           //�ر�����B�ж�
	}
	NVIC_DisableIRQ(RTC_Alarm_IRQn);                          //��ֹ�����ж�
}

//=====================================================================
//�������ƣ�RTC_PeriodWKUP_Get_Int
//�������أ�1���л����жϣ�0��û�л����жϡ�
//����˵������
//���ܸ�Ҫ����ȡ�����жϱ�־��
//=====================================================================
uint8_t RTC_PeriodWKUP_Get_Int()
{
	//��ȡ��ʱ�������жϱ�־
	if(RTC->ISR&RTC_ISR_WUTF_Msk)
		return 1;
	else
		return 0;
}

//=====================================================================
//�������ƣ�RTC_PeriodWKUP_Clear
//�������أ���
//����˵������
//���ܸ�Ҫ����������жϱ�־��
//=====================================================================
void RTC_PeriodWKUP_Clear()
{
	//�����ʱ�������жϱ�־
	RTC->ISR &= ~RTC_ISR_WUTF_Msk;
	EXTI->PR1 |= EXTI_PR1_PIF20_Msk;          //���LINE20���жϱ�־
}

//=====================================================================
//�������ƣ�RTC_Alarm_Get_Int
//�������أ�1���������жϣ�0��û�������жϡ�
//����˵����SelAlarm��0������A��1������B
//���ܸ�Ҫ����ȡ�����жϱ�־��
//=====================================================================
uint8_t RTC_Alarm_Get_Int(uint8_t SelAlarm)
{
	if(SelAlarm==0)  //����A
	{
		if(RTC->ISR&RTC_ISR_ALRAF_Msk)
			return 1;
		else
			return 0;
	}
	else if(SelAlarm==1) //����B
	{
		if(RTC->ISR&RTC_ISR_ALRBF_Msk)
			return 1;
		else
			return 0;
	}
	else
		return 0;
}

//=====================================================================
//�������ƣ�RTC_Alarm_Clear
//�������أ���
//����˵����SelAlarm��0������A��1������B
//���ܸ�Ҫ����������жϱ�־��
//=====================================================================
void RTC_Alarm_Clear(uint8_t SelAlarm)
{

	if(SelAlarm==0)  //����A
	{
		RTC->ISR &= ~RTC_ISR_ALRAF_Msk;       //������A���жϱ�־λ
		EXTI->PR1 |= EXTI_PR1_PIF18_Msk;      //����ж���18���жϱ�־

	}
	else if(SelAlarm==1) //����B
	{
		RTC->ISR &= ~RTC_ISR_ALRBF_Msk;       //������B���жϱ�־λ
		EXTI->PR1 |= EXTI_PR1_PIF18_Msk;      //����ж���18���жϱ�־
	}
}


// ===========================================================================
// �������ƣ�RTC_Wait_Synchro
// ������������
// �������أ�1:����ʧ��;0:���ݳɹ���
// ���ܸ�Ҫ�� �������Ĵ����е�ֵ���ݵ����üĴ�����
// ===========================================================================
uint8_t RTC_Wait_Synchro(void)
{
	uint32_t retry = 0XFFFFF;
	RTC->WPR = 0xCA;                                 //�ر�RTC�Ĵ���д����
	RTC->WPR = 0x53;
	RTC->ISR &= ~(RTC_ISR_RSF_Msk);		                     //���RSFλ
	while(retry&&((RTC->ISR&(RTC_ISR_RSF_Msk))==0x00))       //�ȴ��뱸�ݼĴ���ͬ��
		retry--;
    if(retry==0)                                     //ͬ��ʧ��
    	return 1;
	RTC->WPR = 0xFF;			                     //ʹ��RTC�Ĵ���д����
	return 0;
}
//==============================�ڲ��������崦=================================
//======================================================================
//�������ƣ�Delay_us
//�������أ���
//����˵������
//���ܸ�Ҫ����ʱ - ���뼶
//======================================================================
void Delay_ms(uint16_t u16ms)
{
	uint32_t u32ctr;
	for(u32ctr = 0; u32ctr < (48000*u16ms); u32ctr++)
	{
		__asm ("NOP");
	}
}

// ===========================================================================
// �������ƣ�RTC_DEC2BCD
// ����������ʮ������
// �������أ�ʮ��������Ӧ��BCD���ʽ
// ���ܸ�Ҫ����ʮ������ת��Ϊ��Ӧ��BCD���ʽ
// ===========================================================================
uint8_t RTC_DEC2BCD(uint8_t val)
{
	uint8_t bcdhigh = 0;
	while(val>=10)
	{
		bcdhigh++;
		val -= 10;
	}
	return ((uint8_t)(bcdhigh<<4)|val);
}

// ===========================================================================
// �������ƣ�RTC_BCD2DEC
// ����������BCD���ʽ����
// �������أ�BCD���Ӧ��ʮ������
// ���ܸ�Ҫ����BCD���ʽת��Ϊ��Ӧ��ʮ������
// ===========================================================================
uint8_t RTC_BCD2DEC(uint8_t val)
{
	uint8_t temp = 0;
	temp = (val>>4)*10;
	return (temp+(val&0X0F));
}

// ===========================================================================
// �������ƣ�RTC_Init_Mode
// ������������
// �������أ�0�������ʼ��ģʽ�ɹ���1�������ʼ��ģʽʧ��
// ���ܸ�Ҫ���������ʼ��ģʽʧ�ܣ�������RTCģ���ʼ��
// ===========================================================================
uint8_t RTC_Init_Mode(void)
{
	uint32_t retry = 0XFFFFF;
	if(RTC->ISR&(RTC_ISR_INITF_Msk))
		return 0;
	RTC->ISR |= RTC_ISR_INIT_Msk;	                               //����RTC��ʼ��ģʽ
	while(retry&&((RTC->ISR&(RTC_ISR_INITF_Msk))==0x00))            //�ȴ�����RTC��ʼ��ģʽ�ɹ�
		retry--;
    if(retry==0)
    	return 1;	                                   //ͬ��ʧ��
	else
		return 0; 			                           //ͬ���ɹ�
}

