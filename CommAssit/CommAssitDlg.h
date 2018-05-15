
// CommAssitDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "ComDlg.h"

// CCommAssitDlg 对话框
class CCommAssitDlg : public CDialog
{
// 构造
public:
	CCommAssitDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_COMMASSIT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CComDlg m_dlg_Com;
	CTabCtrl m_Tab;
	afx_msg void OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult);
};
