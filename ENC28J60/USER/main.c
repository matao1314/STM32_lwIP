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

#define CLOCKTICKS_PER_MS 10    //定义时钟节拍


static ip_addr_t ipaddr, netmask, gw; //定义IP地址
struct netif enc28j60_netif;  //定义网络接口
u32_t last_arp_time;			
u32_t last_tcp_time;	
u32_t last_ipreass_time;


u32_t input_time;

#if LWIP_DHCP>0	
u32_t last_dhcp_fine_time;			
u32_t last_dhcp_coarse_time;  
u32 dhcp_ip=0;
#endif




//LWIP查询
void LWIP_Polling(void){
	
	// if(timer_expired(&input_time,5)) //接收包，周期处理函数
 // {
    ethernetif_input(&enc28j60_netif); 
 // }
  if(timer_expired(&last_tcp_time,TCP_TMR_INTERVAL/CLOCKTICKS_PER_MS))//TCP处理定时器处理函数
  {
	 tcp_tmr();
  }
  if(timer_expired(&last_arp_time,ARP_TMR_INTERVAL/CLOCKTICKS_PER_MS))//ARP处理定时器
  {
	etharp_tmr();
  }
	if(timer_expired(&last_ipreass_time,IP_TMR_INTERVAL/CLOCKTICKS_PER_MS)){ //IP重新组装定时器
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

//显示连接状态
void show_connect_status(void){
		if((lwip_flag&LWIP_CONNECTED)==LWIP_CONNECTED){		//连接成功了
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

//选择模式
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
	delay_init();	    	 //延时函数初始化	  
	NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(9600);	 	//串口初始化为9600
 	LED_Init();			     //LED端口初始化
	LCD_Init();			 	
	KEY_Init();				//初始化按键
	RTC_Init();				//初始化RTC
	//Adc_Init();				//初始化ADC	  
	POINT_COLOR=RED;		//设置为红色
	LCD_ShowString(60,30,200,16,16,"ENC28J60 TEST"); 
	LCD_ShowString(60,70,200,16,16,"KEY_UP:Select");	 
	LCD_ShowString(60,90,200,16,16,"KEY_DOWN:Confirm");	  
	LCD_ShowString(60,110,200,16,16,"IP:192.168.1.16");	   						  	 
	LCD_ShowString(60,130,200,16,16,"MASK:255.255.255.0");	   						  	 
	LCD_ShowString(60,150,200,16,16,"GATEWAY:192.168.1.1");	   						  	 
	
 	LCD_ShowString(60,200,200,16,16,"RX:");	   						  	 
	LCD_ShowString(60,220,200,16,16,"TX:");	


	#if LWIP_DHCP>0			   					//若使用DHCP协议
	  ipaddr.addr = 0;
	  netmask.addr = 0;
	  gw.addr = 0; 
	#else										//
	  IP4_ADDR(&ipaddr, 192, 168, 2, 16);  		//设置本地ip地址
	  IP4_ADDR(&gw, 192, 168, 1, 1);			//网关
	  IP4_ADDR(&netmask, 255, 255, 255, 0);		//子网掩码	 
	#endif
   
  init_lwip_timer();  //初始化LWIP定时器
	//初始化LWIP协议栈,执行检查用户所有可配置的值，初始化所有的模块
	lwip_init(); 
	//添加网络接口
	while((netif_add(&enc28j60_netif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input)==NULL))
	{
		LCD_ShowString(60,170,200,16,16,"ENC28J60 Init Error!");	 
		delay_ms(200);
		LCD_Fill(60,170,240,186,WHITE);//清除之前显示
		delay_ms(200);
	}
	//注册默认的网络接口
	netif_set_default(&enc28j60_netif);
	//建立网络接口用于处理通信
  netif_set_up(&enc28j60_netif); 
	 
	/*
    Note: you must call dhcp_fine_tmr() and dhcp_coarse_tmr() at
    the predefined regular intervals after starting the client.
    You can peek in the netif->dhcp struct for the actual DHCP status.	
    ip_addr_t offered_ip_addr;
    ip_addr_t offered_sn_mask;
    ip_addr_t offered_gw_addr;
		dhcp_ip = enc28j60_netif.dhcp->offered_ip_addr.addr;
		printf("IP地址：%ld,%ld,%ld,%ld\t\n",(dhcp_ip&0x000000ff),(dhcp_ip&0x0000ff00)>>8,(dhcp_ip&0x00ff0000)>>16,(dhcp_ip&0xff000000)>>24);	
	*/
    #if LWIP_DHCP>0			   					//若使用DHCP协议
	dhcp_start(&enc28j60_netif);             //为网卡创建一个新的DHCP客户端
	#endif

	select_lwip_mode();  //选择需要进行的实验
	lwip_demo_init();		//初始化lwip_demo	
	while (1)
	{
		LWIP_Polling();  		//LWIP查询
		key=KEY_Scan();
		if(key==KEY_DOWN){	//发送数据
					if((lwip_flag&LWIP_CONNECTED)==LWIP_CONNECTED){ //连接上了才可以发送数据 主要针对TCP来说 UDP就不需要连接了
						tcnt++;
						sprintf((char*)lwip_demo_buf,"This is a test %d\r\n",tcnt);	 
						LCD_Fill(86,220,240,236,WHITE);		//清除之前显示
						LCD_ShowString(86,220,154,16,16,lwip_demo_buf);//显示当前发送数据
						lwip_flag |= LWIP_SEND_DATA;			//标记有数据需要发送		 
					}else	if(lwip_test_mode == LWIP_TCP_CLIENT){  //没有连接上，此时处于TCP客户端模式，则尝试重新连接
						tcp_client_connect_remotehost();
					}else if(lwip_test_mode == LWIP_UDP_SERVER){  
							tcnt++;
							sprintf((char*)lwip_demo_buf,"This is a test %d\r\n",tcnt);						
							LCD_Fill(86,220,240,236,WHITE);		//清除之前显示
							LCD_ShowString(86,220,154,16,16,lwip_demo_buf);//显示当前发送数据
							lwip_flag |= LWIP_SEND_DATA;			//标记有数据需要发送							
							udp_server_send_data();
					}else if(lwip_test_mode == LWIP_UDP_CLIENT){
							tcnt++;
							sprintf((char*)lwip_demo_buf,"This is a test %d\r\n",tcnt);						
							LCD_Fill(86,220,240,236,WHITE);		//清除之前显示
							LCD_ShowString(86,220,154,16,16,lwip_demo_buf);//显示当前发送数据
							lwip_flag |= LWIP_SEND_DATA;			//标记有数据需要发送							
							udp_client_send_data();					
					 }
		}
		if((lwip_flag&LWIP_NEW_DATA)==LWIP_NEW_DATA){
					LCD_Fill(86,200,240,216,WHITE);	//清除之前显示
					LCD_ShowString(86,200,154,16,16,lwip_demo_buf);
					printf("LWIP RX:%s\r\n",lwip_demo_buf);//打印数据
	        lwip_flag &=~LWIP_NEW_DATA;		//清除接受数据的标志
		}
		delay_ms(1); 
	}  
} 


