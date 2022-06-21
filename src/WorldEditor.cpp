#include "stdafx.h"
#include "WorldEditor.h"
#include "TriggerEditor.h"

#include "mapHelper.h"
#include <include\Export.h>
#include <YDPluginManager.h>
#include <HashTable.hpp>
extern MakeEditorData* g_make_editor_data;

std::map<std::string, std::string> g_config_map;

WorldEditor::WorldEditor()
{
	m_currentData = nullptr;
	m_tempPath = nullptr;
}

WorldEditor::~WorldEditor()
= default;

//WorldEditor* WorldEditor::getInstance()
//{
//	static WorldEditor instance;
//
//	return &instance;
//}

uintptr_t WorldEditor::getAddress(uintptr_t addr)
{
	const auto base = reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr));
	return addr - 0x00400000 + base;
}


void WorldEditor::loadConfigData()
{
	auto& triggerEditor = get_trigger_editor();

	TriggerConfigData* configData = nullptr;

	if (g_make_editor_data)
	{
		configData = g_make_editor_data->config_data;
	}
	else
	{
		configData = std_call<TriggerConfigData*>(getAddress(0x004D4DA0));
	}

	triggerEditor.loadTriggerConfig(configData);
}

EditorData* WorldEditor::getEditorData()
{
	if (g_make_editor_data)
	{
		return g_make_editor_data->editor_data;
	}

	if (m_currentData) //�����ǰ������ ��ʹ�õ�ǰ������
	{
		return m_currentData;
	}

	uintptr_t addr = *(uintptr_t*)getAddress(0x803cb0);
	uintptr_t index = *(uintptr_t*)(addr + 0x1b0); //��ǰ��ͼ����

	uintptr_t object = *(uintptr_t*)(*(uintptr_t*)(addr + 0x1a8) + index * 4);

	if (*(uintptr_t*)(object + 0x114))
	{
		uintptr_t uknow = *(uintptr_t*)(object + 0x118);

		return *(EditorData**)uknow;
	}
	MessageBoxA(0, "��ȡ�����༭������", "���󣡣���", MB_OK);
	return 0;
}

void WorldEditor::saveMap(const char* outPath)
{
	this_call<int>(getAddress(0x0055ccd0), getEditorData(), outPath);
}



const char* WorldEditor::getCurrentMapPath()
{
	auto data = getEditorData();

	return data->mappath;
}

const char* WorldEditor::getTempSavePath()
{
	return m_tempPath;
}

int WorldEditor::getSoundDuration(const char* path)
{
	if (g_make_editor_data)
	{
		return g_make_editor_data->get_sound_duration(path);
	}
	uint32_t param[10];
	ZeroMemory(&param, sizeof param);
	auto& v_we = get_world_editor();
	fast_call<int>(v_we.getAddress(0x004DCFA0), path, &param);
	return param[1];
}


std::string WorldEditor::getConfigData(const std::string& parentKey, const std::string& childKey, int index)
{
	if (g_make_editor_data)
	{
		std::string key = parentKey + childKey + std::to_string(index);
		auto it = g_config_map.find(key);
		if (it == g_config_map.end())
		{
			std::string ret = g_make_editor_data->get_config_data(parentKey.c_str(), childKey.c_str(), index);

			g_config_map[key] = ret;
			return ret;
		}
		return it->second;
	}

	auto config = mh::get_config_table();

	const char* value = config->get_value(parentKey.c_str(), childKey.c_str(), index);
	
	if (!value)
	{
		return std::string();
	}

	auto real_value = config->get_value("WorldEditStrings", value, 0);
	if (real_value) {
		return std::string(real_value);
	}
	return std::string(value);
}


bool WorldEditor::getSkillObjectData(uint32_t id,uint32_t level,std::string text, std::string& value)
{
	if (g_make_editor_data)
	{
		value = g_make_editor_data->get_skill_data(id, level, text.c_str());
		return true;
	}
	uint32_t data = std_call<uint32_t>(getAddress(0x004D4EE0));
	char buffer[0x400];
	bool ret = this_call<bool>(getAddress(0x0050B7B0), data, id, text.c_str(), buffer, 0x400, level, 1);
	if (ret) value = buffer;
	return ret;
}


bool WorldEditor::hasSkillByUnit(uint32_t unit_id, uint32_t skill_id)
{
	if (g_make_editor_data) {
		return g_make_editor_data->has_skill_by_unit(unit_id, skill_id);
	}

	uintptr_t data = c_call<uintptr_t>(getAddress(0x004D4CA0));
	return this_call<bool>(getAddress(0x00501020), data, unit_id, skill_id, 0);
}

void WorldEditor::onSaveMap(const char* tempPath, EditorData* data)
{
	m_tempPath = tempPath;
	m_currentData = data;

	print("��ǰ��ͼ·��%s\n", getCurrentMapPath());
	print("�����ͼ·�� %s\n", getTempSavePath());


	auto& triggerEditor = get_trigger_editor();

	TriggerData* triggerData = getEditorData()->triggers;

	triggerEditor.loadTriggers(triggerData);


#if defined(EMBED_YDWE)
	int ret = 6;
#else
	int ret = 0;
	auto& v_helper = get_helper();
	const auto result = v_helper.getConfig();

	
	if (result == -1)
	{

		v_helper.setMenuEnable(false);

		ret = MessageBoxA(0, "�Ƿ����µı���ģʽ����?", "���еļ�����", MB_SYSTEMMODAL | MB_YESNO);

		v_helper.setMenuEnable(true);

		if (ret == 6)
			print("�Զ��屣��ģʽ\n");
		else
			print("ԭʼ����ģʽ\n");
	}
	else if (result == 1)
	{
		ret = 6;
	}
#endif

	clock_t start = clock();
		

	auto& manager = get_ydplugin_manager();

	manager.attach();


	if (ret == 6)
	{

		manager.m_enable = true;


		//customSaveWts(getTempSavePath());//��bug �Ȳ�����
		saveWts();

		saveW3i();
		saveImp();
		saveW3e();
		saveShd();
		saveWpm();
		saveMiniMap();
		saveMmp();
		saveObject();

		

		saveUnits();
		saveRect();
		saveCamara();
		saveSound();

		triggerEditor.saveTriggers(getTempSavePath());
		triggerEditor.saveScriptTriggers(getTempSavePath());
		triggerEditor.saveSctipt(getTempSavePath());

		customSaveDoodas(getTempSavePath());

		//���±�ǩ
		updateSaveFlags();
	}
	else
	{
		manager.m_enable = false;

		saveWts();

		saveW3i();
		saveImp();
		saveW3e();
		saveShd();
		saveWpm();
		saveMiniMap();
		saveMmp();
		saveObject();

		saveDoodas();

		saveUnits();
		saveRect();
		saveCamara();
		saveSound();

		saveTrigger();
		saveScript();
	}

	saveArchive();

		
	print("��ͼ�������ݱ������ �ܺ�ʱ : %f ��\n", (double)(clock() - start) / CLOCKS_PER_SEC);

	m_tempPath = nullptr;
	m_currentData = nullptr;
}

int WorldEditor::saveWts()
{

	print("����wts�ı�����\n");

	clock_t start = clock() ;

	int ret = this_call<int>(getAddress(0x0055DAF0), getEditorData(), getTempSavePath());

	print("wts������� ��ʱ : %f ��\n", (double)(clock() - start) / CLOCKS_PER_SEC);

	return ret;
}

int WorldEditor::saveW3i()
{
	print("����w3i��ͼ��Ϣ����\n");

	clock_t start = clock();

	int ret = this_call<int>(getAddress(0x0055D280), getEditorData(), getTempSavePath(), 1);

	print("w3i������� ��ʱ : %f ��\n", (double)(clock() - start) / CLOCKS_PER_SEC);

	return ret;
}
int WorldEditor::saveImp()
{
	print("����imp�ļ��б�����\n");

	clock_t start = clock();

	uintptr_t object = *(uintptr_t*)((uintptr_t)getEditorData() + 0x3904);
	this_call<int>(getAddress(0x0051CEB0), object, getTempSavePath());
	int ret = this_call<int>(getAddress(0x0055DFD0), getEditorData(), getTempSavePath());

	print("imp ������� ��ʱ : %f ��\n", (double)(clock() - start) / CLOCKS_PER_SEC);

	return ret;
}

int WorldEditor::saveW3e()
{
	print("����w3e������������\n");

	clock_t start = clock();

	int ret = this_call<int>(getAddress(0x005B0C50), getEditorData()->terrain, getTempSavePath());

	print("w3e ������� ��ʱ : %f ��\n", (double)(clock() - start) / CLOCKS_PER_SEC);

	return ret;

}

int WorldEditor::saveShd()
{
	print("����shd������Ӱ����\n");

	clock_t start = clock();

	int ret = this_call<int>(getAddress(0x0055d1f0), getEditorData(), getTempSavePath());

	print("shd ������� ��ʱ : %f ��\n", (double)(clock() - start) / CLOCKS_PER_SEC);

	return ret;
}


int WorldEditor::saveWpm()
{
	print("����wpm����·������\n");

	clock_t start = clock();


	std::string path = std::string(getTempSavePath()) + ".wpm";
	int ret = this_call<int>(getAddress(0x005E91C0), getEditorData()->terrain, path.c_str());

	print("wpm ������� ��ʱ : %f ��\n", (double)(clock() - start) / CLOCKS_PER_SEC);

	return ret;
}

int WorldEditor::saveMiniMap()
{
	print("����minimapС��ͼ����\n");

	clock_t start = clock();

	std::string path = std::string(getTempSavePath()) + "Map.tga";

	int ret = this_call<int>(getAddress(0x00583200), getEditorData(), path.c_str(), 0);

	print("minimap ������� ��ʱ : %f ��\n", (double)(clock() - start) / CLOCKS_PER_SEC);

	return ret;
}


int WorldEditor::saveMmp()
{
	print("����mmpԤ��С�ļ�������\n");

	clock_t start = clock();
	int ret = this_call<int>(getAddress(0x00583D00), getEditorData(), getTempSavePath());

	print("mmp ������� ��ʱ : %f ��\n", (double)(clock() - start) / CLOCKS_PER_SEC);

	return ret;
}

int WorldEditor::saveObject()
{
	print("�����������\n");

	clock_t start = clock();

	std::string path = std::string(getTempSavePath()) + "Map.tga";


	uintptr_t object = *(uintptr_t*)((uintptr_t)getEditorData() + 0x3908);
	this_call<int>(getAddress(0x00518CA0), object, getTempSavePath());

	object = *(uintptr_t*)((uintptr_t)getEditorData() + 0x390c);
	this_call<int>(getAddress(0x00518CA0), object, getTempSavePath());

	int ret = this_call<int>(getAddress(0x0051B5B0), getEditorData()->objects, getTempSavePath(), 1);

	print("��� ������� ��ʱ : %f ��\n", (double)(clock() - start) / CLOCKS_PER_SEC);

	return ret;;
}

int WorldEditor::saveDoodas()
{
	print("����war3map.doo����װ����\n");

	clock_t start = clock();

	int ret = this_call<int>(getAddress(0x0062BAE0), getEditorData()->doodas, getTempSavePath(), 1);

	print("war3map.doo ������� ��ʱ : %f ��\n", (double)(clock() - start) / CLOCKS_PER_SEC);

	return ret;
}


int WorldEditor::saveUnits()
{
	print("����war3mapUnit.doo���ε�λԤ������\n");

	clock_t start = clock();

	int ret = this_call<int>(getAddress(0x0062BAE0), getEditorData()->units, getTempSavePath(), 1);

	print("war3mapUnit.doo ������� ��ʱ : %f ��\n", (double)(clock() - start) / CLOCKS_PER_SEC);

	return ret;
}

int WorldEditor::saveRect()
{
	print("����w3r������������\n");

	clock_t start = clock();

	int ret = this_call<int>(getAddress(0x0062ACF0), getEditorData()->regions, getTempSavePath());

	print("war3map.doo ������� ��ʱ : %f ��\n", (double)(clock() - start) / CLOCKS_PER_SEC);

	return ret;
}


int WorldEditor::saveCamara()
{
	print("����w3cԤ�辵ͷ����\n");

	clock_t start = clock();

	int ret = this_call<int>(getAddress(0x005AEBB0), getEditorData()->cameras, getTempSavePath());

	print("w3c ������� ��ʱ : %f ��\n", (double)(clock() - start) / CLOCKS_PER_SEC);

	return ret;
}
int WorldEditor::saveSound()
{
	print("����w3s��������\n");

	clock_t start = clock();

	int ret = this_call<int>(getAddress(0x005EACE0), getEditorData()->sounds, getTempSavePath());

	print("w3s ������� ��ʱ : %f ��\n", (double)(clock() - start) / CLOCKS_PER_SEC);

	return ret;
}

int WorldEditor::saveTrigger()
{
	print("����wtg + wct ��������\n");

	clock_t start = clock();

	int ret = this_call<int>(getAddress(0x00520ED0), getEditorData()->triggers, getTempSavePath(), 1);

	print("wtg + wct  ������� ��ʱ : %f ��\n", (double)(clock() - start) / CLOCKS_PER_SEC);

	return ret;
}

int WorldEditor::saveScript()
{
	print("����war3map.j�ű��ļ�\n");

	clock_t start = clock();

	int ret = this_call<int>(getAddress(0x0055DA80), getEditorData(), getTempSavePath());

	print("war3map.j ������� ��ʱ : %f ��\n", (double)(clock() - start) / CLOCKS_PER_SEC);

	return ret;
}



int WorldEditor::saveArchive()
{


	fs::path path = fs::path(getTempSavePath());
	path.remove_filename();

	if (path.string().substr(path.string().size() - 1) == "\\")
		path = fs::path(path.string().substr(0, path.string().size() - 1));

	std::string name = path.filename().string();
	//if (name.length() < 4)
	//	return 0;

	name = name.substr(0,name.length() - 4);

	fs::path pathTemp = path / name;

	print("������ļ��д����mpq�ṹ\n");

	print("·�� %s\n", path.string().c_str());

	clock_t start = clock();


	int ret = this_call<int>(getAddress(0x0055D720), getEditorData(), pathTemp.string().c_str(), 1);

	if (ret)
	{
		path.remove_filename();

		fs::path path2 = path / name;

		//�ƶ��ļ�Ŀ¼
		ret = fast_call<int>(getAddress(0x004D0F60), pathTemp.string().c_str(), path2.string().c_str(), 1, 0);

		print("��ͼ������ ��ʱ : %f ��\n", (double)(clock() - start) / CLOCKS_PER_SEC);

		return ret;

	}

	
	return 0;
}



int WorldEditor::customSaveWts(const char* path)
{
	print("�Զ��屣��war3map.wts�ı�����\n");

	clock_t start = clock();

	BinaryWriter writer;
	auto string_data = getEditorData()->strings;


	writer.write<uint8_t>(0xEF);
	writer.write<uint8_t>(0xBB);
	writer.write<uint8_t>(0xBF);

	std::vector<TriggerString*> list(string_data->count);
	for (size_t i = 0; i < string_data->count; i++)
	{
		TriggerString* info = &string_data->array[i];
		list[i] = info;
	}
	std::sort(list.begin(), list.end(), [&](TriggerString* a, TriggerString* b) 
	{
		return a->index < b->index;
	});

	for (auto& i : list)
	{
		writer.write_string("STRING " + std::to_string(i->index));
		writer.write_string("\r\n{\r\n");
		writer.write_string(std::string(i->str));
		writer.write_string("\r\n}\r\n\r\n");
	}
	std::ofstream out(std::string(path) + ".wts", std::ios::binary);
	writer.finish(out);
	out.close();
	print("war3map.wts ������� ��ʱ : %f ��\n", (double)(clock() - start) / CLOCKS_PER_SEC);

	return 1;
}


int WorldEditor::customSaveDoodas(const char* path)
{
	print("�Զ��屣��war3map.doo����װ����\n");

	clock_t start = clock();
	auto doodas = getEditorData()->doodas; 

	auto& editor = get_trigger_editor();

	auto& variableTable = editor.variableTable;

	BinaryWriter writer; 


	writer.write_string("W3do"); 


	uint32_t write_version = 0x8; 

	writer.write<uint32_t>(write_version); 


	uint32_t write_subversion = 0xB; 

	writer.write<uint32_t>(write_subversion); 


	writer.write<uint32_t>(doodas->unit_count); 


	uint32_t object = *(uint32_t*)(((uint32_t)doodas) + 0xE0);
	uint32_t addr = *(uint32_t*)((*(uint32_t*)(doodas)) + 0xc8);

	char buffer[0x100];

	for (size_t i = 0;i<doodas->unit_count;i++)
	{
		Unit* unit = &doodas->array[i]; 

		writer.write_string(std::string(unit->name,0x4)); 
		writer.write<uint32_t>(unit->variation); 
		writer.write<float>(unit->x); 
		writer.write<float>(unit->y); 
		writer.write<float>(unit->z); 
		writer.write<float>(unit->angle);  
		writer.write<float>(unit->scale_x); 
		writer.write<float>(unit->scale_y); 
		writer.write<float>(unit->scale_z); 

		sprintf(buffer, "gg_dest_%.04s_%04d", unit->name, unit->index);

	
		//����װ����״̬
		uint8_t flag = 0;

		
		//ע�͵�����Щ��Ч��̫�� û̫���Ҫ���ж� 
		//�ж��Ƿ��ڿ��õ�ͼ��  �ڱ߽�Ϊtrue

		//if (!this_call<int>(getAddress(0x005E73A0), object, &unit->x))
		//	flag = 1;
		
		//�ж��Ƿ�ȫ�ֱ������� 
		//if (!this_call<int>(addr, doodas, i)) 

		//����Ӧ���ж���û�����ù�������Ʒ
		//if (variableTable.find(buffer) == variableTable.end())
		if (unit->item_setting_count <= 0 && unit->item_table_index == -1 && variableTable.find(buffer) == variableTable.end())
		{
			flag |= 0x2;
		}
		else
		{
			flag &= 0xfd;
		}

		//�Ƿ�����и߶� �ڵ��α༭������ ctrl + pageup or pagedown ���ù��߶ȵ�װ����
		if (*(uint8_t*)((uint32_t)unit + 0x84) & 0x8)
		{
			flag |= 0x4;
		}
		writer.write<uint8_t>(flag);
		writer.write<uint8_t>(unit->doodas_life); 
		writer.write<int32_t>(unit->item_table_index); 
		writer.write<uint32_t>(unit->item_setting_count2); 

		for (size_t a = 0; a < unit->item_setting_count2;a++)
		{
			ItemTableSetting* setting = unit->item_setting; 
			writer.write<uint32_t>(setting->info_count2); 

			for (size_t b = 0; b < setting->info_count2; b++)
			{
				ItemTableInfo* info = &setting->item_infos[b]; 
				// ����Ʒ��ӵ�����
				// ��������λ�û��ø�
				// �����ֱ����൥λ�����ƻ����Ǳ����õĵ���
				if (*(uint32_t*)(info->name) > 0) {
					writer.write_string(std::string(info->name, info->name + 0x4));
				}
				else
					writer.write_string("\0\0\0\0");
				writer.write<uint32_t>(info->rate); 
			}
		}

		writer.write<uint32_t>(unit->index); 
	}
	uint32_t write_special_version = 0;
	writer.write<uint32_t>(write_special_version);
	writer.write<uint32_t>(doodas->special_table->special_doodas_count); 

	for (size_t i = 0; i< doodas->special_table->special_doodas_count;i++)
	{
		SpecialDoodas* unit = &doodas->special_table->special[i];
		std::string id = std::string(unit->name, 0x4);
		// �����װ����
		writer.write_string(id);
		writer.write<uint32_t>(unit->variation); 
		writer.write<uint32_t>(unit->x); 
		writer.write<uint32_t>(unit->y); 
	}


	std::ofstream out(std::string(path) + ".doo", std::ios::binary);
	writer.finish(out);
	out.close();

	print("war3map.doo ������� ��ʱ : %f ��\n", (double)(clock() - start) / CLOCKS_PER_SEC);

	return 1;
}


void WorldEditor::updateSaveFlags()
{
	auto world_data = getEditorData();
	if (world_data->is_test)
		return;
	auto trigger_data = world_data->triggers;
	
	trigger_data->updage_flag = 0;
	trigger_data->variables->updage_flag = 0;
	for (size_t i = 0; i < trigger_data->categoriy_count; i++)
	{
		Categoriy* categoriy = trigger_data->categories[i];
		uint32_t trigger_count = categoriy->trigger_count;
		for (uint32_t n = 0; n < trigger_count; n++)
		{
			Trigger* trigger = categoriy->triggers[n];
			trigger->updage_flag = 0;
		}
	}

	world_data->doodas->updage_flag = 0;
}

WorldEditor& get_world_editor()
{
	static WorldEditor instance;
	return instance;
}
