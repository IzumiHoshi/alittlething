#include "StdAfx.h"
#include ".\sockettcpclient.h"

CSocketTCPClient::CSocketTCPClient(void)
: m_dwPendingIOCount(0)
{
	m_hSocket = INVALID_SOCKET;
	m_hEvent = WSA_INVALID_EVENT;
	m_hWndParent = NULL;
	memset(&m_saRemote, 0, sizeof(SOCKADDR_IN));
	memset(&m_perIODataRead, 0, sizeof(PER_IO_DATA));
	memset(&m_perIODataWrite, 0, sizeof(PER_IO_DATA));
}

CSocketTCPClient::~CSocketTCPClient(void)
{
	Close();
}

BOOL CSocketTCPClient::Open(LPCTSTR szIP, u_short usRemotePort, HWND hWndParent, BOOL bEvent)
{
	DWORD dwIP = 0;
	if(szIP)
	{
		dwIP = inet_addr(szIP);
	}
	return Open(ntohl(dwIP), usRemotePort, hWndParent, bEvent);
}

BOOL CSocketTCPClient::Open(DWORD dwRemoteIP, u_short usRemotePort, HWND hWndParent, BOOL bEvent)
{
	WSADATA wsaData;
	int nRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
	m_hSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, 0);
	if(m_hSocket == INVALID_SOCKET)
	{
		WSACleanup();
		return FALSE;
	}
	BOOL lVal = 1;
	setsockopt(m_hSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&lVal, sizeof(BOOL));
	setsockopt(m_hSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&lVal, sizeof(BOOL)); 
	int iValue =  2* 1024 * 1024;
	setsockopt(m_hSocket, SOL_SOCKET, SO_SNDBUF, (CHAR*)&iValue, sizeof(iValue));//设置发送缓冲区
	setsockopt(m_hSocket, SOL_SOCKET, SO_RCVBUF, (CHAR*)&iValue, sizeof(iValue));//设置接受缓冲区
	memset(&m_saRemote, 0, sizeof(SOCKADDR_IN));
	m_saRemote.sin_family = AF_INET;
	m_saRemote.sin_addr.s_addr = htonl(dwRemoteIP);//inet_addr("192.168.2.98");
	m_saRemote.sin_port = htons(usRemotePort);

	if (bEvent)
	{	
		m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		if(m_hEvent == WSA_INVALID_EVENT)
		{
			closesocket(m_hSocket);
			WSACleanup();
			return FALSE;
		}
		WSAEventSelect(m_hSocket, m_hEvent, FD_CONNECT | FD_READ | FD_CLOSE);
	}

	m_hWndParent = hWndParent;
	return TRUE;
}

BOOL CSocketTCPClient::Close(void)
{
	if(m_hSocket != INVALID_SOCKET)
	{
		DisConnect();
		closesocket(m_hSocket);
		m_hSocket = INVALID_SOCKET;
		WSACleanup();
	}
	if(m_hEvent != WSA_INVALID_EVENT)
	{
		CloseHandle(m_hEvent);
		m_hEvent = WSA_INVALID_EVENT;
	}
	return TRUE;
}

int CSocketTCPClient::Connect(void)
{
	int iRet = WSAConnect(m_hSocket, (SOCKADDR *)&m_saRemote, sizeof(SOCKADDR_IN), NULL, NULL, NULL, NULL);

	return iRet;
}

int CSocketTCPClient::DisConnect(void)
{
	shutdown(m_hSocket, SD_BOTH);
	return 0;
}


int CSocketTCPClient::Receive(char* pBuf, int iLength)
{
	return recv(m_hSocket, pBuf, iLength, 0);
}

int CSocketTCPClient::SendNoTry(char* pBuf,int iLength)  //发送数据与Send功能一样，不重试
{
	return send(m_hSocket, pBuf, iLength, 0);
}


int CSocketTCPClient::Send(char* pBuf, int iLength)
{
	int nRet = 0;
	int nSend = 0;
	int nTryCount = 200;
	while(iLength && nTryCount >= 0)
	{
		nSend = send(m_hSocket, pBuf, iLength, 0);
		if(nSend >= 0)
		{
			iLength -= nSend;
			pBuf += nSend;
			nRet += nSend;
			nTryCount = 200;//!! 发送成功 nTryCount = -1;   2012.10.08
		}
		else
		{
			DWORD dwError = WSAGetLastError();
			if(dwError != WSAEWOULDBLOCK)
			{
				break;
			}
			Sleep(50);
			nTryCount --;
		}
	}
	return nRet;
}

BOOL CSocketTCPClient::OpenEx(DWORD dwRemoteIP, u_short usRemotePort, HWND hWndParent)
{
	WSADATA wsaData;
	int nRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
	m_hSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if(m_hSocket == INVALID_SOCKET)
	{
		WSACleanup();
		return FALSE;
	}
	BOOL lVal = 1;
	setsockopt(m_hSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&lVal, sizeof(BOOL));
	lVal = 0;
	//setsockopt(m_hSocket, SOL_SOCKET, SO_RCVBUF, (char*)&lVal, sizeof(BOOL));

	memset(&m_saRemote, 0, sizeof(SOCKADDR_IN));
	m_saRemote.sin_family = AF_INET;
	m_saRemote.sin_addr.s_addr = htonl(dwRemoteIP);//inet_addr("192.168.2.98");
	m_saRemote.sin_port = htons(usRemotePort);

	m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if(m_hEvent == WSA_INVALID_EVENT)
	{
		closesocket(m_hSocket);
		m_hSocket = INVALID_SOCKET;
		WSACleanup();
		return FALSE;
	}
	WSAEventSelect(m_hSocket, m_hEvent, FD_CONNECT | FD_CLOSE);
	m_hWndParent = hWndParent;
	return TRUE;
}

int CSocketTCPClient::ReceiveEx(char* pBuf, int iLength)
{
	InterlockedIncrement((LPLONG)&m_dwPendingIOCount);
	memset(&m_perIODataRead, 0, sizeof(PER_IO_DATA));

	m_perIODataRead.wsaBuf.buf = pBuf;
	m_perIODataRead.wsaBuf.len = iLength;
	m_perIODataRead.dwBytesTransferred = iLength;
	m_perIODataRead.dwIOType = IO_TYPE_READ;
	return WSARecv(m_hSocket, &m_perIODataRead.wsaBuf, 1, &(m_perIODataRead.dwBytesTransferred), &m_perIODataRead.dwFlags, &(m_perIODataRead.wsaOverLapped), NULL);
}

int CSocketTCPClient::SendEx(char* pBuf, int iLength)
{
	memset(&m_perIODataWrite, 0, sizeof(PER_IO_DATA));
	m_perIODataWrite.wsaBuf.buf = pBuf;
	m_perIODataWrite.wsaBuf.len = iLength;
	m_perIODataWrite.dwBytesTransferred = iLength;
	m_perIODataWrite.dwIOType = IO_TYPE_WRITE;
	return WSASend(m_hSocket, &m_perIODataWrite.wsaBuf, 1, &m_perIODataWrite.dwBytesTransferred, 0, &m_perIODataWrite.wsaOverLapped, NULL);
}


