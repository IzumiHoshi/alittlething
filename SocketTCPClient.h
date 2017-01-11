#pragma once
#include "Winsock2.h"

#define IO_TYPE_READ			1
#define IO_TYPE_WRITE			2
#define IO_TYPE_REQUEST_READ	3
#define IO_TYPE_CANCEL			4

namespace _COMMON
{

typedef struct PER_IO_DATA
{
	WSAOVERLAPPED wsaOverLapped;
	WSABUF wsaBuf;
	DWORD dwFlags;
	DWORD dwBytesTransferred;
	DWORD dwIOType;
}PER_IO_DATA, *LPPER_IO_DATA;

class CSocketTCPClient
{
public:
	CSocketTCPClient(void);
	~CSocketTCPClient(void);

public:
	HANDLE m_hEvent;
	SOCKET m_hSocket;
	SOCKADDR_IN m_saRemote;
	HWND m_hWndParent;
	BOOL Open(DWORD dwRemoteIP, u_short usRemotePort, HWND hWndParent, BOOL bEvent = TRUE);
	BOOL Open(LPCTSTR szIP, u_short usRemotePort, HWND hWndParent, BOOL bEvent = TRUE);
	BOOL Close(void);
	int Connect(void);
	int DisConnect(void);
	int Receive(char* pBuf, int iLength);
	int SendNoTry(char* pBuf,int iLength); 
	int Send(char* pBuf, int iLength);
	BOOL OpenEx(DWORD dwRemoteIP, u_short usRemotePort, HWND hWndParent);
	int ReceiveEx(char* pBuf, int iLength);
	int SendEx(char* pBuf, int iLength);
	PER_IO_DATA m_perIODataRead;
	PER_IO_DATA m_perIODataWrite;
	DWORD m_dwPendingIOCount;
};
}
using namespace _COMMON;
