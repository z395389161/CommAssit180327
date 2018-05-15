#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "Comm.h"
#include "EnumSerial.h"

// CComDlgdialog

class CComDlg: public CDialog
{
	DECLARE_DYNAMIC(CComDlg)

public:
	CComDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CComDlg();

// Dialog Data
	enum { IDD = IDD_COM_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CCommCOM m_CommCom; 

	CStatusBar m_StatusBar;
	CProgressCtrl m_Progress;

	BOOL m_bool_Com_Open;
	BOOL m_bEndSend;

	BYTE* m_pbyte_Recv_Buffer;
	BYTE* m_pbyte_Send_Buffer;
	BYTE* m_pbyte_Recv_Buff;
	BYTE* m_pbyte_Display_Buffer;
	char* m_pchar_Display_Hex_Buffer;
	BOOL m_bSending;
	BYTE m_byte_data[18];
	int m_state;

	char* m_battery_current;
	char* m_battery_voltage;
	char* m_battery_soc;
	char* m_battery_temp;
	char* m_battery_quantity;
	char* m_battery_index;
	char* m_battery_judge;
	char* m_battery_state1;
	char* m_battery_state2;
	char* m_battery_state0;
	char* m_battery_state;


	int m_int_Recv_Buffer_Read;
	int m_int_Recv_Buffer_Write;

	HANDLE m_hThread_Send;
	HANDLE m_hThread_Recv;
	HANDLE m_hThread_Display;

	CFile m_file_Com_Recv;
	CString m_cstring_Com_RecvPath;

	CComboBox m_combo_Com_Port;
	CComboBox m_combo_Com_BaudRate;
	CComboBox m_combo_Com_DataBits;
	CComboBox m_combo_Com_VerifyBits;
	CComboBox m_combo_Com_StopBits;
	CButton m_button_Com_Open;
	CButton m_check_Com_RecvHex;
	CButton m_check_Com_SendHex;
	CButton m_chekc_Com_Send_Loop;
	int m_int_Com_SendPeriod;
	CRichEditCtrl m_richedit_Com_Recv;
	CRichEditCtrl m_richedit_Com_Send;
	int m_int_Com_Send_Length;
	int m_int_Com_Recv_Length;
	afx_msg void OnBnClickedButtonComOpen();
	afx_msg void OnBnClickedButtonComRefreshport();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	int ComRecv(void);
	int ComDisplay(void);
	int DeInitData(void);
	int Bytes2HexStr(const void* pvoidSrc, int intVoidSrcLength, char* pcharHexStr, int intCharHexStrLength);
	int LockComment(void);
	int UnlockComment(void);
	LRESULT OnMessageRefreshCount(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedButtonComSendCmd();
	int HexStr2Bytes(char* pcharHexStr, int intHexStrLen, BYTE* pBytes, int* pintBytesLen);
	afx_msg void OnBnClickedButtonComCleardisplay();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CEdit m_com_current;
	CEdit m_com_soc;
	CEdit m_com_index;
	CEdit m_com_quantity;
	CEdit m_com_voltage;
	CEdit m_com_temp;

	CString m_str_current;
	CString m_str_soc;
	CString m_str_index;
	CString m_str_quantity;
	CString m_str_voltage;
	CString m_str_temp;
	CStatic m_icon_low;
	CStatic m_icon_overtemp;
	CStatic m_icon_overpower;
	CStatic m_icon_charge;
	HICON m_hIcon_green;
	HICON m_hIcon_red;
	HICON m_hIcon_gray;
};
