#pragma once

typedef enum _TRESULT{TRESULT_OK = 0, TRESULT_OPENED, TRESULT_OCCUPIED, TRESULT_SETTINGERROR, TRESULT_TIMEOUT, TRESULT_TRANSERROR, TRESULT_DATAERROR, TRESULT_ALREADYCLOSE, TRESULT_CLOSEERROR}TRESULT;

#define MAC_PROTOCOL_LOW    0x08
#define MAC_PROTOCOL_HI		0x88

#define PCAP_FILTER		    "ether proto 0x0888"

#define TYPE_MAC_PKT_ACK		0x1
#define TYPE_MAC_PKT_ASYN		0x2
#define TYPE_MAC_PKT_UPGRADE	0x3
#define TYPE_MAC_PKT_SYN		0x4

#define MAXSIZE_MSGBODY         512
#define TCP_MAXSEGSZ	        1460

typedef struct _tagLMsgFormat{
	BYTE dmac[6];
	BYTE smac[6];
	BYTE type[2];
	BYTE sign;
	BYTE reserved1;
	WORD len;
	DWORD counter;
	BYTE  body[MAXSIZE_MSGBODY];
}LMSGFORMAT,*PLMSGFORMAT;

class CComm
{
public:
	CComm(void);
	~CComm(void);

	virtual UINT SetTimeOut(UINT timeOut) = 0;

	virtual TRESULT Open();
	virtual TRESULT Close();

	virtual TRESULT Send(const void *pbuf, UINT size, UINT *pRealSize = NULL);
	virtual TRESULT Recv(void *pbuf, UINT maxsize, UINT *pRealSize = NULL);
};

//CCommCOM
class CCommCOM : public CComm
{
public:
//	CCommCOM(LPCTSTR port, DWORD baudRate, BYTE dataBits, BYTE parity, BYTE stopBits, DWORD timeOut);
	CCommCOM(void);
	~CCommCOM(void);

	virtual UINT SetTimeOut(UINT timeOut);

	virtual TRESULT Open();
	virtual TRESULT Close();

	virtual TRESULT Send(const void *pbuf, UINT size, UINT *pRealSize = NULL);
	virtual TRESULT Recv(void *pbuf, UINT maxsize, UINT *pRealSize = NULL);

	TRESULT CCommCOM::SetPara(char* pcharCom, DWORD dwordBaudRate, BYTE byteDataBits, BYTE byteVerifyBits, BYTE byteStopBits, DWORD dwordTimeouts);

protected:
	HANDLE    m_ptrCom;

	CString	  m_strSerialName;
	DWORD	  m_dwBaudRate;
	BYTE      m_byDataBits;
	BYTE      m_byParity;
	BYTE      m_byStopBits;
	DWORD     m_dwTimeOut;
	OVERLAPPED m_overap_Read;
	OVERLAPPED m_overap_Write;
};


//CCommPcap
class CCommPcap : public CComm
{
public:
	CCommPcap(const char *source, const char* dst_mac, int snaplen, int read_timeout, BYTE sign);
	~CCommPcap(void);

	virtual UINT SetTimeOut(UINT timeOut);

	virtual TRESULT Open();
	virtual TRESULT Close();

	virtual TRESULT Send(const void *pbuf, UINT size, UINT *pRealSize = NULL);
	virtual TRESULT Recv(void *pbuf, UINT maxsize, UINT *pRealSize = NULL);

protected:
	PVOID     m_ptrPcap;

	CStringA  m_strPcapSource;
	BYTE	  m_byDstMac[6];
	int	      m_nSnapLen;
	int       m_nRead_timeout;
	BYTE      m_bySign;
	DWORD     m_dwCounter;

protected:
	BOOL GetMacAddress(const char* source, char* mac_buf, UINT size_buf);
};


//CCommWan
class CCommWan : public CComm
{
public:
	CCommWan(void);
	~CCommWan(void);

	virtual UINT SetComm(const char* lpszAddr, WORD wRemPort, BOOL bUseDomain, UINT nTimeout);

	virtual UINT SetTimeOut(UINT timeOut);
	virtual UINT GetTimeOut();

	virtual TRESULT Open();
	virtual TRESULT Close();

	virtual TRESULT Send(const void *pbuf, UINT size, UINT *pRealSize = NULL);
	virtual TRESULT Recv(void *pbuf, UINT maxsize, UINT *pRealSize = NULL);

protected:
	PVOID     m_ptrSock;

	CStringA  m_strAddr;
	WORD	  m_wRemPort;
	BOOL	  m_bUseDomain;
	UINT      m_nTimeOut;
public:
	unsigned long m_ulong_Local_IP;
	unsigned short m_ushort_Local_Port;
};
