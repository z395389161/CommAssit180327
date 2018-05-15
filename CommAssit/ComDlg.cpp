// Com.cpp : implementation file
//

#include "stdafx.h"
#include "CommAssit.h"
#include "ComDlg.h"
#include <stdio.h>  



// CComDlgdialog

#define SEND_BUFFER_SIZE 64
#define RECV_BUFFER_SIZE (20*1024*1024)
#define RECV_BUFF_SIZE 2048
#define DISPLAY_BUFFER_SIZE (RECV_BUFFER_SIZE + 1)
#define DISPLAY_HEX_BUFFER_SIZE (3*DISPLAY_BUFFER_SIZE + 1)
#define BATTERY_CURRENT_SIZE 6
#define BATTERY_VOLTAGE_SIZE 32
#define BATTERY_SOC_SIZE 32
#define BATTERY_TEMP_SIZE 32
#define BATTERY_QUANTITY_SIZE 32
#define BATTERY_INDEX_SIZE 32
#define BATTERY_STATE_SIZE 16

#define WM_REFRESH_COUNT (WM_USER + 1)

DWORD __stdcall ThreadProcComRecv(LPVOID lpParameter)
{
	CComDlg *pDlg = (CComDlg*)lpParameter;
	pDlg->ComRecv();

	return 0;
}


DWORD __stdcall ThreadProcComDisplay(LPVOID lpParameter)
{
	CComDlg *pDlg = (CComDlg*)lpParameter;
	pDlg->ComDisplay();

	return 0;
}


IMPLEMENT_DYNAMIC(CComDlg, CDialog)


CComDlg::CComDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CComDlg::IDD, pParent)
	, m_int_Com_SendPeriod(0)
	, m_int_Com_Send_Length(0)
	, m_int_Com_Recv_Length(0)
{
	m_bool_Com_Open = FALSE;
	m_pbyte_Recv_Buffer = NULL;
	m_pbyte_Send_Buffer = NULL;
	m_pbyte_Recv_Buff = NULL;
	m_pbyte_Display_Buffer = NULL;
	m_pchar_Display_Hex_Buffer = NULL;
	m_pbyte_Recv_Buffer = new BYTE[RECV_BUFFER_SIZE];
	m_pbyte_Send_Buffer = new BYTE[SEND_BUFFER_SIZE];
	m_pbyte_Recv_Buff = new BYTE[RECV_BUFF_SIZE];
	m_pbyte_Display_Buffer = new BYTE[DISPLAY_BUFFER_SIZE];
	m_pchar_Display_Hex_Buffer = new char[DISPLAY_HEX_BUFFER_SIZE];
	m_int_Recv_Buffer_Read = 0;
	m_int_Recv_Buffer_Write = 0;
	m_battery_current=NULL;
	m_battery_voltage=NULL;
	m_battery_soc=NULL;
	m_battery_temp=NULL;
	m_battery_quantity=NULL;
	m_battery_index=NULL;

	m_battery_judge=NULL;
	m_battery_state2=NULL;
	m_battery_state1=NULL;
	m_battery_state0=NULL;
	m_battery_state=NULL;

	m_battery_current=new char[BATTERY_CURRENT_SIZE];
	m_battery_voltage=new char[BATTERY_VOLTAGE_SIZE];
	m_battery_soc=new char[BATTERY_SOC_SIZE];
	m_battery_temp=new char[BATTERY_TEMP_SIZE];
	m_battery_quantity=new char[BATTERY_QUANTITY_SIZE];
	m_battery_index=new char[BATTERY_INDEX_SIZE];
	m_battery_judge=new char[BATTERY_STATE_SIZE];
	m_battery_state1=new char[BATTERY_STATE_SIZE];
	m_battery_state2=new char[BATTERY_STATE_SIZE];
	m_battery_state0=new char[BATTERY_STATE_SIZE];
	m_battery_state=new char[BATTERY_STATE_SIZE];

	m_hThread_Send = INVALID_HANDLE_VALUE;
	m_hThread_Recv = INVALID_HANDLE_VALUE;
	m_hThread_Display = INVALID_HANDLE_VALUE;

	m_bSending = FALSE;

	m_state = 0;
}

CComDlg::~CComDlg()
{
}

void CComDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_COM_PORT, m_combo_Com_Port);
	DDX_Control(pDX, IDC_COMBO_COM_BAUDRATE, m_combo_Com_BaudRate);
	DDX_Control(pDX, IDC_COMBO_COM_DATABITS, m_combo_Com_DataBits);
	DDX_Control(pDX, IDC_COMBO_COM_VERIFYBITS, m_combo_Com_VerifyBits);
	DDX_Control(pDX, IDC_COMBO_COM_STOPBITS, m_combo_Com_StopBits);
	DDX_Control(pDX, IDC_BUTTON_COM_OPEN, m_button_Com_Open);
	DDX_Control(pDX, IDC_CHECK_COM_RECVHEX, m_check_Com_RecvHex);
	DDX_Control(pDX, IDC_CHECK_COM_SENDHEX, m_check_Com_SendHex);
	DDX_Control(pDX, IDC_CHECK_COM_SEND_LOOP, m_chekc_Com_Send_Loop);
	DDX_Text(pDX, IDC_EDIT_COM_SENDPERIOD, m_int_Com_SendPeriod);
	DDX_Control(pDX, IDC_RICHEDIT_COM_RECV, m_richedit_Com_Recv);
	DDX_Control(pDX, IDC_RICHEDIT_COM_SEND, m_richedit_Com_Send);
	DDX_Text(pDX, IDC_EDIT_COM_SEND_LENGTH, m_int_Com_Send_Length);
	DDX_Text(pDX, IDC_EDIT_COM_RECV_LENGTH, m_int_Com_Recv_Length);
	DDX_Control(pDX, IDC_EDIT_COM_CURRENT, m_com_current);
	DDX_Control(pDX, IDC_EDIT_COM_SOC, m_com_soc);
	DDX_Control(pDX, IDC_EDIT_COM_INDEX, m_com_index);
	DDX_Control(pDX, IDC_EDIT_COM_QUANTITY, m_com_quantity);
	DDX_Control(pDX, IDC_EDIT_COM_VOLTAGE, m_com_voltage);
	DDX_Control(pDX, IDC_EDIT_COM_TEMP, m_com_temp);

	DDX_Control(pDX, IDC_STATIC_LOW, m_icon_low);
	DDX_Control(pDX, IDC_STATIC_OVERTEMP, m_icon_overtemp);
	DDX_Control(pDX, IDC_STATIC_OVERPOWER, m_icon_overpower);
	DDX_Control(pDX, IDC_STATIC_CHARGE, m_icon_charge);
}


BEGIN_MESSAGE_MAP(CComDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_COM_OPEN, &CComDlg::OnBnClickedButtonComOpen)
	ON_BN_CLICKED(IDC_BUTTON_COM_REFRESHPORT, &CComDlg::OnBnClickedButtonComRefreshport)
	ON_WM_DESTROY()
	ON_MESSAGE(WM_REFRESH_COUNT, &CComDlg::OnMessageRefreshCount)
	ON_BN_CLICKED(IDC_BUTTON_COM_SEND_CMD, &CComDlg::OnBnClickedButtonComSendCmd)
	ON_BN_CLICKED(IDC_BUTTON_COM_CLEARDISPLAY, &CComDlg::OnBnClickedButtonComCleardisplay)
ON_WM_TIMER()
END_MESSAGE_MAP()


// CComDlgmessage handlers

void CComDlg::OnBnClickedButtonComOpen()
{
	// TODO: Add your control notification handler code here
	if(m_bool_Com_Open)
	{
		TRESULT res;
		//关闭线程
		TerminateThread(m_hThread_Recv, 0);
		TerminateThread(m_hThread_Display, 0);

		if(INVALID_HANDLE_VALUE != m_hThread_Send)
		{
			TerminateThread(m_hThread_Send, 0);
		}

		res = m_CommCom.Close();
		if(TRESULT_OK != res)
		{
			::SendMessage(m_StatusBar.GetSafeHwnd(),SB_SETTEXT,(WPARAM)1,(LPARAM)(_T("关闭串口失败！")));
			return;
		}
		m_button_Com_Open.SetWindowText("打开");
		UnlockComment();
		::SendMessage(m_StatusBar.GetSafeHwnd(),SB_SETTEXT,(WPARAM)1,(LPARAM)(_T("关闭串口成功！")));
		m_bool_Com_Open = FALSE;
	}
	else
	{
		//获取参数
		TRESULT res;
		char charCom[10] = {0};
		char charBaudRate[10] = {0};
		char charDataBits[10] = {0};
		DWORD dwordBaudRate = 0;
		BYTE byteDataBits = 0;
		BYTE byteVerifyBits = 0;
		BYTE byteStopBits = 0;

		m_combo_Com_Port.GetWindowText(charCom, 10);
		m_combo_Com_BaudRate.GetWindowText(charBaudRate, 10);
		m_combo_Com_DataBits.GetWindowText(charDataBits, 10);
		byteVerifyBits = m_combo_Com_VerifyBits.GetCurSel();
		byteStopBits = m_combo_Com_StopBits.GetCurSel();
		dwordBaudRate = atoi(charBaudRate);
		byteDataBits = atoi(charDataBits);
		res = m_CommCom.SetPara(charCom, dwordBaudRate, byteDataBits, byteVerifyBits, byteStopBits, 1);
		//打开串口
		res = m_CommCom.Open();
		if(TRESULT_OK != res)
		{
			::SendMessage(m_StatusBar.GetSafeHwnd(),SB_SETTEXT,(WPARAM)1,(LPARAM)(_T("打开串口失败！")));
			return;
		}
		m_bool_Com_Open = TRUE;
		::SendMessage(m_StatusBar.GetSafeHwnd(),SB_SETTEXT,(WPARAM)1,(LPARAM)(_T("打开串口成功！")));
		m_button_Com_Open.SetWindowText("关闭");
		LockComment();
		//开启线程
		m_hThread_Recv = CreateThread(NULL,0,ThreadProcComRecv,this,0,NULL);
		m_hThread_Display = CreateThread(NULL, 0, ThreadProcComDisplay, this, 0, NULL);
	}
}

void CComDlg::OnBnClickedButtonComRefreshport()
{
	// TODO: Add your control notification handler code here
	CArray<SSerInfo,SSerInfo&>asi;
	EnumSerialPorts(asi,FALSE);

	m_combo_Com_Port.ResetContent();
	
	if(asi.GetSize())
	{
		for(int i=0;i<asi.GetSize();i++)
		{
			m_combo_Com_Port.AddString(asi[i].strPortName);
		}
		m_combo_Com_Port.SetCurSel(0);
	}

}

BOOL CComDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	//刷新串口
	OnBnClickedButtonComRefreshport();
	//初始化串口参数选项
	m_combo_Com_BaudRate.AddString("110");
	m_combo_Com_BaudRate.AddString("300");
	m_combo_Com_BaudRate.AddString("600");
	m_combo_Com_BaudRate.AddString("1200");
	m_combo_Com_BaudRate.AddString("2400");
	m_combo_Com_BaudRate.AddString("4800");
	m_combo_Com_BaudRate.AddString("9600");
	m_combo_Com_BaudRate.AddString("14400");
	m_combo_Com_BaudRate.AddString("19200");
	m_combo_Com_BaudRate.AddString("38400");
	m_combo_Com_BaudRate.AddString("56000");
	m_combo_Com_BaudRate.AddString("57600");
	m_combo_Com_BaudRate.AddString("115200");
	m_combo_Com_BaudRate.AddString("128000");
	m_combo_Com_BaudRate.AddString("230400");
	m_combo_Com_BaudRate.AddString("256000");
	m_combo_Com_BaudRate.AddString("460800");
	m_combo_Com_BaudRate.AddString("512000");
	m_combo_Com_BaudRate.AddString("921600");
	m_combo_Com_BaudRate.SetCurSel(6);

	m_combo_Com_DataBits.AddString("8");
	m_combo_Com_DataBits.AddString("7");
	m_combo_Com_DataBits.AddString("6");
	m_combo_Com_DataBits.AddString("5");
	m_combo_Com_DataBits.SetCurSel(0);

	m_combo_Com_VerifyBits.AddString("NO");
	m_combo_Com_VerifyBits.AddString("ODD");
	m_combo_Com_VerifyBits.AddString("EVEN");
	m_combo_Com_VerifyBits.AddString("MASK");
	m_combo_Com_VerifyBits.AddString("SPACE");
	m_combo_Com_VerifyBits.SetCurSel(0);

	m_combo_Com_StopBits.AddString("1");
	m_combo_Com_StopBits.AddString("1.5");
	m_combo_Com_StopBits.AddString("2");
	m_combo_Com_StopBits.SetCurSel(0);
	//设置状态栏
	BOOL bRet=m_StatusBar.Create(this);
	UINT nIDS[3]={20001,20002,20003};
	bRet=m_StatusBar.SetIndicators(nIDS,3);
	m_StatusBar.SetPaneInfo(0,nIDS[0],SBPS_NORMAL,11);
	m_StatusBar.SetPaneInfo(1,nIDS[1],SBPS_NORMAL,200);
	m_StatusBar.SetPaneInfo(2,nIDS[2],SBPS_NORMAL,340);
	m_StatusBar.SetPaneText(1,"提示");

	RepositionBars(AFX_IDW_CONTROLBAR_FIRST,AFX_IDW_CONTROLBAR_LAST,0);

	//设置进度条
	m_Progress.Create(WS_CHILD|WS_VISIBLE,CRect(0,0,300,40),this,11000);
	m_Progress.SetRange(0,100);
	m_Progress.SetPos(0);
	m_Progress.SetParent(&m_StatusBar);
	CRect itemRC;
	m_StatusBar.GetItemRect(2,itemRC);
	m_Progress.MoveWindow(itemRC);
	m_Progress.ShowWindow(SW_SHOW);

	CFont font;
	font.CreateFont(14,0,0,0,FW_DONTCARE,FALSE,FALSE,0, 1,
    OUT_DEFAULT_PRECIS, 
    CLIP_DEFAULT_PRECIS,
    DEFAULT_QUALITY, 
    DEFAULT_PITCH|FF_SWISS,
    _T("宋体"));
	m_richedit_Com_Recv.SetFont(&font);
	m_richedit_Com_Send.SetFont(&font);
	m_richedit_Com_Send.SetWindowTextA("7E 01 03 00 04");//默认发送协议的具体数据

	//m_int_Com_SendPeriod.SetwindowText();
	CEdit*  pEdit=(CEdit*)GetDlgItem(IDC_EDIT_COM_SENDPERIOD);
	pEdit->SetWindowText(_T("100")); //设置时间框默认显示的内容

	m_hIcon_green= LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_M_ICON_GREEN));
	m_hIcon_gray= LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_M_ICON_GRAY));
	m_hIcon_red= LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_M_ICON_RED));
	m_icon_low.SetIcon(m_hIcon_gray);
	m_icon_charge.SetIcon(m_hIcon_gray);
	m_icon_overtemp.SetIcon(m_hIcon_gray);
	m_icon_overpower.SetIcon(m_hIcon_gray);


	//CHARFORMATA cfmt;
	//memset(&cfmt, 0, sizeof(cfmt));
	//cfmt.cbSize = sizeof(cfmt);
	//cfmt.yHeight = 200;
	//cfmt.dwMask = (CFM_SIZE | CFM_FACE | CFM_CHARSET);
	//cfmt.bCharSet = ANSI_CHARSET;
	//memcpy(cfmt.szFaceName, _T("宋体"), strlen(_T("宋体")));
	//m_richedit_Com_Send.SetWordCharFormat(cfmt);
	//m_richedit_Com_Send.SetDefaultCharFormat(cfmt);
	m_richedit_Com_Send.SendMessage(EM_SETLANGOPTIONS,0,0);
	//m_richedit_Com_Recv.SetWordCharFormat(cfmt);
	//m_richedit_Com_Recv.SetDefaultCharFormat(cfmt);
	m_richedit_Com_Recv.SendMessage(EM_SETLANGOPTIONS,0,0);
	font.Detach();
	font.DeleteObject();


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CComDlg::OnDestroy()
{
	DeInitData();
	CDialog::OnDestroy();

	// TODO: Add your message handler code here
}

int CComDlg::ComRecv(void)//接收数据
{
	UINT uintRecvedLength = 0;
	UINT uintRecvLength = 1024;
//	TRESULT res;
	int intOffset = 0;

	while(m_bool_Com_Open)
	{
		uintRecvedLength = 0;
		m_CommCom.Recv(m_pbyte_Recv_Buff, uintRecvLength, &uintRecvedLength);
		if(uintRecvedLength > 0)
		{
	//		TRACE1("\r\nrecv=%d", uintRecvedLength);
			m_int_Com_Recv_Length += uintRecvedLength;
			PostMessage(WM_REFRESH_COUNT, 0, 0);
			
			if( (uintRecvedLength + m_int_Recv_Buffer_Write) > RECV_BUFFER_SIZE)
			{
				intOffset = (RECV_BUFFER_SIZE - m_int_Recv_Buffer_Write);
				memcpy(m_pbyte_Recv_Buffer + m_int_Recv_Buffer_Write, m_pbyte_Recv_Buff, intOffset);
				m_int_Recv_Buffer_Write = 0;
				memcpy(m_pbyte_Recv_Buffer, m_pbyte_Recv_Buff + intOffset, (uintRecvedLength - intOffset));
				m_int_Recv_Buffer_Write += (uintRecvedLength - intOffset);
			}
			else
			{
				memcpy(m_pbyte_Recv_Buffer + m_int_Recv_Buffer_Write, m_pbyte_Recv_Buff, uintRecvedLength);
				m_int_Recv_Buffer_Write += (uintRecvedLength);
			}
		}
		else
		{
		//	TRACE0("\r\nsleep.");
			Sleep(20);
		}
	}
	return 0;
}
void trimall(char* s)//去掉空格的函数
{
	int l=strlen(s);
	char* tp=(char*)malloc(l+1);
	char* ctp=tp;
	char* cs=s;
	while(*s)
	{
		if(*s!=' ')
		{
			*tp=*s;
			tp++;
		}
		s++;
	}
	*tp='\0';
	strcpy(cs,ctp);
	free(ctp);
}
char* substr(char *str,int start,int len)//取接收数据中的一段
{
	char* res = (char*)malloc(len+1);
	res[len] = 0;
	memcpy(res, str + start - 1, len);
	return res;

}
 

int CComDlg::ComDisplay(void)//十六进制显示的数据
{
	int intRead;
	int intWrite;
	int intHexLength = 0;

	int m_current=0;
	int m_quantity=0;
	int m_soc=0;
	int m_temp=0;
	int m_index=0;
	int m_voltage=0;

	int m_state2=0;
	int m_state0=0;

	float m_voltage1=0;
	float m_temp1=0;
	float m_current1=0;

	while(m_bool_Com_Open)
	{
		intRead = m_int_Recv_Buffer_Read;
		intWrite = m_int_Recv_Buffer_Write;

		if(intRead != intWrite)//有数据
		{
			memset(m_pbyte_Display_Buffer, 0, DISPLAY_BUFFER_SIZE);
//			if(BST_CHECKED == (m_check_Com_RecvHex.GetCheck()))//如果点击按钮想要16进制显示
//			{
				memset(m_pchar_Display_Hex_Buffer, 0, DISPLAY_HEX_BUFFER_SIZE);
				if(intWrite > intRead)
				{
					TRACE1("\r\nrecv1: %d", intWrite - intRead);
					memcpy(m_pbyte_Display_Buffer, m_pbyte_Recv_Buffer + intRead, intWrite - intRead);
					intHexLength = Bytes2HexStr(m_pbyte_Display_Buffer, intWrite - intRead, m_pchar_Display_Hex_Buffer, DISPLAY_HEX_BUFFER_SIZE);
					m_richedit_Com_Recv.SetSel(-1, -1);
					m_richedit_Com_Recv.ReplaceSel(m_pchar_Display_Hex_Buffer);
					m_richedit_Com_Recv.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);
					
					BYTE* pbyte = m_pbyte_Display_Buffer;
					for(int i = 0; i < (intWrite - intRead); i++)
					{
						switch(m_state)
						{
						case 0:
							if(0x7E == *(pbyte + i))
							{
								m_byte_data[m_state] = *(pbyte + i);
								m_state++;
							}
							else
							{
								m_state = 0;
							}
							break;
						case 1:
							if(0x01 == *(pbyte + i))
							{
								m_byte_data[m_state] = *(pbyte + i);
								m_state++;
							}
							else
							{
								m_state = 0;
							}
							break;
						case 2:
							if(0x03 == *(pbyte + i))
							{
								m_byte_data[m_state] = *(pbyte + i);
								m_state++;
							}
							else
							{
								m_state = 0;
							}
							break;
						default:
							if(m_state < (18))
							{
								m_byte_data[m_state] = *(pbyte + i);
								m_state++;
							}
							if(18 == m_state)
							{
								intHexLength = Bytes2HexStr(m_byte_data, 18, m_pchar_Display_Hex_Buffer, DISPLAY_HEX_BUFFER_SIZE);
								trimall(m_pchar_Display_Hex_Buffer);

								m_battery_voltage=substr(m_pchar_Display_Hex_Buffer,9,4);
								m_battery_current=substr(m_pchar_Display_Hex_Buffer,13,4);
								m_battery_soc=substr(m_pchar_Display_Hex_Buffer,17,2);
								m_battery_temp=substr(m_pchar_Display_Hex_Buffer,19,4);
								m_battery_quantity =substr(m_pchar_Display_Hex_Buffer,23,4);
								m_battery_index=substr(m_pchar_Display_Hex_Buffer,27,4);

								//m_battery_state1=substr(m_pchar_Display_Hex_Buffer,31,2);
								m_battery_state2=substr(m_pchar_Display_Hex_Buffer,33,2);
								m_battery_judge=substr(m_pchar_Display_Hex_Buffer,35,2);
								sscanf(m_battery_state2,"%x",&m_state2);
								ltoa(m_state2,m_battery_state1,2);
								sprintf(m_battery_state0,"%08s",m_battery_state1);//长度至少为8，没有到8用0代替
								m_battery_state=substr(m_battery_state0,2,1);
								m_state0=atoi(m_battery_state);


								sscanf(m_battery_voltage,"%x",&m_voltage); //十六进制转化为十进制
								sscanf(m_battery_current,"%x",&m_current); 
								sscanf(m_battery_soc,"%x",&m_soc); 
								sscanf(m_battery_temp,"%x",&m_temp); 
								sscanf(m_battery_quantity,"%x",&m_quantity); 
								sscanf(m_battery_index,"%x",&m_index); 


								m_voltage1=(float)m_voltage/1000;//添加小数点
								m_temp1=(float)m_temp/10;
								m_current1=(float)m_current/1000;

								if(m_voltage1<100&&m_current1<20&&m_temp1<100&&m_soc<=100&&m_quantity<10000&&m_index<1000)
								{
									m_str_voltage.Format(_T("%3.3f"),m_voltage1);//在编辑框中显示
									m_com_voltage.SetWindowText(m_str_voltage);
									m_str_current.Format(_T("%3.3f"),m_current1);
									m_com_current.SetWindowText(m_str_current);
									m_str_quantity.Format(_T("%d"),m_quantity);
									m_com_quantity.SetWindowText(m_str_quantity);
									m_str_temp.Format(_T("%3.1f"),m_temp1);
									m_com_temp.SetWindowText(m_str_temp);
									m_str_soc.Format(_T("%d"),m_soc);
									m_com_soc.SetWindowText(m_str_soc);
									m_str_index.Format(_T("%d"),m_index);
									m_com_index.SetWindowText(m_str_index);
								}


								if(m_state0==0)//设置控件指示灯
								{
									m_icon_charge.SetIcon(m_hIcon_green);
								}
								else
								{
									m_icon_charge.SetIcon(m_hIcon_gray);
								}
									
                                if(m_soc<=15)
								{
									m_icon_low.SetIcon(m_hIcon_red);
								}
								else
								{
									m_icon_low.SetIcon(m_hIcon_gray);
								}
								if(m_temp1>45&&m_temp1<100)
								{
									m_icon_overtemp.SetIcon(m_hIcon_red);
								}
								else
								{
									m_icon_overtemp.SetIcon(m_hIcon_gray);
								}
								
								m_state = 0;
							}
							break;
						}
					}

				}
				else
				{
					TRACE1("\r\nrecv2: %d",  RECV_BUFFER_SIZE - intRead + intWrite);
					memcpy(m_pbyte_Display_Buffer, m_pbyte_Recv_Buffer + intRead, RECV_BUFFER_SIZE - intRead);
					memcpy(m_pbyte_Display_Buffer + RECV_BUFFER_SIZE - intRead, m_pbyte_Recv_Buffer, intWrite);
					intHexLength = Bytes2HexStr(m_pbyte_Display_Buffer, RECV_BUFFER_SIZE - intRead + intWrite, m_pchar_Display_Hex_Buffer, DISPLAY_HEX_BUFFER_SIZE);
					m_richedit_Com_Recv.SetSel(-1, -1);
					m_richedit_Com_Recv.ReplaceSel(m_pchar_Display_Hex_Buffer);
					m_richedit_Com_Recv.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);

				}

			m_int_Recv_Buffer_Read = intWrite;
		}
		else
		{
			Sleep(10);
		}

	}
	return 0;
}



int CComDlg::DeInitData(void)
{
	//终止线程
	if(INVALID_HANDLE_VALUE != m_hThread_Recv)
	{
		TerminateThread(m_hThread_Recv, 0);
		m_hThread_Recv = INVALID_HANDLE_VALUE;
	}

	if(INVALID_HANDLE_VALUE !=m_hThread_Send)
	{
		TerminateThread(m_hThread_Send, 0);
		m_hThread_Send = INVALID_HANDLE_VALUE;
	}

	if(INVALID_HANDLE_VALUE != m_hThread_Display)
	{
		TerminateThread(m_hThread_Display, 0);
		m_hThread_Display = INVALID_HANDLE_VALUE;
	}

	if(NULL != m_pbyte_Send_Buffer)
	{
		delete [] m_pbyte_Send_Buffer;
		m_pbyte_Send_Buffer = NULL;
	}

	if(NULL != m_pbyte_Recv_Buffer)
	{
		delete [] m_pbyte_Recv_Buffer;
		m_pbyte_Recv_Buffer = NULL;
	}

	if(NULL != m_pbyte_Recv_Buff)
	{
		delete [] m_pbyte_Recv_Buff;
		m_pbyte_Recv_Buff = NULL;
	}

	if(NULL != m_pbyte_Display_Buffer)
	{
		delete [] m_pbyte_Display_Buffer;
		m_pbyte_Display_Buffer = NULL;
	}

	if(NULL != m_pchar_Display_Hex_Buffer)
	{
		delete [] m_pchar_Display_Hex_Buffer;
		m_pchar_Display_Hex_Buffer = NULL;
	}


	if(CFile::hFileNull != m_file_Com_Recv)
	{
		m_file_Com_Recv.Close();
	}

	if(m_bool_Com_Open)
	{
		m_CommCom.Close();
	}

	return 0;
}

int CComDlg::Bytes2HexStr(const void* pvoidSrc, int intVoidSrcLength, char* pcharHexStr, int intCharHexStrLength)//将byte字符串转换成十六进制字符串
{
	BYTE* pbyteSrc = (BYTE*)pvoidSrc;
	int intSrcLength = intVoidSrcLength;
	int intHexStrLength = 0;
	BYTE byteTmp = 0;
	BYTE byteTmpH = 0;
	BYTE byteTmpL = 0;

	if(NULL == pvoidSrc)
	{
		return -1;
	}

	if(NULL == pcharHexStr)
	{
		return -1;
	}

	if(intCharHexStrLength < 3*intVoidSrcLength)
	{
		return -1;
	}

	for(int inti = 0; inti < intSrcLength; inti++)
	{
		byteTmp = *(pbyteSrc + inti);
		byteTmpH = (byteTmp >> 4);
		byteTmpL = (byteTmp & 0x0F);

		if( (byteTmpH >= 0) && (byteTmpH <= 9) )
		{
			*(pcharHexStr + intHexStrLength) = (byteTmpH + '0');
		}
		else
		{
			*(pcharHexStr + intHexStrLength) = (byteTmpH - 10 + 'A');
		}
		intHexStrLength++;

		if( (byteTmpL >= 0) && (byteTmpL <= 9) )
		{
			*(pcharHexStr + intHexStrLength) = (byteTmpL + '0');
		}
		else
		{
			*(pcharHexStr + intHexStrLength) = (byteTmpL - 10 + 'A');
		}
		intHexStrLength++;

		*(pcharHexStr + intHexStrLength) = ' ';
		intHexStrLength++;
	}

	return intHexStrLength;
}

int CComDlg::LockComment(void)//禁用窗口中的控件
{
	m_combo_Com_Port.EnableWindow(FALSE);
	m_combo_Com_BaudRate.EnableWindow(FALSE);
	m_combo_Com_DataBits.EnableWindow(FALSE);
	m_combo_Com_VerifyBits.EnableWindow(FALSE);
	m_combo_Com_StopBits.EnableWindow(FALSE);

	return 0;
}

int CComDlg::UnlockComment(void)
{
	m_combo_Com_Port.EnableWindow(TRUE);
	m_combo_Com_BaudRate.EnableWindow(TRUE);
	m_combo_Com_DataBits.EnableWindow(TRUE);
	m_combo_Com_VerifyBits.EnableWindow(TRUE);
	m_combo_Com_StopBits.EnableWindow(TRUE);

	return 0;
}


LRESULT CComDlg::OnMessageRefreshCount(WPARAM wParam, LPARAM lParam)
{
	UpdateData(FALSE);
	return LRESULT();
}

void CComDlg::OnBnClickedButtonComSendCmd()
{
	// TODO: Add your control notification handler code here
	TRESULT res;

	if(m_bSending)
	{
		return;
	}

	m_bSending = TRUE;

	char* pcharContent = NULL;

	int intContentSize = 20*1024*1024;
	pcharContent = new char[intContentSize];
	
	memset(pcharContent, 0, intContentSize);

	char* pcharContentHex = NULL;
	pcharContentHex = new char[intContentSize];
	memset(pcharContentHex, 0, intContentSize);

	int intContentHexSize = 0;

	m_richedit_Com_Send.GetWindowText(pcharContent, intContentSize);

	int intContentLen = strlen(pcharContent);
TRACE1("\r\nchar-size=%d", intContentLen);
	if(!m_bool_Com_Open)
	{
		::SendMessage(m_StatusBar.GetSafeHwnd(),SB_SETTEXT,(WPARAM)1,(LPARAM)(_T("串口未打开！")));
		goto FLAG_END;
	}

	if(0 == intContentLen)
	{
		::SendMessage(m_StatusBar.GetSafeHwnd(),SB_SETTEXT,(WPARAM)1,(LPARAM)(_T("发送数据为空！")));
		goto FLAG_END;
	}



	UINT intSendSize = 0;
	UINT intSended = 0;

//	if(BST_CHECKED == m_check_Com_SendHex.GetCheck())//十六进制发送
//	{
		HexStr2Bytes(pcharContent, intContentLen, NULL, (int*)(&intSendSize));

		if(intSendSize > 0)
		{
			BYTE* pbyteTmp = new BYTE[intSendSize];
			HexStr2Bytes(pcharContent, intContentLen, pbyteTmp, (int*)(&intSendSize));

			res = m_CommCom.Send(pbyteTmp, intSendSize, &intSended);
	
			if(intSended != intSendSize)
			{
				::SendMessage(m_StatusBar.GetSafeHwnd(),SB_SETTEXT,(WPARAM)1,(LPARAM)(_T("发送失败！")));
			}
			else
			{
				::SendMessage(m_StatusBar.GetSafeHwnd(),SB_SETTEXT,(WPARAM)1,(LPARAM)(_T("发送成功！")));
			}

			m_int_Com_Send_Length += intSended;
			PostMessage(WM_REFRESH_COUNT, 0, 0);
		}

		if(BST_CHECKED == m_chekc_Com_Send_Loop.GetCheck())
		{
			SetTimer(0,m_int_Com_SendPeriod,NULL);
			UpdateData(TRUE);
		}
		else
		{
			KillTimer(0);
		}

FLAG_END:
	if(NULL != pcharContent)
	{
		delete [] pcharContent;
	}

	if(NULL != pcharContentHex)
	{
		delete [] pcharContentHex;
	}

	m_bSending = FALSE;

	return;
}

int CComDlg::HexStr2Bytes(char* pcharHexStr, int intHexStrLen, BYTE* pBytes, int* pintBytesLen)//将十六进制字符串转化为bytes字符串
{
	int intBytesLen = 0;

	int nLen = intHexStrLen;

	char* pcharTmp = new char[2*intHexStrLen + 1];
	memset(pcharTmp, 0, 2*intHexStrLen + 1);
	int intTmpLen = 0;

	//重新排序，组织按照xx空格xx空格的方式组织但是是倒序的。
	for(int i=nLen-1;i>=0;i--)
	{
		if((intTmpLen%3) == 0)
		{
			if( ( (pcharHexStr[i] >= '0') && (pcharHexStr[i ]<= '9') ) || ( (pcharHexStr[i] >= 'a') && (pcharHexStr[i] <= 'f')) || ((pcharHexStr[i] >= 'A') && (pcharHexStr[i] <= 'F')) )
			{
				pcharTmp[intTmpLen++]=pcharHexStr[i];
			}
		}
		else if((intTmpLen%3) == 1)
		{
			if( ( (pcharHexStr[i] >= '0') && (pcharHexStr[i] <= '9')) || ((pcharHexStr[i] >= 'a') && (pcharHexStr[i] <= 'f')) || ((pcharHexStr[i] >= 'A') && (pcharHexStr[i] <= 'F')))
			{
				pcharTmp[intTmpLen] = pcharHexStr[i];
				intTmpLen++;
			}
			else if(pcharHexStr[i] == ' ')
			{
				pcharTmp[intTmpLen++] = '0';
				pcharTmp[intTmpLen++] = ' ';
			}
			else
			{
				pcharTmp[--intTmpLen] = 0;
			}
		}
		else if((intTmpLen%3) == 2)
		{
			if(pcharHexStr[i] == ' ')
			{
				pcharTmp[intTmpLen++] = ' ';
			}
		}
	}

	int intBufferTmpLen = intTmpLen + 1;
	char* pcharBufferTmp = new char[intBufferTmpLen + 1];
	memset(pcharBufferTmp, 0, intBufferTmpLen + 1);

	if((intTmpLen%3) == 1)
	{
		pcharBufferTmp[0] = '0';

		for(int i = 1;i < intTmpLen + 1; i++)
		{
			pcharBufferTmp[i] = pcharTmp[intTmpLen-i];
		}

		intTmpLen++;
	}
	else
	{
		for(int i = 0;i < intTmpLen; i++)
		{
			pcharBufferTmp[i] = pcharTmp[intTmpLen-1-i];
		}
	}

	intTmpLen += 1;

	if(NULL == pBytes)
	{
		*pintBytesLen = (intTmpLen / 3);
	}
	else
	{
		nLen = intTmpLen/3;

		if(nLen > *pintBytesLen)
		{
			nLen = *pintBytesLen;
		}

		for(int i = 0; i < nLen; i++)
		{
			BYTE nByte = 0;

			if((pcharBufferTmp[i*3+0] >= '0') && (pcharBufferTmp[i*3+0] <= '9'))
			{
				nByte += (BYTE)(pcharBufferTmp[i*3+0]-'0')*16;
			}
			else if((pcharBufferTmp[i*3+0] >= 'a') && (pcharBufferTmp[i*3+0] <= 'f'))
			{
				nByte += (BYTE)(pcharBufferTmp[i*3+0]-'a'+10)*16;
			}
			else// if(pcharBufferTmp[i*3+0]>='a'&&pcharBufferTmp[i*3+0]<='f')
			{
				nByte += (BYTE)(pcharBufferTmp[i*3+0]-'A'+10)*16;
			}





			if((pcharBufferTmp[i*3+1] >= '0') && (pcharBufferTmp[i*3+1] <= '9'))
			{
				nByte += (BYTE)(pcharBufferTmp[i*3+1]-'0');
			}
			else if((pcharBufferTmp[i*3+1] >= 'a') && (pcharBufferTmp[i*3+1] <= 'f'))
			{
				nByte += (BYTE)(pcharBufferTmp[i*3+1]-'a'+10);
			}
			else// if(pcharBufferTmp[i*3+0]>='a'&&pcharBufferTmp[i*3+0]<='f')
			{
				nByte += (BYTE)(pcharBufferTmp[i*3+1]-'A'+10);
			}

			*(pBytes + i) = nByte;
		}
	}

	if(NULL != pcharTmp)
	{
		delete [] pcharTmp;
		pcharTmp = NULL;
	}

	if(NULL != pcharBufferTmp)
	{
		delete [] pcharBufferTmp;
		pcharBufferTmp = NULL;
	}

	return 0;
}

void CComDlg::OnBnClickedButtonComCleardisplay()//刷新按钮
{
	// TODO: Add your control notification handler code here
	m_int_Com_Send_Length = 0;
	m_int_Com_Recv_Length = 0;
	m_int_Recv_Buffer_Read = 0;
	m_int_Recv_Buffer_Write = 0;

	m_richedit_Com_Recv.SetSel(0, -1);
	m_richedit_Com_Recv.Clear();
	m_richedit_Com_Recv.SetSel(0, 0);

	if(CFile::hFileNull != m_file_Com_Recv)
	{
		m_file_Com_Recv.SetLength(0);
		::SendMessage(m_StatusBar.GetSafeHwnd(),SB_SETTEXT,(WPARAM)1,(LPARAM)(_T("写入内容已清空！")));
	}
	else
	{
		::SendMessage(m_StatusBar.GetSafeHwnd(),SB_SETTEXT,(WPARAM)1,(LPARAM)(_T("写入已关闭，未清空！")));
	}

	PostMessage(WM_REFRESH_COUNT, 0, 0);
}




void CComDlg::OnTimer(UINT_PTR nIDEvent)//计时器
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	switch(nIDEvent)
	{
	case 0:
		OnBnClickedButtonComSendCmd();
		break;
	}
		
	CDialog::OnTimer(nIDEvent);
}
