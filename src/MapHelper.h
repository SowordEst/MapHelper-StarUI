#pragma once
#include "stdafx.h"
#include <base\hook\inline.h>
#include <include\EditorData.h>
#include "WorldEditor.h"

template<typename dst_type, typename src_type>
dst_type union_cast(src_type src)
{
	union {
		src_type s;
		dst_type d;
	}u;
	u.s = src;
	return u.d;
}


struct ActionInfo
{
	int type_id;
	std::string name;
};


void SetActionToTextBufferSize(int size);


typedef std::vector<ActionInfo> ActionInfoList;
typedef std::map<std::string, ActionInfoList> ActionInfoMap;

extern ActionInfoMap g_actionInfoTable;

extern HMODULE g_hModule;

extern fs::path g_module_path;

class Helper
{
public:
	Helper();
	~Helper();
	
	//static Helper* getInstance();

	void enableConsole();

	void attach();//����

	void detach();//����

	int getConfig() const;

	void setMenuEnable(bool is_enable);
private:


	//�����ͼ
	static uintptr_t onSaveMap();

	//ѡ��ת��ģʽ
	int onSelectConvartMode();

	//���Զ���ת������ʱ
	int onConvertTrigger(Trigger* trg);

public:
	fs::path ydwe_path;

protected:
	bool m_bAttach;

	//�Զ���jass��������hook
	hook::hook_t* m_hookSaveMap;
	hook::hook_t* m_hookConvertTrigger;

	//��̬�������� �������͵�hook
	hook::hook_t* m_hookCreateUI;
	hook::hook_t* m_hookReturnTypeStrcmp;

	//�Զ��嶯�����hook
	hook::hook_t* m_hookGetChildCount;
	hook::hook_t* m_hookGetString;
	hook::hook_t* m_hookGetActionType;
	
	//�ǶȻ��Ȼ�����hook����
	hook::hook_t* m_hookParamTypeStrncmp1;
	hook::hook_t* m_hookParamTypeStrncmp2;
	hook::hook_t* m_hookGetParamType;

	//hook we�ĵ��� �����ʱ��������в˵�
	uintptr_t m_hookMessageBoxA;

	hook::hook_t* m_hookInsertActionToText;

	fs::path m_configPath;
};
extern Helper g_CHelper;
Helper& get_helper();
