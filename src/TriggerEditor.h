#pragma once

#include "stdafx.h"
#include "mapHelper.h"
#include <include\EditorData.h>

class TriggerEditor
{
public:
	TriggerEditor();
	~TriggerEditor();

	//static TriggerEditor* getInstance();

	void loadTriggers(TriggerData* data);
	void loadTriggerConfig(TriggerConfigData* data);

	void saveTriggers(const char* path); //����wtg
	void saveScriptTriggers(const char* path);//���� wct
	void saveSctipt(const char* path); //����j

	//����triggerתjass  
	std::string convertTrigger(Trigger* trigger);

	TriggerType* getTypeData(const std::string& type);
	std::string getBaseType(const std::string& type);
	std::string getScriptName(Action* action);

	

	//weԭ���tתj
	std::string originConvertTrigger(Trigger* trigger);

	//weԭ���tת�ı�
	std::string originConvertActionText(Action* action);


	//�������༭��ת����������Ϊ�Զ���ű���ʱ��
	bool onConvertTrigger(Trigger* trigger);

private:
	void writeCategoriy(BinaryWriter& writer);
	void writeVariable(BinaryWriter& writer);
	void writeTrigger(BinaryWriter& writer);
	void writeTrigger(BinaryWriter& writer,Trigger* trigger);
	void writeAction(BinaryWriter& writer, Action* action);
	void writeParameter(BinaryWriter& writer, Parameter* param);


	//�������Ƿ���к���������
	bool hasBlackAction(Trigger* trigger, bool* is_init, bool* is_disable);



	//������������������Ʒ ����� ����id
	std::string WriteRandomDisItem(const char* id); 

protected:
	TriggerConfigData* m_configData;
	TriggerData* m_editorData;

	uint32_t m_version;

	const std::string seperator = "//===========================================================================\n";

	//��������Ĭ�ϵ�ֵ
	std::unordered_map<std::string, TriggerType*> m_typesTable;

public:

	std::unordered_map<std::string, Variable*> variableTable;

	std::unordered_map<std::string, bool> m_initFuncTable;

	//ui������ �������к�������ui ��ʹ��ԭ��ת��
	std::unordered_map<std::string, bool> m_blacklist_map;

	std::unordered_map<Action*, Parameter*> m_param_action_parent_map;
	
	std::string action_to_text_key;
	bool is_convert = false;

	std::string spaces[200];
};
extern TriggerEditor g_trigger_editor;
TriggerEditor& get_trigger_editor();
//������Ʒ���͵�jass������
std::string randomItemTypes[];