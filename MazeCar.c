#include<reg52.h>
#define uint unsigned int
#define uchar unsigned char


sbit IN1=P1^0;
sbit IN2=P1^1;
sbit IN3=P1^4;
sbit IN4=P1^5;
sbit ENA=P1^2;
sbit ENB=P1^3;
sbit front=P3^5;
sbit left=P3^6;
sbit right=P3^2;

uchar count=0,straight=0;

void DelayMS(uint x)
{
   uchar i;
	while(x--) for(i=0;i<60;i++);
}

void main()
{
	P3=0xff;
	count=0;
	left=1;
	right=1;
	front=1;
	ENA=1;
	ENB=1;
	IN1=1;//右轮驱动
	IN2=0;
	IN3=1;//左轮驱动
	IN4=0;
	TMOD=0x11; //定时器模式，两个定时器都开模式1
	TH1=236;
	TL1=120;   //10ms
	EA=1;
	ET1=1;
	TR1=0;        //定时器1用来减速，默认关闭，只有转向时打开
	TH0=158;      //50ms
	TL0=88;ET0=1;    //定时器0用来判断是否卡死
	while(1)
	{
		 if(left==0&&front==0&&right==0) //三面都是墙，将转180度
		 { 
			  TR1=1;   //打开减速定时器
			  IN1=0;IN2=1;IN3=1;IN4=0;TR0=0;
	      DelayMS(327); // *****************************************************三面不能走时 掉头需要的时间
		    IN1=1;IN2=0;IN3=1;IN4=0;
	      TR1=0;  //关闭减速定时器
		 } 
		 else if(left==0&&front==0&&right==1)// 只有右边能走，需要右转
		 {	
			  TR1=1;  //打开减速定时器
			  TR0=0;
			  IN3=1;IN4=0;IN1=0;IN2=1; 
		    DelayMS(321); // ****************************************************只有右边能走时 右转需要的时间
			  TR1=0;  //关闭减速定时器
			  IN1=1;IN2=0;
			  DelayMS(400);   // ****************************************************只有右边能走时 右转后需要预前进的时间

     } 
		 else if(left==0&&front==1&&right==0)//只有前面能走，往前走
		 {
        IN1=1;IN2=0;IN3=1;IN4=0;TR0=0;
     } 

     else if(left==0&&front==1&&right==1) //前面和右边都能走，需要判断
		 {	 
        DelayMS(325); //先往前走一点****************************************出口处 需要探出头得时间
			  if(left==0&&front==1&&right==1)   //如果前进了一段时间前面还是没有障碍就右转。
			  {
				   TR1=1;  //打开减速定时器
				   IN1=0;IN2=1;IN3=1;IN4=0; //右转
		       DelayMS(321);   // ************************************************出口处 右转需要的时间
					 TR1=0;  //关闭减速定时器
					 TR0=0;
		       IN1=1;IN2=0;IN3=1;IN4=0;
		       DelayMS(400);   // ************************************************出口处 右转后需要预前进的时间
        }
			}
     else if(left==1&&front==0&&right==0) //只有左边能走，需要左转
		 {
			  TR1=1;   //打开减速定时器
			  IN3=0;IN4=1;IN1=1;IN2=0;  //左轮后退，右轮前进，将左转
			  TR0=0;
		    DelayMS(331);   // ****************************************************只有左边能走时 左转需要的时间
			  TR1=0;    //关闭减速定时器
		    IN3=1;IN4=0;
		    DelayMS(400); // ****************************************************只有左边能走时 左转后需要预前进的时间
     }
     else if(left==1&&front==0&&right==1)  //左边和右边能走，前面不能走，左转
		 { 
		    TR1=1;   //打开减速定时器
			  IN3=0;IN4=1;IN1=1;IN2=0;  //左轮后退，右轮前进，将左转
			  TR0=0;
		    DelayMS(338);  // ***************************************************左右都能走时 左转需要的时间
			  TR1=0;    //关闭减速定时器
		    IN3=1;IN4=0;
		    DelayMS(350);   // *************************************************左右都能走时 转正后需要预前进的时间
     }
     else if(left==1&&front==1&&right==0) //左边和前面能走，默认往前走，但是如果卡住，就用中断函数微调整
     { 			 
			  TR0=1;  //开启微调整定时器 650ms
		    IN3=1;IN4=0;IN1=1;IN2=0;
			  continue;
     }
		 if(left==1&&front==1&&right==1)	
		 {
        while(1) //前进一小段一小段
			  {
       	   IN1=1;IN2=0;IN3=1;IN4=0;
				   DelayMS(50);   //每步为50ms
				   if(left==1&&front==1&&right==1) 
				      straight++;  //如果还保持这个状态，计数值加一
				   else{straight=0; break;}  //如果不是这个状态，计数值归零，跳出判断循环
				   if(straight==25)  //如果计数值达到25，就是前进了1250ms还是这个状态，需要微调整
				   {
              straight=0;    //如果计数值达到12，就是前进了600ms还是这个状态，需要微调整
					    IN1=0;IN2=1;IN3=0;IN4=1;
		          DelayMS(315);   //**********************************************三面没东西微调时 需要倒退的时间
					    TR1=1;
              IN3=1;IN4=0;
		          DelayMS(400);    // *********************************************三面没东西微调时 要微掉头的时间
					    TR1=0;
	       	    IN3=1;IN4=0;IN1=1;IN2=0;
           }
        }
     }
		 TR0=0;
		 TH0=158;
	   TL0=88;
		 count=0;
  }
}
void T0_time() interrupt 1  //判断是否卡死的中断，卡死状态为左前能走，右边不能走
{
  TH0=158;  //定时器周期为50ms
	TL0=88;
	count++;
	if(count==13) //忍耐极限为650ms
	{
     IN1=0;IN2=1;IN3=0;IN4=1;  //确认卡死后先后退一点。
		 DelayMS(300);TR1=1;
		 IN1=1;IN2=0; //右轮前进，车的朝向往左微调
		 DelayMS(200); //*****************************************微调退转持续时间
		 TR1=0;
		 IN3=1;IN4=0;IN1=1;IN2=0;
		 count=0;    //计数值归零；
		 TH0=158;TL0=88;
		 TR0=0; //定时器初始化并关闭
  }
}
void T1_time() interrupt 3  //减速中断。转弯时减速
{ 
   uchar a,b,c,d; 
	 a=IN1;
	 b=IN2;
	 c=IN3;
	 d=IN4;
	 TH1=236;
	 TL1=120;
     
   IN1=1;
   IN2=1;
	 IN3=1;
	 IN4=1;
	 DelayMS(3);

	 IN1=a;
	 IN2=b;
	 IN3=c;
	 IN4=d;
}