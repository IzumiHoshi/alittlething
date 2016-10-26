#pragma once
/*****
add by chhb 2016.6.1
��ܵ����л��� ���ڴ����
****/
enum enumMessage{MSG_Start, MSG_Back};
class NotifyBack
{
public:
	NotifyBack(){nErrCode = 0;dThreadId=0;eType = MSG_Start;}
	virtual ~NotifyBack(){};
	int nErrCode;//�����봦��Ľ��
	DWORD dThreadId;//Ҫ���͵������߳�ID
	enumMessage eType;
};//�̳�

class V3083Msg:public MSG
{
public:
	V3083Msg();
	~V3083Msg();
	V3083Msg operator=(MSG msg);
	V3083Msg operator=(V3083Msg msg);
	DWORD m_latetime;//�ӳ�ʱ��
	BOOL m_bNeedAddAgain;//��Ϣ�����������
	int m_loop;//����
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
	//���Žӿ�
	DWORD GetThreadId();//��ȡ��ǰ�߳�ID
	DWORD GetParentThreadId();
	BOOL SetMessageToThreadId(DWORD dThreadId);//������Ϣ���ص��߳�ID
	//BOOL AddMessageToThreadId(DWORD dThreadId);//�����Ϣ���ص��߳�ID

	BOOL SetMode(eWorkMode eMode);//�����̵߳�ģʽ
	virtual BOOL OnProcessMessage(LPMSG pMsg);//����̳д���
    virtual BOOL InitInistance();//����̳г�ʼ��
	virtual BOOL ExitInistance();//����̳г����˳�
	virtual BOOL FreeTheNotify(void* pFree) = 0;
	virtual HWND GetMainWndHwnd();

	BOOL V3083_GetMessage(V3083Msg* lpMsg);
	void   V3083_FreeMessage(V3083Msg* lpMsg);
	//BOOL StartWork();
private:
	BOOL InitClass();//��ʼ��
	static UINT WINAPI RealThreadRun(void* pParm);
	
	BOOL SendBackMessage(LPMSG pMsg);
	BOOL PostTaskMessage(LPMSG pMsg);

private:
	HANDLE m_Thread;//�߳̾��
	DWORD  m_dThreadID;//�߳�ID
	eWorkMode m_eWorkMode;//ģʽ
	CRITICAL_SECTION  m_cs;//��
	DWORD m_dThreadIDSendMessage;//��Ϣ�����߳�ID
	list<V3083Msg> m_ThreadTaskList;//�����б�
};

typedef struct tagNotifyBackInfo
{
	BOOL bUsed;//�Ƿ���ʹ��
	NotifyBack* pData;//�����ڴ�
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

public: //�ӿ�
	NotifyBack* GetNotify();
	BOOL BackNotify(NotifyBack* pBack);
	void SetMaxMemory(int nMax);
	BOOL StartWork();
	virtual void AllClear();
	int GetMaxMemory(int nMax){return m_nMax;};
	
private:
	void AddNewElement(NotifyBack* pStart, BOOL bUsed = FALSE);

public://���̳�
	virtual NotifyBack* GetNewAnimal();
	virtual void FreeNotify(void* pUser);
	virtual void ClearData(NotifyBack* pData);
protected:
	vector< LPNOTIFYBACKINFO> m_listNotify;//�ڴ��б�
	CRITICAL_SECTION m_protectcs;

private:
	int m_nMax;//����ڴ���
};