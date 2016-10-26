#include "stdafx.h"
#include "BaseThread.h"

V3083Msg::V3083Msg()
{
	m_bNeedAddAgain = FALSE;
	m_latetime = 0;
	m_loop = 0;
}

V3083Msg::~V3083Msg()
{

}
//重载消息赋值
V3083Msg V3083Msg::operator=(MSG msg)
{
	this->hwnd = msg.hwnd;
	this->lParam = msg.lParam;
	this->wParam = msg.wParam;
	this->message = msg.message;
	this->pt = msg.pt;
	this->time = msg.time;
	return *this;
}
//重载消息赋值
V3083Msg V3083Msg::operator=(V3083Msg msg)
{
	this->hwnd = msg.hwnd;
	this->lParam = msg.lParam;
	this->wParam = msg.wParam;
	this->message = msg.message;
	this->pt = msg.pt;
	this->time = msg.time;
	this->m_latetime = msg.m_latetime;
	this->m_bNeedAddAgain = msg.m_bNeedAddAgain;
	return *this;
}

CBaseThread::CBaseThread(void)
{
	m_Thread = NULL;//线程句柄
	m_dThreadID = 0;//线程ID
	m_dThreadIDSendMessage = 0;//消息推送线程ID
	m_eWorkMode = Minior_mode;
	InitializeCriticalSection(&m_cs);//锁
	InitInistance();
	InitClass();
}

CBaseThread::~CBaseThread()
{
	ExitInistance();
	if(m_Thread)
	{
		PostThreadMessage(m_dThreadID, WM_TASK_QUIT, 0, 0);
		WaitForSingleObject(m_Thread, INFINITE);
		CloseHandle(m_Thread);
	}
	m_Thread = NULL;
	DeleteCriticalSection(&m_cs);
}

BOOL CBaseThread::InitClass()
{
	m_Thread = (HANDLE)CreateThread(NULL, 0,LPTHREAD_START_ROUTINE(RealThreadRun), (void*)this, 0, &m_dThreadID);
	if (m_Thread == NULL)
	{
		return FALSE;
	}
	return TRUE;
}

UINT WINAPI CBaseThread::RealThreadRun(void* pParm)
{
	CBaseThread* pThis = (CBaseThread*)pParm;
	if(pThis)
	{
		//MSG msg;
		V3083Msg msg;
		while(TRUE)
		{
			//if(GetMessage(&msg, 0, 0, 0))
			if(pThis->V3083_GetMessage(&msg))
			{
				/*if(msg.message == WM_TASK_QUIT)
					return 0;//中止的消息*/
				int nDelayTime=0;
				BOOL bAddAgain = FALSE;
				if(msg.lParam != 0 && pThis->OnProcessMessage(&msg))
				{
					if(msg.lParam != 0 && !msg.m_bNeedAddAgain) //表示消息响应已经结束 不需要再执行了
					{
						switch(pThis->m_eWorkMode)
						{
						case Minior_mode:
							pThis->SendBackMessage(&msg);
							break;
						case Major_mode:
							pThis->PostTaskMessage(&msg);	
							break;
						}
					}
				}
				pThis->V3083_FreeMessage(&msg);
			}
			else
			{
				return 0;//消息中止
			}
		}

	}
	return 0;
}

BOOL CBaseThread::SendBackMessage(LPMSG pMsg)
{
	((NotifyBack*)(pMsg->lParam))->eType = MSG_Back;
	if(m_dThreadIDSendMessage == 0) 
	{
		return FALSE;
	}
	return PostThreadMessage(m_dThreadIDSendMessage, pMsg->message, pMsg->wParam, pMsg->lParam);
}

BOOL CBaseThread::PostTaskMessage(LPMSG pMsg)
{
	BOOL bRet = FALSE;
	NotifyBack* pBack =(NotifyBack*)(pMsg->lParam);
	switch(pBack->eType)
	{
	case MSG_Start:
			if( pBack->dThreadId != 0)
				bRet= PostThreadMessage(pBack->dThreadId, pMsg->message, pMsg->wParam, pMsg->lParam);
		break;
	case MSG_Back:
		if(GetMainWndHwnd())
			bRet = PostMessage(GetMainWndHwnd(), WM_TASK_MSG_BACK, pMsg->message, pMsg->lParam);
		break;
	}
	return bRet;
}


BOOL CBaseThread::OnProcessMessage(LPMSG pMsg)
{
	return FALSE;
}

BOOL CBaseThread::InitInistance()
{
	return TRUE;
}

BOOL CBaseThread::ExitInistance()
{
	return TRUE;
}

DWORD CBaseThread::GetThreadId()
{
	return m_dThreadID;
}

DWORD CBaseThread::GetParentThreadId()
{
	if(m_eWorkMode == Minior_mode)
		return m_dThreadIDSendMessage;
	return 0;
}

BOOL CBaseThread::SetMessageToThreadId(DWORD dThreadId)
{
	this->m_dThreadIDSendMessage = dThreadId;
	return TRUE;
}


BOOL CBaseThread::SetMode(eWorkMode eMode)
{
	this->m_eWorkMode = eMode;
	return TRUE;
}

HWND CBaseThread::GetMainWndHwnd()
{
	return NULL;
}

BOOL CBaseThread::V3083_GetMessage(V3083Msg* lpMsg)
{
	while(m_ThreadTaskList.empty())
	{
		MSG msg;
		if(GetMessage(&msg, 0, 0, 0))
		{
			if(msg.message == WM_TASK_QUIT)
				return FALSE;//中止的消息
			else if(msg.lParam != 0)
			{
				V3083Msg v3083msg;
				v3083msg = msg;
				m_ThreadTaskList.push_back(v3083msg);//放进自己的消息队列
				break;
			}
		}
	}
	*lpMsg = m_ThreadTaskList.front();
	return TRUE;
}

void  CBaseThread::V3083_FreeMessage(V3083Msg* lpMsg)
{
	if (!lpMsg->m_bNeedAddAgain)
	{
		m_ThreadTaskList.pop_front();
	}
	else
	{
		if(lpMsg->m_latetime<5) //两边限制
			lpMsg->m_latetime = 5;
		if(lpMsg->m_latetime>2000)
			lpMsg->m_latetime = 2000;
		DWORD nStartTime = GetTickCount();
		while(GetTickCount() - nStartTime <lpMsg->m_latetime)//时间延迟
		{
			Sleep(10);
		}
		m_ThreadTaskList.pop_front();//丢掉原来的头
		lpMsg->m_loop++;
		lpMsg->m_bNeedAddAgain = FALSE;
		lpMsg->m_latetime = 0;
		m_ThreadTaskList.push_front(*lpMsg);//换成新的头 内容一样 计数器加1
	}
}


CMemoryControl::CMemoryControl()
{
	m_nMax = 100;
	InitializeCriticalSection(&m_protectcs);
}

CMemoryControl::~CMemoryControl()
{
	AllClear();
	DeleteCriticalSection(&m_protectcs);
}

void CMemoryControl::SetMaxMemory(int nMax)
{
	if(nMax>100)//下限
		this->m_nMax = nMax;
	if(this->m_nMax >150)//上限
		this->m_nMax = 150;
}

NotifyBack* CMemoryControl::GetNotify()
{
	NotifyBack* pBack = NULL;
	::EnterCriticalSection(&m_protectcs);
	for (vector<LPNOTIFYBACKINFO>::iterator it = m_listNotify.begin(); it!=m_listNotify.end(); it++)
	{
		if(!(*it)->bUsed)
		{
			(*it)->bUsed = TRUE;
			pBack = (*it)->pData;
			break;
		}
	}
	if(!pBack)
	{
		pBack = GetNewAnimal();
		//AddNewElement(pBack, TRUE);
		//m_nMax++;
	}
	::LeaveCriticalSection(&m_protectcs);
	return pBack;
}
BOOL CMemoryControl::BackNotify(NotifyBack* pBack)
{
	::EnterCriticalSection(&m_protectcs);
	BOOL bFree = FALSE, bRet = FALSE;
/*	if(m_listNotify.size() >m_nMax)//超支了
	{
		bFree = TRUE;
	}*/
	for (vector<LPNOTIFYBACKINFO>::iterator it = m_listNotify.begin(); it!=m_listNotify.end(); it++)
	{
		if((*it)->pData == pBack)
		{
			/*if(bFree)
			{
				FreeNotify((*it)->pData);
				(*it)->pData = NULL;
				delete *it;
				m_listNotify.erase(it);//删除它
			}
			else*/
			{
				(*it)->bUsed = FALSE;
				ClearData((*it)->pData);
			}
			bRet = TRUE;
			break;
		}
	}
	if(!bRet)//如果找到了
		if(pBack)//如果不为空
			FreeNotify(pBack);//释放内存
	::LeaveCriticalSection(&m_protectcs);
	return bRet ;
}

NotifyBack* CMemoryControl::GetNewAnimal()
{
	return new NotifyBack;
}

void CMemoryControl::AddNewElement(NotifyBack* pStart, BOOL bUsed)
{
	if(!pStart)
		return ;
	NOTIFYBACKINFO* pElement = new NOTIFYBACKINFO;
	pElement->bUsed = bUsed;
	pElement->pData = pStart;
	m_listNotify.push_back(pElement);
}

BOOL CMemoryControl::StartWork()
{
	for (int i=0; i<m_nMax; i++)
	{
		AddNewElement(GetNewAnimal());
	}
	return TRUE;
}

void CMemoryControl::FreeNotify(void* pUser)
{
	delete (NotifyBack*)pUser;
	pUser = NULL;
}

void CMemoryControl::ClearData(NotifyBack* pData)
{
	pData->nErrCode = 0;
	pData->eType = MSG_Start;
	pData->dThreadId = 0;
}

void CMemoryControl::AllClear()
{
	::EnterCriticalSection(&m_protectcs);
	for (vector<LPNOTIFYBACKINFO>::iterator it = m_listNotify.begin() ;it!=m_listNotify.end(); it++)
	{
		if((*it)->pData)
			FreeNotify((*it)->pData);
		delete (*it);
	}
	m_listNotify.clear();
	::LeaveCriticalSection(&m_protectcs);
}
