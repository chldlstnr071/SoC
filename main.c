#include <stdio.h>
#include "xil_types.h"
#include "xparameters.h"
#include "xiicps.h"
#include "seven_seg.h"
#include "textlcd.h"
#include "xuartps_hw.h"

#define IIC_SLAVE_ADDR  0x51
#define IIC_SCLK_RATE  100000

#define  CR    0x0D
#define  LF    0x0A
#define  S_MAX    20
#define  Time_MAX    20

void  WriteTLCDReg(char *pRegVal, int val);
char bin2ascii(char bin);
int  ReadRTC(XIicPs Iic, u8 *SendBuffer, u8 *RecvBuffer);

void 	GetCmd(u8 *sel);
void 	PrintChar(u8 *str);
void 	InitMsg(void);
void	GetTime(u8 *Time, int* seg);
void SettingTime(XIicPs Iic, int *seg);
int	KeyToAscii(u8 a);

int main()
{
	 XIicPs  Iic;
	 int   IicStatus;
	 u8  *SendBuffer;
	 u8  RecvBuffer[14];
	 u8  pRecvBuffer[14];
	 u8  sel = 0;
	 u32  CntrlRegister;
	 u8	Time[Time_MAX] = {0, };
	 u8 Seg[S_MAX]	= 	{0, };

	 int  SegReg;
	 int  TEXTReg;
	 int  ChangeSeg;
	 char TlcdReg_upline[16];
	 char TlcdReg_downline[16];
	 int  seg[6];

	 int  i;
	 int  wait;
	 int n;


	 	CntrlRegister = XUartPs_ReadReg(XPAR_PS7_UART_1_BASEADDR, XUARTPS_CR_OFFSET);
	 	XUartPs_WriteReg( XPAR_PS7_UART_1_BASEADDR, XUARTPS_CR_OFFSET,
	 	   ((CntrlRegister & ~XUARTPS_CR_EN_DIS_MASK) | XUARTPS_CR_TX_EN | XUARTPS_CR_RX_EN) );

	 	InitMsg();

	 	 while(1)
	 	 {
	 		 EXIT:
	 		  PrintChar("What number do you want? ");
	 		  PrintChar("1. Current Time");
	 		  PrintChar("2. Date");
	 		  PrintChar("3. Setting Time ");
	 		  GetCmd(&sel);

	 		  switch(sel)
	 		  {
	 			   case ('1') :
	 			   {
	 				   do
	 					{
	 						n = XUartPs_IsReceiveData(XPAR_PS7_UART_1_BASEADDR);

	 						sprintf(TlcdReg_upline, "Clock Time      ");

	 						IicStatus = ReadRTC(Iic, SendBuffer, RecvBuffer);

	 						if (IicStatus != XST_SUCCESS)
	 						{
	 							return XST_FAILURE;
	 						}

	 						SegReg = ((RecvBuffer[2]&0x3f)<<24)+((0xA)<<20)+((RecvBuffer[1]&0x7f)<<12)+((0xA)<<8)+((RecvBuffer[0]&0x7f));
	 						SEVEN_SEG_mWriteReg(XPAR_SEVEN_SEG_0_S00_AXI_BASEADDR, 0, SegReg);

	 						WriteTLCDReg(TlcdReg_downline, SegReg);
	 						for(i = 0; i < 4; i++)
	 						{
	 							TEXTLCD_mWriteReg(XPAR_TEXTLCD_0_S00_AXI_BASEADDR, 4*i, (TlcdReg_upline[4*i]<<24)+(TlcdReg_upline[4*i+1]<<16)+(TlcdReg_upline[4*i+2]<<8)+(TlcdReg_upline[4*i+3]));
	 							TEXTLCD_mWriteReg(XPAR_TEXTLCD_0_S00_AXI_BASEADDR, 4*i+16, (TlcdReg_downline[4*i]<<24)+(TlcdReg_downline[4*i+1]<<16)+(TlcdReg_downline[4*i+2]<<8)+(TlcdReg_downline[4*i+3]));
	 						}

	 						for(wait = 0; wait < 1200; wait++);
	 					}while(n == FALSE);

	 				   goto EXIT;
	 				}


			  case ('2'):
				{
					sprintf(TlcdReg_upline, "Date of Today   ");

					IicStatus = ReadRTC(Iic, SendBuffer, RecvBuffer);

					if (IicStatus != XST_SUCCESS)
					{
						return XST_FAILURE;
					}

					SegReg = ((RecvBuffer[6]&0xff)<<24)+((0xA)<<20)+((RecvBuffer[5]&0x1f)<<12)+((0xA)<<8)+((RecvBuffer[3]&0x3f));
					SEVEN_SEG_mWriteReg(XPAR_SEVEN_SEG_0_S00_AXI_BASEADDR, 0, SegReg);

					WriteTLCDReg(TlcdReg_downline, SegReg);

					for(i = 0; i < 4; i++)
					{
						TEXTLCD_mWriteReg(XPAR_TEXTLCD_0_S00_AXI_BASEADDR, 4*i, (TlcdReg_upline[4*i]<<24)+(TlcdReg_upline[4*i+1]<<16)+(TlcdReg_upline[4*i+2]<<8)+(TlcdReg_upline[4*i+3]));
					}
					TEXTLCD_mWriteReg(XPAR_TEXTLCD_0_S00_AXI_BASEADDR, 16, (TlcdReg_downline[0]<<24)+(TlcdReg_downline[1]<<16)+(0x59<<8)+(0x20));
					TEXTLCD_mWriteReg(XPAR_TEXTLCD_0_S00_AXI_BASEADDR, 20, (TlcdReg_downline[3]<<24)+(TlcdReg_downline[4]<<16)+((0x4D)<<8)+((0x20)));
					TEXTLCD_mWriteReg(XPAR_TEXTLCD_0_S00_AXI_BASEADDR, 24, (TlcdReg_downline[6]<<24)+(TlcdReg_downline[7]<<16)+((0x44)<<8)+((0x20)));
					TEXTLCD_mWriteReg(XPAR_TEXTLCD_0_S00_AXI_BASEADDR, 28, (0x20<<24)+(0x20<<16)+(0x20<<8)+(0x20));

					break;
				}


	 			 default :
				{
					if(sel!=CR)
					{
					sprintf(TlcdReg_upline, "select only     ");
					sprintf(TlcdReg_downline, "1 to 2 please   ");
					PrintChar("Select only 1 ~ 2, please!");
					}

					for(i = 0; i < 4; i++)
					{
					  TEXTLCD_mWriteReg(XPAR_TEXTLCD_0_S00_AXI_BASEADDR, 4*i, (TlcdReg_upline[4*i]<<24)+(TlcdReg_upline[4*i+1]<<16)+(TlcdReg_upline[4*i+2]<<8)+(TlcdReg_upline[4*i+3]));
					  TEXTLCD_mWriteReg(XPAR_TEXTLCD_0_S00_AXI_BASEADDR, 4*i+16, (TlcdReg_downline[4*i]<<24)+(TlcdReg_downline[4*i+1]<<16)+(TlcdReg_downline[4*i+2]<<8)+(TlcdReg_downline[4*i+3]));
					}

					for(wait = 0; wait < 1200; wait++);
					break;
					}

	 		  }
	 }
}

void WriteTLCDReg(char *pRegVal, int val)
{
	int  i = 0;
	char temp;

	for(i = 0; i < 8; i++)
	{
		temp = bin2ascii((val>>(28-4*i))&0x0f);
		pRegVal[i] = temp;
		pRegVal[i+8] = 0x20;
	}
}

char bin2ascii(char bin)
{
	return bin = (0x30)+bin;
}

void InitMsg(void)
{
		PrintChar("===== Welcome!!! =====");
}

void PrintChar(u8 *str)
{
	XUartPs_SendByte(XPAR_PS7_UART_1_BASEADDR, LF);
	XUartPs_SendByte(XPAR_PS7_UART_1_BASEADDR, CR);

	while(*str != 0)
	{
		XUartPs_SendByte(XPAR_PS7_UART_1_BASEADDR, *str++);
	}
}

void GetCmd(u8 *sel)
{
	*sel = XUartPs_RecvByte(XPAR_PS7_UART_1_BASEADDR);
	if((*sel)=='1'||(*sel)=='2'||(*sel)=='3')
	XUartPs_SendByte(XPAR_PS7_UART_1_BASEADDR, *sel);
}

int ReadRTC(XIicPs Iic, u8 *SendBuffer, u8 *RecvBuffer)
{
	int    Status;
	XIicPs_Config *Config;

	Config = XIicPs_LookupConfig(XPAR_XIICPS_0_DEVICE_ID);
	if (Config == NULL)
	{
	return XST_FAILURE;
	}

	Status = XIicPs_CfgInitialize(&Iic, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS)
	{
	return XST_FAILURE;
	}

	XIicPs_SetSClk(&Iic, IIC_SCLK_RATE);

	*SendBuffer = 0x02;

	int i;

	for(i=0; i<14; i++)
	{
		RecvBuffer[i] = 0x00;
	}

	Status = XIicPs_MasterSendPolled(&Iic, SendBuffer, 1, IIC_SLAVE_ADDR);
	if(Status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}

	while(XIicPs_BusIsBusy(&Iic))
	{
	/* NOP */
	}

	Status = XIicPs_MasterRecvPolled(&Iic, RecvBuffer, 8, IIC_SLAVE_ADDR);

	if (Status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}

	return XST_SUCCESS;

}
