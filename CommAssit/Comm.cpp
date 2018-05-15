#include "StdAfx.h"
#include "Comm.h"

CComm::CComm(void)
{
}

CComm::~CComm(void)
{
}

TRESULT CComm::Open()
{
	return TRESULT_OK;
}

TRESULT CComm::Close()
{
	return TRESULT_OK;
}

TRESULT CComm::Send(const void *pbuf, UINT size, UINT *pRealSize/* = NULL*/)
{
	return TRESULT_OK;
}

TRESULT CComm::Recv(void *pbuf, UINT maxsize, UINT *pRealSize/* = NULL*/)
{
	return TRESULT_OK;
}

//CCommCOM

//CCommCOM::CCommCOM(LPCTSTR port, DWORD baudRate, BYTE dataBits, BYTE parity, BYTE stopBits, DWORD timeOut)
CCommCOM::CCommCOM(void)
{
	m_ptrCom = INVALID_HANDLE_VALUE;

	//m_strSerialName = port;
	//m_dwBaudRate = baudRate;
	//m_byDataBits = dataBits;
	//m_byParity = parity;
	//m_byStopBits = stopBits;
	//m_dwTimeOut = timeOut;
}


CCommCOM::~CCommCOM(void)
{
	Close();
}

UINT CCommCOM::SetTimeOut(UINT timeOut)
{
	UINT ret = m_dwTimeOut;
	m_dwTimeOut = timeOut;

	return ret;
}

TRESULT CCommCOM::SetPara(char* pcharCom, DWORD dwordBaudRate, BYTE byteDataBits, BYTE byteVerifyBits, BYTE byteStopBits, DWORD dwordTimeouts)
{
	m_strSerialName = pcharCom;
	m_dwBaudRate = dwordBaudRate;
	m_byDataBits = byteDataBits;
	m_byParity = byteVerifyBits;
	m_byStopBits = byteStopBits;
	m_dwTimeOut = dwordTimeouts;

	return TRESULT_OK;
}

TRESULT CCommCOM::Open()
{
	// Make sure that channel is closed
	if (m_ptrCom != INVALID_HANDLE_VALUE)
	{
		return TRESULT_OPENED;
	}

	CString strPort;

	if(m_strSerialName[0] == 'C' || m_strSerialName[0] == 'c')
	{
		strPort.Format(_T("\\\\.\\%s"), m_strSerialName);
	}
	else
	{
		strPort = m_strSerialName;
	}

	// Open com port
//	m_ptrCom = CreateFileA(strPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
m_ptrCom = CreateFileA(strPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if(m_ptrCom == INVALID_HANDLE_VALUE)
	{
		return TRESULT_OCCUPIED;
	}

	// Set communication buffers
	if (!SetupComm(m_ptrCom, 4096, 4096))
	{
		return TRESULT_SETTINGERROR;
	}

	// Set the serial communications device
	DCB dcb;
	ZeroMemory (&dcb, sizeof(DCB));
	dcb.DCBlength = sizeof(DCB);
	dcb.BaudRate = m_dwBaudRate;
	dcb.fBinary = 1;
	dcb.fDtrControl = DTR_CONTROL_DISABLE;
	dcb.fRtsControl = RTS_CONTROL_DISABLE;
	dcb.ByteSize = m_byDataBits;
	dcb.Parity = m_byParity;
	dcb.StopBits = m_byStopBits;

	if (!SetCommState(m_ptrCom, &dcb))
	{
		return TRESULT_SETTINGERROR;
	}

	// Set the serial communications timeout
	COMMTIMEOUTS ctmo;
	ZeroMemory (&ctmo, sizeof(COMMTIMEOUTS));
	ctmo.ReadIntervalTimeout = m_dwTimeOut; //任意相邻连个字符之间的超时设置
	ctmo.ReadTotalTimeoutMultiplier = m_dwTimeOut;//写操作总的超时时间的系数
	ctmo.ReadTotalTimeoutConstant = m_dwTimeOut;//读操作总的超时时间的修正常量
	ctmo.WriteTotalTimeoutConstant = m_dwTimeOut;
	ctmo.WriteTotalTimeoutMultiplier = m_dwTimeOut;


	if (!SetCommTimeouts(m_ptrCom, &ctmo))
	{
		return TRESULT_SETTINGERROR;
	}

	if (m_overap_Read.hEvent != NULL)
	{
		ResetEvent(m_overap_Read.hEvent);
	}

	if (m_overap_Write.hEvent != NULL)
	{
		ResetEvent(m_overap_Write.hEvent);
	}

	ZeroMemory(&m_overap_Read,sizeof(m_overap_Read));
	ZeroMemory(&m_overap_Write,sizeof(m_overap_Write));

	m_overap_Read.hEvent = CreateEvent(NULL,TRUE,TRUE,NULL);
	m_overap_Write.hEvent = CreateEvent(NULL,TRUE,TRUE,NULL);
	
	if (m_overap_Read.hEvent == NULL)
	{
		return TRESULT_SETTINGERROR;
	}

	if (m_overap_Write.hEvent == NULL)
	{
		return TRESULT_SETTINGERROR;
	}

	SetCommMask(m_ptrCom, EV_RXCHAR);

	return TRESULT_OK;
}

TRESULT CCommCOM::Close()
{
//	TRACE1("\r\nINVALID_HANDLE_VALUE=0x%x", INVALID_HANDLE_VALUE);
	if (m_ptrCom != INVALID_HANDLE_VALUE)
	{
		if (CloseHandle (m_ptrCom))
		{
			m_ptrCom = INVALID_HANDLE_VALUE;

			if (m_overap_Read.hEvent != NULL)
			{
				ResetEvent(m_overap_Read.hEvent);
			}

			if (m_overap_Write.hEvent != NULL)
			{
				ResetEvent(m_overap_Write.hEvent);
			}

			ZeroMemory(&m_overap_Read,sizeof(m_overap_Read));
			ZeroMemory(&m_overap_Write,sizeof(m_overap_Write));

			return TRESULT_OK;
		}
		else
		{
			return TRESULT_CLOSEERROR;
		}
	}
	else
	{
		return TRESULT_ALREADYCLOSE;
	}
}






//TRESULT CCommCOM::Send(const void *pbuf, UINT size, UINT *pRealSize/* = NULL*/)
//{
//	DWORD dwError;
//	*pRealSize = 0;
//	DWORD dwWritten = 0;
//
//	if (m_ptrCom == INVALID_HANDLE_VALUE)
//	{
//		return TRESULT_OCCUPIED;
//	}
//
//	if (!WriteFile(m_ptrCom, pbuf, size, &dwWritten, &m_overap_Write))
//	{
//		while (!GetOverlappedResult(m_ptrCom, &m_overap_Write, &dwWritten, TRUE))
//		{
//			TRACE1("\r\nwrite=%d", dwWritten);
//			Sleep((size - dwWritten)/50);
//		}
//	}
//
//	if (dwWritten == 0)
//	{
//		return TRESULT_TIMEOUT;
//	}
//
//	if (pRealSize)
//	{
//		*pRealSize = dwWritten;
//	}
//
//	if (dwWritten != size)
//	{
//		return TRESULT_DATAERROR;
//	}
//
//	return TRESULT_OK;
//}





TRESULT CCommCOM::Send(const void *pbuf, UINT size, UINT *pRealSize/* = NULL*/)
{
	DWORD dwError;
	*pRealSize = 0;
	DWORD dwWritten = 0;

	COMSTAT comstat;

	memset(&comstat, 0, sizeof(comstat));

	if (m_ptrCom == INVALID_HANDLE_VALUE)
	{
		return TRESULT_OCCUPIED;
	}

	if (!WriteFile(m_ptrCom, pbuf, size, &dwWritten, &m_overap_Write))
	{
//		while (!GetOverlappedResult(m_ptrCom, &m_overap_Write, &dwWritten, TRUE))
//		{
//			TRACE1("\r\nwrite=%d", dwWritten);
//			Sleep((size - dwWritten)/50);
//		}

		while(ClearCommError(m_ptrCom, &dwError, &comstat))
		{
		//	TRACE1("\r\nremain=%d", comstat.cbOutQue);
			if(comstat.cbOutQue)
			{
				Sleep((comstat.cbOutQue)/100);
			}
			else
			{
				dwWritten = size;
				break;
			}
		}
	}

	if (dwWritten == 0)
	{
		return TRESULT_TIMEOUT;
	}

	if (pRealSize)
	{
		*pRealSize = dwWritten;
	}

	if (dwWritten != size)
	{
		return TRESULT_DATAERROR;
	}

	return TRESULT_OK;
}


TRESULT CCommCOM::Recv(void *pbuf, UINT maxsize, UINT *pRealSize/* = NULL*/)
{
	DWORD dwError;
	DWORD dwRead = 0;
	*pRealSize = 0;
	COMSTAT ComStat;

	memset(&ComStat, 0, sizeof(ComStat));

	if (m_ptrCom == INVALID_HANDLE_VALUE)
	{
		return TRESULT_OCCUPIED;
	}

//FLAG_START:
	BOOL bres = ClearCommError(m_ptrCom, &dwError, &ComStat);
//	TRACE3("\r\nrecv res = %d, len = %d, err = %d", bres, ComStat.cbInQue, GetLastError());

	if(ComStat.cbInQue > 0)
	{
//		TRACE1("\r\nmaxsize=%d", maxsize);
//		TRACE1("\r\nInQue=%d", ComStat.cbInQue);
		if(maxsize > ComStat.cbInQue)
		{
			maxsize = ComStat.cbInQue;
		}

		BOOL bres = ReadFile(m_ptrCom, pbuf, maxsize, &dwRead, &m_overap_Read);
		if((!bres) && (GetLastError() == ERROR_IO_PENDING) )
		{
			if(WaitForSingleObject(m_overap_Read.hEvent, 500) == WAIT_OBJECT_0)
			{
				bres = GetOverlappedResult(m_ptrCom, &m_overap_Read, &dwRead, FALSE);
			//	goto FLAG_START;
			}
		}
	}

	if (dwRead <= 0)
	{
		return TRESULT_TIMEOUT;
	}

	if (pRealSize)
	{
		*pRealSize = dwRead;
	}

	if (dwRead != maxsize)
	{
		return TRESULT_DATAERROR;
	}

	return TRESULT_OK;
}





//
//
//TRESULT CCommCOM::Recv(void *pbuf, UINT maxsize, UINT *pRealSize/* = NULL*/)
//{
//	DWORD dwError;
//	DWORD dwRead = 0;
//	*pRealSize = 0;
//
//	if (m_ptrCom == INVALID_HANDLE_VALUE)
//	{
//		return TRESULT_OCCUPIED;
//	}
//
//	if(!ClearCommError(m_ptrCom, &dwError, NULL))
//	{
//		return TRESULT_TRANSERROR;
//	}
//
//	if(!ReadFile(m_ptrCom, pbuf, maxsize, &dwRead, &m_overap_Read))
//	{
//		while(!GetOverlappedResult(m_ptrCom, &m_overap_Read, &dwRead, FALSE))
//		{
//		//	Sleep(20);
//			continue;
//		}
//	}
//
//	if (dwRead <= 0)
//	{
//		return TRESULT_TIMEOUT;
//	}
//
//	if (pRealSize)
//	{
//		*pRealSize = dwRead;
//	}
//
//	if (dwRead != maxsize)
//	{
//		return TRESULT_DATAERROR;
//	}
//
//	return TRESULT_OK;
//}








//CCommWan

CCommWan::CCommWan(void)
{
	
}


CCommWan::~CCommWan(void)
{
	Close();
}


UINT CCommWan::SetComm(const char* lpszAddr, WORD wRemPort, BOOL bUseDomain, UINT nTimeout)
{
	m_ptrSock = NULL;

	m_strAddr = lpszAddr;
	m_wRemPort = wRemPort;
	m_bUseDomain = bUseDomain;
	m_nTimeOut = nTimeout;

	return 1;
}


UINT CCommWan::SetTimeOut(UINT timeOut)
{
	UINT ret = m_nTimeOut;
	m_nTimeOut = timeOut;

	return ret;
}

UINT CCommWan::GetTimeOut()
{
	return m_nTimeOut;
}

int getpeeraddr(SOCKET& sockfd)
{
    struct sockaddr_in peeraddr;
    int len = sizeof(peeraddr);

    int ret = getsockname(sockfd, (struct sockaddr *)&peeraddr, &len);
    if (ret < 0)
        return 1;
	return 0;
}

BOOL SockConnect(SOCKET& sock, ULONG ulIP, WORD wPort, UINT nTimeout)
{
	BOOL ret = FALSE;

	//fill sockaddr_in
	struct sockaddr_in  serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = ulIP;
	serv_addr.sin_port = htons(wPort);

	unsigned long ul = 1;
	ioctlsocket(sock, FIONBIO, &ul); //设置为非阻塞模式
	
	if ( connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR )
	{
		//fill timeval
		timeval tm;
		tm.tv_sec  = nTimeout / 1000;
		tm.tv_usec = (nTimeout % 1000) * 1000;

		//fill fd_set
		fd_set set;
		FD_ZERO(&set);
		FD_SET(sock, &set);

		if( select(sock + 1, NULL, &set, NULL, &tm) > 0 )
		{
			int nError = -1;
			int nLen = sizeof(int);

			getsockopt(sock, SOL_SOCKET, SO_ERROR, (char *)&nError, /*(socklen_t *)*/&nLen);
			ret = (nError == 0) ? TRUE : FALSE;
		} 
		else 
		{
			ret = FALSE;
		}
	}
	else 
	{
		ret = TRUE;
	}

	ul = 0;
	ioctlsocket(sock, FIONBIO, &ul); //设置为阻塞模式

	if (ret)
	{
		setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&nTimeout, sizeof(nTimeout));
		setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&nTimeout, sizeof(nTimeout));
	}

	return ret;
}

TRESULT CCommWan::Open()
{
	struct sockaddr_in client_addr;
	int intLen = sizeof(client_addr);
	int intResult = 0;

	if (m_ptrSock)
	{
		return TRESULT_OPENED;
	}

	// Create socket
	if ((m_ptrSock = (LPVOID)socket(AF_INET, SOCK_STREAM, 0)) == NULL)
	{
		return TRESULT_OCCUPIED;
	}

	//get ip
	ULONG ulIP;

	if (m_bUseDomain)
	{
		HOSTENT* pHost = NULL;

		if ( m_strAddr.IsEmpty() || ((pHost = gethostbyname(m_strAddr)) == NULL) )
		{
			ulIP = INADDR_BROADCAST;
		}
		else
		{
			CopyMemory(&ulIP, pHost->h_addr_list[0], pHost->h_length);
		}
	}
	else
	{
		ulIP = inet_addr(m_strAddr);

		if ( ulIP == 0 )
		{
			return TRESULT_SETTINGERROR;
		}
	}

	if (!SockConnect((SOCKET&)m_ptrSock, ulIP, m_wRemPort, m_nTimeOut))
	{
		return TRESULT_SETTINGERROR;
	}

	//int nTrycount = 0;

	//while (!SockConnect((SOCKET&)m_ptrSock, ulIP, m_wRemPort, m_nTimeOut))
	//{
	//	if (nTrycount++ >= 4)
	//	{
	//		return TRESULT_SETTINGERROR;
	//	}
	//}

//	int intres = getpeeraddr((SOCKET)m_ptrSock);

	//获取本地ip以及端口号
	intResult = getsockname((SOCKET&)m_ptrSock, (struct sockaddr *)(&client_addr), &intLen);
	if(0 == intResult)
	{
		m_ulong_Local_IP = client_addr.sin_addr.s_addr;
		m_ushort_Local_Port = htons(client_addr.sin_port);
	}

	return TRESULT_OK;
}

TRESULT CCommWan::Close()
{
	if (m_ptrSock)
	{
		if (closesocket((SOCKET)m_ptrSock) == 0)
		{
			m_ptrSock = NULL;

			return TRESULT_OK;
		}
		else
		{
			return TRESULT_CLOSEERROR;
		}
	}
	else
	{
		return TRESULT_ALREADYCLOSE;
	}
}

TRESULT CCommWan::Send(const void *pbuf, UINT size, UINT *pRealSize/* = NULL*/)
{
	if (m_ptrSock == NULL)
	{
		return TRESULT_OCCUPIED;
	}

	int res = send((SOCKET)m_ptrSock, (const char *)pbuf, size, 0);

	if (res == 0)
	{
		return TRESULT_TIMEOUT;
	}

	if (res < 0)
	{
		return TRESULT_TRANSERROR;
	}

	if (pRealSize)
	{
		*pRealSize = res;
	}

	return TRESULT_OK;
}

TRESULT CCommWan::Recv(void *pbuf, UINT maxsize, UINT *pRealSize/* = NULL*/)
{
	if (m_ptrSock == NULL)
	{
		return TRESULT_OCCUPIED;
	}

	int res = recv((SOCKET)m_ptrSock, (char *)pbuf, maxsize, 0);

	if (res == 0)
	{
		return TRESULT_TIMEOUT;
	}

	if (res < 0)
	{
		return TRESULT_TRANSERROR;
	}

	if (pRealSize)
	{
		*pRealSize = res;
	}

	return TRESULT_OK;
}


