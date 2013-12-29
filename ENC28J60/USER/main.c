#include "led.h"
#include "delay.h"
#include "key.h"
#include "mysys.h"
#include "lcd.h"
#include "usart.h"	 
#include "led.h"
#include "delay.h"
#include "key.h"
#include "mysys.h"
#include "lcd.h"
#include "usart.h"	 
#include "rtc.h" 
#include "adc.h"
#include "enc28j60.h"			   
#include "math.h" 	
#include "string.h"	 

#include "lwip/init.h"
#include "lwip/ip.h"
#include "lwip/dhcp.h"
#include "lwip/tcp_impl.h"
#include "lwip/ip_frag.h"
#include "netif/etharp.h"
#include "netif/ethernetif.h"
#include "arch/sys_arch.h"

#include "lwip_demo.h"

#define CLOCKTICKS_PER_MS 10    //����ʱ�ӽ���


static ip_addr_t ipaddr, netmask, gw; //����IP��ַ
struct netif enc28j60_netif;  //��������ӿ�
u32_t last_arp_time;			
u32_t last_tcp_time;	
u32_t last_ipreass_time;


u32_t input_time;

#if LWIP_DHCP>0	
u32_t last_dhcp_fine_time;			
u32_t last_dhcp_coarse_time;  
u32 dhcp_ip=0;
#endif




//LWIP��ѯ
void LWIP_Polling(void){
	
	// if(timer_expired(&input_time,5)) //���հ������ڴ�����
 // {
    ethernetif_input(&enc28j60_netif); 
 // }
  if(timer_expired(&last_tcp_time,TCP_TMR_INTERVAL/CLOCKTICKS_PER_MS))//TCP����ʱ��������
  {
	 tcp_tmr();
  }
  if(timer_expired(&last_arp_time,ARP_TMR_INTERVAL/CLOCKTICKS_PER_MS))//ARP����ʱ��
  {
	etharp_tmr();
  }
	if(timer_expired(&last_ipreass_time,IP_TMR_INTERVAL/CLOCKTICKS_PER_MS)){ //IP������װ��ʱ��
		ip_reass_tmr();
  }
  #if LWIP_DHCP>0			   					
  if(timer_expired(&last_dhcp_fine_time,DHCP_FINE_TIMER_MSECS/CLOCKTICKS_PER_MS))
  {
	 dhcp_fine_tmr();
  }
  if(timer_expired(&last_dhcp_coarse_time,DHCP_COARSE_TIMER_MSECS/CLOCKTICKS_PER_MS))
  {
	dhcp_coarse_tmr();
  }  
  #endif
  
}

//��ʾ����״̬
void show_connect_status(void){
		if((lwip_flag&LWIP_CONNECTED)==LWIP_CONNECTED){		//���ӳɹ���
			switch(lwip_test_mode){
					case LWIP_TCP_SERVER:
						  LCD_ShowString(30,180,200,16,16,"TCP Server Connected   ");
							break;
					case LWIP_TCP_CLIENT:
						   LCD_ShowString(30,180,200,16,16,"TCP Client Connected   ");
							break;
					case LWIP_UDP_SERVER:
						  LCD_ShowString(30,180,200,16,16,"UDP Server Connected   ");
							break;
					case LWIP_UDP_CLIENT:
						   LCD_ShowString(30,180,200,16,16,"UDP Client Connected   ");
							break;					
					default:break;
			}
		}else{
					switch(lwip_test_mode){
					case LWIP_TCP_SERVER:
						  LCD_ShowString(30,180,200,16,16,"TCP Server Disconnected   ");
							break;
					case LWIP_TCP_CLIENT:
						   LCD_ShowString(30,180,200,16,16,"TCP Client Disconnected   ");
							break;
					case LWIP_UDP_SERVER:
						  LCD_ShowString(30,180,200,16,16,"UDP Server Connected   ");
							break;
					case LWIP_UDP_CLIENT:
						   LCD_ShowString(30,180,200,16,16,"UDP Client Connected   ");
							break;	
					case LWIP_WEBSERVER:
						  LCD_ShowString(30,180,200,16,16,"Webserver   "); 
						  break;
					default:break;
			}
		}
}

//ѡ��ģʽ
void select_lwip_mode(void){
  u16 times=0;
	u8 key=0;
	u8 mode=0;
	u8 flag=0;
	POINT_COLOR = BLUE;
	do{
		key=KEY_Scan();
		if(key==KEY_UP){	
			mode++;
			switch(mode%5){
				case 0: 
					lwip_test_mode = LWIP_TCP_SERVER;
					break;				
				case 1: 
					lwip_test_mode = LWIP_TCP_CLIENT;
					break;
				case 2: 
					lwip_test_mode = LWIP_UDP_SERVER;
					break;		
				case 3: 
					lwip_test_mode = LWIP_UDP_CLIENT;
					break;	
				case 4: 
					lwip_test_mode = LWIP_WEBSERVER;
					break;					
				default:break;
			}
		}
		delay_ms(1);
		times++;
		if((times%500==0)){
			if(!flag){
				show_connect_status();
			}else{
				LCD_ShowString(30,180,200,16,16,"                              ");   
			}
			flag = !flag;
		}
 
	}while(key!=KEY_DOWN);
	show_connect_status();
}

int main(void)
 {	 
	u16 tcnt=0;
	u8 key;
	delay_init();	    	 //��ʱ������ʼ��	  
	NVIC_Configuration(); 	 //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(9600);	 	//���ڳ�ʼ��Ϊ9600
 	LED_Init();			     //LED�˿ڳ�ʼ��
	LCD_Init();			 	
	KEY_Init();				//��ʼ������
	RTC_Init();				//��ʼ��RTC
	//Adc_Init();				//��ʼ��ADC	  
	POINT_COLOR=RED;		//����Ϊ��ɫ
	LCD_ShowString(60,30,200,16,16,"ENC28J60 TEST"); 
	LCD_ShowString(60,70,200,16,16,"KEY_UP:Select");	 
	LCD_ShowString(60,90,200,16,16,"KEY_DOWN:Confirm");	  
	LCD_ShowString(60,110,200,16,16,"IP:192.168.1.16");	   						  	 
	LCD_ShowString(60,130,200,16,16,"MASK:255.255.255.0");	   						  	 
	LCD_ShowString(60,150,200,16,16,"GATEWAY:192.168.1.1");	   						  	 
	
 	LCD_ShowString(60,200,200,16,16,"RX:");	   						  	 
	LCD_ShowString(60,220,200,16,16,"TX:");	


	#if LWIP_DHCP>0			   					//��ʹ��DHCPЭ��
	  ipaddr.addr = 0;
	  netmask.addr = 0;
	  gw.addr = 0; 
	#else										//
	  IP4_ADDR(&ipaddr, 192, 168, 2, 16);  		//���ñ���ip��ַ
	  IP4_ADDR(&gw, 192, 168, 1, 1);			//����
	  IP4_ADDR(&netmask, 255, 255, 255, 0);		//��������	 
	#endif
   
  init_lwip_timer();  //��ʼ��LWIP��ʱ��
	//��ʼ��LWIPЭ��ջ,ִ�м���û����п����õ�ֵ����ʼ�����е�ģ��
	lwip_init(); 
	//�������ӿ�
	while((netif_add(&enc28j60_netif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input)==NULL))
	{
		LCD_ShowString(60,170,200,16,16,"ENC28J60 Init Error!");	 
		delay_ms(200);
		LCD_Fill(60,170,240,186,WHITE);//���֮ǰ��ʾ
		delay_ms(200);
	}
	//ע��Ĭ�ϵ�����ӿ�
	netif_set_default(&enc28j60_netif);
	//��������ӿ����ڴ���ͨ��
  netif_set_up(&enc28j60_netif); 
	 
	/*
    Note: you must call dhcp_fine_tmr() and dhcp_coarse_tmr() at
    the predefined regular intervals after starting the client.
    You can peek in the netif->dhcp struct for the actual DHCP status.	
    ip_addr_t offered_ip_addr;
    ip_addr_t offered_sn_mask;
    ip_addr_t offered_gw_addr;
		dhcp_ip = enc28j60_netif.dhcp->offered_ip_addr.addr;
		printf("IP��ַ��%ld,%ld,%ld,%ld\t\n",(dhcp_ip&0x000000ff),(dhcp_ip&0x0000ff00)>>8,(dhcp_ip&0x00ff0000)>>16,(dhcp_ip&0xff000000)>>24);	
	*/
    #if LWIP_DHCP>0			   					//��ʹ��DHCPЭ��
	dhcp_start(&enc28j60_netif);             //Ϊ��������һ���µ�DHCP�ͻ���
	#endif

	select_lwip_mode();  //ѡ����Ҫ���е�ʵ��
	lwip_demo_init();		//��ʼ��lwip_demo	
	while (1)
	{
		LWIP_Polling();  		//LWIP��ѯ
		key=KEY_Scan();
		if(key==KEY_DOWN){	//��������
					if((lwip_flag&LWIP_CONNECTED)==LWIP_CONNECTED){ //�������˲ſ��Է������� ��Ҫ���TCP��˵ UDP�Ͳ���Ҫ������
						tcnt++;
						sprintf((char*)lwip_demo_buf,"This is a test %d\r\n",tcnt);	 
						LCD_Fill(86,220,240,236,WHITE);		//���֮ǰ��ʾ
						LCD_ShowString(86,220,154,16,16,lwip_demo_buf);//��ʾ��ǰ��������
						lwip_flag |= LWIP_SEND_DATA;			//�����������Ҫ����		 
					}else	if(lwip_test_mode == LWIP_TCP_CLIENT){  //û�������ϣ���ʱ����TCP�ͻ���ģʽ��������������
						tcp_client_connect_remotehost();
					}else if(lwip_test_mode == LWIP_UDP_SERVER){  
							tcnt++;
							sprintf((char*)lwip_demo_buf,"This is a test %d\r\n",tcnt);						
							LCD_Fill(86,220,240,236,WHITE);		//���֮ǰ��ʾ
							LCD_ShowString(86,220,154,16,16,lwip_demo_buf);//��ʾ��ǰ��������
							lwip_flag |= LWIP_SEND_DATA;			//�����������Ҫ����							
							udp_server_send_data();
					}else if(lwip_test_mode == LWIP_UDP_CLIENT){
							tcnt++;
							sprintf((char*)lwip_demo_buf,"This is a test %d\r\n",tcnt);						
							LCD_Fill(86,220,240,236,WHITE);		//���֮ǰ��ʾ
							LCD_ShowString(86,220,154,16,16,lwip_demo_buf);//��ʾ��ǰ��������
							lwip_flag |= LWIP_SEND_DATA;			//�����������Ҫ����							
							udp_client_send_data();					
					 }
		}
		if((lwip_flag&LWIP_NEW_DATA)==LWIP_NEW_DATA){
					LCD_Fill(86,200,240,216,WHITE);	//���֮ǰ��ʾ
					LCD_ShowString(86,200,154,16,16,lwip_demo_buf);
					printf("LWIP RX:%s\r\n",lwip_demo_buf);//��ӡ����
	        lwip_flag &=~LWIP_NEW_DATA;		//����������ݵı�־
		}
		delay_ms(1); 
	}  
} 


