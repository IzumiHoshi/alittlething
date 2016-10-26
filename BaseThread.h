#pragma once
/*****
add by chhb 2016.6.1
框架的运行机制 及内存管理
****/
enum enumMessage{MSG_Start, MSG_Back};
class NotifyBack
{
public:
	NotifyBack(){nErrCode = 0;dThreadId=0;eType = MSG_Start;}
	virtual ~NotifyBack(){};
	int nErrCode;//错误码处理的结果
	DWORD dThreadId;//要发送的数据线程ID
	enumMessage eType;
};//继承

class V3083Msg:public MSG
{
public:
	V3083Msg();
	~V3083Msg();
	V3083Msg operator=(MSG msg);
	V3083Msg operator=(V3083Msg msg);
	DWORD m_latetime;//延迟时间
	BOOL m_bNeedAddAgain;//消息重新塞会队列
	int m_loop;//计数
};
enum eWorkMode{Minior_mode, Major_mode};
#define DOING_NOTHING -1
#define WM_TASK_QUIT		(WM_USER + 1)
#define WM_TASK_MSG_BACK	(WM_USER + 2)
#include <list>
class CBaseThread
{
public:
	CBaseThread(void);
	CBaseThread(eWorkMode eMode);

	virtual ~CBaseThread(void);
public:
	//开放接口
	DWORD GetThreadId();//获取当前线程ID
	DWORD GetParentThreadId();
	BOOL SetMessageToThreadId(DWORD dThreadId);//设置消息返回的线程ID
	//BOOL AddMessageToThreadId(DWORD dThreadId);//添加消息返回的线程ID

	BOOL SetMode(eWorkMode eMode);//设置线程的模式
	virtual BOOL OnProcessMessage(LPMSG pMsg);//子类继承处理
    virtual BOOL InitInistance();//子类继承初始化
	virtual BOOL ExitInistance();//子类继承程序退出
	virtual BOOL FreeTheNotify(void* pFree) = 0;
	virtual HWND GetMainWndHwnd();

	BOOL V3083_GetMessage(V3083Msg* lpMsg);
	void   V3083_FreeMessage(V3083Msg* lpMsg);
	//BOOL StartWork();
private:
	BOOL InitClass();//初始化
	static UINT WINAPI RealThreadRun(void* pParm);
	
	BOOL SendBackMessage(LPMSG pMsg);
	BOOL PostTaskMessage(LPMSG pMsg);

private:
	HANDLE m_Thread;//线程句柄
	DWORD  m_dThreadID;//线程ID
	eWorkMode m_eWorkMode;//模式
	CRITICAL_SECTION  m_cs;//锁
	DWORD m_dThreadIDSendMessage;//消息推送线程ID
	list<V3083Msg> m_ThreadTaskList;//任务列表
};

typedef struct tagNotifyBackInfo
{
	BOOL bUsed;//是否在使用
	NotifyBack* pData;//数据内存
	tagNotifyBackInfo()
	{
		bUsed = FALSE;
		pData = NULL;
	}
}NOTIFYBACKINFO,*LPNOTIFYBACKINFO;

class CMemoryControl 
{
public:
	CMemoryControl();
	virtual ~CMemoryControl();

public: //接口
	NotifyBack* GetNotify();
	BOOL BackNotify(NotifyBack* pBack);
	void SetMaxMemory(int nMax);
	BOOL StartWork();
	virtual void AllClear();
	int GetMaxMemory(int nMax){return m_nMax;};
	
private:
	void AddNewElement(NotifyBack* pStart, BOOL bUsed = FALSE);

public://待继承
	virtual NotifyBack* GetNewAnimal();
	virtual void FreeNotify(void* pUser);
	virtual void ClearData(NotifyBack* pData);
protected:
	vector< LPNOTIFYBACKINFO> m_listNotify;//内存列表
	CRITICAL_SECTION m_protectcs;

private:
	int m_nMax;//最大内存数
};