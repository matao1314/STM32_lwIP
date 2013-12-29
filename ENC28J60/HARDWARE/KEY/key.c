//PB0 - key1
//PB1 - key2
#include "stm32f10x.h"

#include "key.h" 
#include "delay.h"	

 /*
 * ��������Key_GPIO_Config
 * ����  �����ð����õ���I/O��
 * ����  ����
 * ���  ����
 */
void KEY_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

	/*���������˿ڣ�PB0����ʱ��*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);

 	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0|GPIO_Pin_1; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz; 
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU; 

	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

 /*
 * ��������Key_Scan
 * ����  ������Ƿ��а�������
 * ����  ��GPIOx��x ������ A��B��C��D���� E
 *		     GPIO_Pin������ȡ�Ķ˿�λ 	
 * ���  ��KEY_OFF(û���°���)��KEY_ON�����°�����
 */
u8 KEY_Scan(void)
{			
u8 KEY1,KEY2;
static u8 key_up=1;//�������ɿ���־	
KEY1=GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0);
KEY2=GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1);
if(key_up&&(KEY1==0||KEY2==0))
{
delay_ms(10);//ȥ���� 
key_up=0;
if(KEY1==0)
{
return 1;
}
else if(KEY2==0)
{
return 2;
}
}
else 
if(KEY1==1&&KEY2==1)
key_up=1; 	    
return 0;// �ް�������
}
