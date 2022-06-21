#pragma once

#include "stdafx.h"
#include "TriggerEditor.h"
#include <include\EditorData.h>


class WorldEditor
{
public:

	WorldEditor();
	~WorldEditor();
	
	//static WorldEditor* getInstance();

	static uintptr_t getAddress(uintptr_t addr);

	EditorData* getEditorData();

	const char* getCurrentMapPath();

	const char* getTempSavePath();

	std::string getConfigData(const std::string& parentKey, const std::string& childKey, int index = 0);

	//��ȡ����������� ����id �ȼ� �ֶ� ����ֵ
	bool getSkillObjectData(uint32_t id, uint32_t level, std::string text, std::string& value);

	//��λ�Ƿ�������� �����Ըü���Ϊģ����չ�������Ӽ���
	bool hasSkillByUnit(uint32_t unit_id, uint32_t skill_id);

	int getSoundDuration(const char* path);

	void saveMap(const char* outPath);

	void onSaveMap(const char* tempPath, EditorData* data);

	void loadConfigData();
private:

	int saveWts();
	int saveW3i();
	int saveImp();
	int saveW3e();
	int saveShd();
	int saveWpm();
	int saveMiniMap();
	int saveMmp();
	int saveObject();
	int saveDoodas();
	int saveUnits();
	int saveRect();
	int saveCamara();
	int saveSound();
	int saveTrigger();
	int saveScript();
	int saveArchive();

	int customSaveWts(const char* path);
	int customSaveDoodas(const char* path);

	void updateSaveFlags();
protected: 

	


	uintptr_t m_editorObject;
	//fs::path m_tmp_path{};
	const char* m_tempPath;
	bool m_bInit;
public:
	EditorData* m_currentData;

};
extern WorldEditor g_c_world_editor;
WorldEditor& get_world_editor();