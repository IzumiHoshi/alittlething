// TestDemo.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <string>
//#include "SocketTCPClient.h"

using namespace std;

#define CMD_REQ_LOGON		1
#define CMD_BACK_LOGON		2
#define CMD_REQ_AV			3
#define CMD_BACK_AUDIO		4
#define CMD_BACK_VIDEO		6
#define CMD_REQ_SEND_AV		7
#define CMD_BACK_SEND_AUDIO	8
#define CMD_REQ_GET_PARA	11
#define CMD_BACK_GET_PARA	12
#define CMD_REQ_SET_PARA	13
#define CMD_BACK_SET_PARA	14
#define CMD_REQ_PTZ			15
#define CMD_BACK_PTZ		16
#define CMD_REQ_UPGRADE		17
#define CMD_BACK_UPGRADE	18

#define CMD_REQ_VIDEO_SHIFT    33//21
#define CMD_ACK_VIDEO_SHIFT    34//22
#define CMD_REQ_PICNUM_SHIFT   25
#define CMD_ACK_PICNUM_SHIFT   26
#define CMD_REQ_NETPARAM       27
#define CMD_ACK_NETPARAM       28
#define CMD_REQ_DEVSTART_PARAM 29
#define CMD_ACK_DEVSTART_PARAM 30
#define CMD_REQ_GET_DEVPARAM   31
#define CMD_ACK_GET_DEVPARAM   32

#define CMD_REQ_SET_RTSP       43
#define CMD_ACK_SET_RTSP       44
#define CMD_REQ_GET_RTSP       45
#define CMD_ACK_GET_RTSP       46

#define CMD_REQ_SUBSCRIBLE_PICTURE		65
#define CMD_ACK_SUBSCRIBLE_PICTURE		66
#define CMD_REQ_UNSUBSCRIBLE_PICTURE	67
#define CMD_ACK_UNSUBSCRIBLE_PICTURE	68
#define CMD_PUSH_PICTURE_INFO			69
#define CMD_ACK_PUSH_PICTURE_INFO		70

#define RE_SUCCESS			0
#define RE_USERNAME_ERROR	1
#define RE_PASSWORD_ERROR	2
#define NAME_LEN			32
typedef struct tagLOGON_INFO
{
	unsigned int   m_UserID;
	unsigned short m_ErrorCode;
	unsigned short m_ExtLength;
}LOGON_INFO,*LPLOGON_INFO;

typedef struct tagLOGON_REQ_INFO
{
	unsigned char  m_UserName[NAME_LEN];
	unsigned char  m_Password[NAME_LEN];
	unsigned char  m_Res[64]; //wxf 添加新协议保留的64字节，参见《INFINOVA数字产品传输协议》2012-3-6修改的地方
	tagLOGON_REQ_INFO()
	{
		memset(m_UserName,0,NAME_LEN);
		memset(m_Password,0,NAME_LEN);
		memset(m_Res,0,64);
	}
}LOGON_REQ_INFO,*LPLOGON_REQ_INFO;

typedef struct tagCMD_HEADER
{
	unsigned char  m_StartCode[3];
	unsigned char  m_CmdType;
	unsigned short m_PacketSN;
	unsigned short m_PacketNum;
	unsigned int   m_DataLength;
}CMD_HEADER,*LPCMD_HEADER;

struct S_REQ_PUSH_PICTURE_INFO
{
	unsigned char  m_StartCode[3];
	unsigned char  m_CmdType;
	unsigned short m_PacketSN;
	unsigned short m_PacketNum;
	unsigned int   m_DataLength;
	unsigned int   nPicProLen;	//图片属性长度
	unsigned int   nPicLen;		//图片内容长度
	unsigned int   nExtLen;		//扩展长度
};

typedef struct tagPicturePushInfo
{
	int		nChannel;			/**< 产生图片的通道 */
	char	cName[80];			/**< 图片的名称 最长32个字符*/
	char	cFormat[16];		/**< 图片的格式 目前只有jpg*/
	long	lStartTimeSecond;	/**< 图片的起始时间 */
	long	lStartTimeMs;		/**< 图片的起始时间（毫秒） */
	long	lEndTimeSecond;		/**< 图片的结束时间 */
	long	lEndTimeMs;			/**< 图片的结束时间（毫秒） */
	char	cPlate[128];		/**< 关联信息:车牌 */
}S_PICTURE_PUSH_INFO;

bool Connect(SOCKET &hSocket, string strDevIP, unsigned short uPort)
{
	if(hSocket != INVALID_SOCKET) //防止多次connect
	{
		return false;
	}

	hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(hSocket == INVALID_SOCKET)
	{
		return false;
	}

	int nNetTimeout = 2000;
	int nReuse = 1;
	setsockopt(hSocket,SOL_SOCKET,SO_RCVTIMEO,(char *)&nNetTimeout,sizeof(int));
	setsockopt(hSocket,SOL_SOCKET,SO_SNDTIMEO,(char *)&nNetTimeout,sizeof(int));
	setsockopt(hSocket,SOL_SOCKET,SO_REUSEADDR,(char *)&nReuse,sizeof(int));

	//设置非阻塞方式连接
	unsigned long ul = 1;
	int ret = ioctlsocket(hSocket, FIONBIO, (unsigned long*)&ul);
	if(ret==SOCKET_ERROR)
		return false;

	sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(uPort); 
	local.sin_addr.S_un.S_addr = inet_addr(strDevIP.c_str());

	connect(hSocket, (sockaddr*)&local, sizeof (local)); 

	//select 模型，即设置超时
	struct timeval timeout ;
	fd_set r;

	FD_ZERO(&r);
	FD_SET(hSocket, &r);
	timeout.tv_sec = 2; //连接超时2秒
	timeout.tv_usec =0;
	ret = select(0, 0, &r, 0, &timeout);
	if ( ret <= 0 )
	{
		::closesocket(hSocket);
		hSocket = INVALID_SOCKET;
		return false;
	}
	//再设回阻塞模式
	unsigned long ul1= 0 ;
	ret = ioctlsocket(hSocket, FIONBIO, (unsigned long*)&ul1);
	if(ret==SOCKET_ERROR)
	{
		::closesocket (hSocket);
		hSocket = INVALID_SOCKET;
		return false;
	}

	return true;
}

bool LogOn(SOCKET &hSocket, string strUsername, string strPassword)
{
	int iRevLen;
	BYTE pBuf[1024] = {0};
	LPLOGON_INFO pLogonInfo;
	LPCMD_HEADER pCmprintfeader = (LPCMD_HEADER)pBuf;
	LPLOGON_REQ_INFO pLogonReqInfo = (LPLOGON_REQ_INFO)(pBuf + sizeof(CMD_HEADER));

	pCmprintfeader->m_StartCode[0] = 'I';
	pCmprintfeader->m_StartCode[1] = 'N';
	pCmprintfeader->m_StartCode[2] = 'F';
	pCmprintfeader->m_CmdType = CMD_REQ_LOGON;
	pCmprintfeader->m_PacketNum = htons(1);
	pCmprintfeader->m_PacketSN = htons(1);
	pCmprintfeader->m_DataLength = htonl(sizeof(LOGON_REQ_INFO));

	memcpy(pLogonReqInfo->m_UserName,strUsername.c_str(),NAME_LEN);
	memcpy(pLogonReqInfo->m_Password,strPassword.c_str(),NAME_LEN);
	if(send(hSocket,(char*)pBuf, sizeof(CMD_HEADER) + sizeof(LOGON_REQ_INFO),NULL)== SOCKET_ERROR)
	{
		return false;
	}

	memset(pBuf,0,sizeof(pBuf));
	iRevLen = sizeof(CMD_HEADER) + sizeof(LOGON_INFO);
	if(recv(hSocket,(char*)pBuf,iRevLen,NULL) == SOCKET_ERROR)
	{
		return false;	
	}	

	pCmprintfeader = (LPCMD_HEADER)pBuf;
	pLogonInfo = (LPLOGON_INFO)(pBuf+sizeof(CMD_HEADER));
	if (pCmprintfeader->m_CmdType == CMD_BACK_LOGON && pLogonInfo->m_ErrorCode == RE_SUCCESS)
	{
		return true;	
	}
	else if(pLogonInfo->m_ErrorCode == RE_USERNAME_ERROR)
	{
		return false;
	}
	else if(pLogonInfo->m_ErrorCode == RE_PASSWORD_ERROR)
	{
		return false;
	}
	return false;
}

string toXML(S_PICTURE_PUSH_INFO * pPushInfo)
{
	throw std::logic_error("The method or operation is not implemented.");
}

std::string SendMsg(SOCKET &hSocket, S_PICTURE_PUSH_INFO *pPushInfo, BYTE *pSendData, unsigned long nLenth)
{
	int iRevLen;
	BYTE pBuf[1024] = {0};
	LPCMD_HEADER pCmprintfeader = (LPCMD_HEADER)pBuf;
	string strXml = toXML(pPushInfo);
	BYTE *pData = pBuf + sizeof(CMD_HEADER);

	pCmprintfeader->m_StartCode[0] = 'I';
	pCmprintfeader->m_StartCode[1] = 'N';
	pCmprintfeader->m_StartCode[2] = 'F';
	pCmprintfeader->m_CmdType = CMD_PUSH_PICTURE_INFO;
	pCmprintfeader->m_PacketNum = htons(1);
	pCmprintfeader->m_PacketSN = htons(1);
	pCmprintfeader->m_DataLength = htonl(nLenth);

	memcpy_s(pData, strXml.length(), strXml.c_str(), strXml.length());

	if(send(hSocket,(char*)pBuf, sizeof(CMD_HEADER) + strXml.length(),NULL)== SOCKET_ERROR)
	{
		return "";
	}

	memset(pBuf,0,sizeof(pBuf));
	iRevLen = 1024;

	if(recv(hSocket,(char*)pBuf,iRevLen,NULL) == SOCKET_ERROR)
	{
		return "";	
	}

	//if (pCmprintfeader->m_CmdType == CMD_ACK_PUSH_PICTURE_INFO)
	//{
	//	return GetPictureSubscribleID((char *)pData, ntohl(pCmprintfeader->m_DataLength));
	//}

	printf("%s Failed! RecvCmd = %d", __FUNCTION__, pCmprintfeader->m_CmdType);
	return "";
}

#pragma comment(lib,"Ws2_32.lib")

int _tmain(int argc, _TCHAR *argv[])
{
	printf("hello word\n");
	//CSocketTCPClient *pSocket = new CSocketTCPClient();
	BYTE *pBuf = new BYTE[2*1024*1024];
	string strIP;
	unsigned short port;
	printf("Please input remoteIP: \n");
	cin>>strIP;
	printf("Please input remotePort: \n");
	cin>>port;

	//if (!pSocket->Open(strIP.c_str(), port, NULL, FALSE))
	//{
	//	printf("Open Socket error! -%d", WSAGetLastError());
	//	system("pause");
	//	return -1;
	//}
	system("pause");
	return 0;
}

