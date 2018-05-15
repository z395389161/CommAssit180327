
// CommAssitDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "CommAssit.h"
#include "CommAssitDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCommAssitDlg 对话框




CCommAssitDlg::CCommAssitDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCommAssitDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCommAssitDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB, m_Tab);
}

BEGIN_MESSAGE_MAP(CCommAssitDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, &CCommAssitDlg::OnTcnSelchangeTab)
END_MESSAGE_MAP()


// CCommAssitDlg 消息处理程序

BOOL CCommAssitDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_Tab.InsertItem(0, "串口通讯");
	m_dlg_Com.Create(IDD_COM_DIALOG, &m_Tab);

	CRect clientRect;
	m_Tab.GetWindowRect(clientRect);
	clientRect.DeflateRect(0, 0, 30, 60);
	m_dlg_Com.MoveWindow(clientRect);

	m_dlg_Com.ShowWindow(SW_SHOW);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CCommAssitDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CCommAssitDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CCommAssitDlg::OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here

	int intCurSel = 0;
	m_dlg_Com.ShowWindow(SW_SHOW);

	*pResult = 0;
}
