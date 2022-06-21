#pragma once

#include <inttypes.h>

struct TriggerType
{
	uint32_t flag;//0x0
	const char type[0x8c];//0x4
	const char value[0xc8];//0x90 ������TriggerData�ļ����TriggerTypeDefaults��Ĭ��ֵ
	const char base_type[0x30];//0x158
	uint32_t is_import_path;//0x188
	char unknow[0x14];//0x18c
};//size 0x1a0

struct TriggerConfigData
{
	char unknow[0x1c];
	uint32_t type_count; //0x1c
	TriggerType* array;//0x20
};

struct Parameter
{
	enum Type:int {
		invalid = -1,
		preset,
		variable,
		function,
		string
	};
	uint32_t table; //0x0
	uint32_t unknow2; //0x4
	uint32_t typeId; //0x8 ���������� -1 ~3
	char type_name[0x40];//0xc �������� �ַ���
	const char value[0x12c]; //0x4c
	struct Action* funcParam;//0x178 ��0ʱ��ʾ �˲�����������
	Parameter* arrayParam;//0x17c ��0ʱ ��ʾ�ò������������ ����ӵ���Ӳ���
};

struct Action
{
	enum Type:int {
		event,
		condition,
		action,
		none
	};

	struct VritualTable
	{
		uint32_t (__thiscall* destroy)(void* pThis,bool freeMemory);
		uint32_t unknow2;
		uint32_t(__thiscall* getType)(void* pThis);
	};
	VritualTable* table; //0x0
	int fake_child_group_count;		//0x4 ����һ�����ƫ����
	int unknow1;					//0x8
	uint32_t child_count;	//0xc
	Action** child_actions;//0x10
	char unknow2[0xc];	 //0x14
	const char name[0x100]; //0x20
	uint32_t unknow3;//0x120;
	uint32_t unknow32;//0x124;
	uint32_t param_count; // 0x128
	Parameter** parameters;//0x12c
	uint32_t unknow4; //0x130
	uint32_t unk_object;//0x134
	uint32_t unk_num;//0x138
	uint32_t enable;//0x13c 
	char unknow5[0x14];//0x140
	uint32_t group_id;//0x154 �������������Ӷ���ʱΪ0 ������-1
};

struct Trigger
{
	char unknow1[0x8];
	uint32_t number;//0x8
	uint32_t line_count; //0xc
	Action** actions;	//0x10
	char unknow2[0x4];//0x14
	uint32_t is_comment; //0x18
	uint32_t unknow3; //0x1c
	uint32_t is_enable;  //0x20 
	uint32_t is_disable_init; //0x24
	uint32_t is_custom_srcipt;//0x28
	uint32_t is_initialize;//0x2c Ӧ��Ĭ�϶���1
	uint32_t unknow7;//0x30
	uint32_t custom_jass_size;//0x34
	const char* custom_jass_script;//0x38
	char unknow4[0x10]; //0x3c
	const char name[0x100];//0x4c
	uint32_t unknow5;//0x14c
	struct Categoriy* parent;//0x150 //�ô������ڵ��ļ���
	const char text[0x808];//0x154 ˵���ı�
	uint32_t updage_flag;//0x95c ���±�ǩ Ϊ1ʱ��ʾ���¸Ķ�

};

struct Categoriy
{
	uint32_t categoriy_id;
	const char categoriy_name[0x10C];
	uint32_t has_change; // 0x110
	uint32_t unknow2; // 0x114
	uint32_t is_comment; // 0x118
	char unknow[0x14];// 0x11c
	uint32_t trigger_count;//0x130 ��ǰ�����еĴ���������
	Trigger** triggers;		//0x134
};

struct Variable
{
	uint32_t unknow1;	//0x0 δ֪ ����1
	uint32_t is_array;	//0x4
	uint32_t array_size;//0x8
	uint32_t is_init;	//0xc
	const char type[0x1E];//0x10
	const char name[0x64];//0x2e
	const char value[0x12e];//0x92
};

struct VariableData
{
	char unknow1[0x8];		//0x0
	uint32_t globals_count;//0x8 ����gg_ ������ ����Ԥ�����ݵ�ȫ�ֱ���
	Variable* array; //0xc
	uint32_t unknow2;//0x10
	uint32_t updage_flag;//0x14 ���±�ǩ Ϊ1ʱ��ʾ���¸Ķ�
};

struct TriggerData
{
	uint32_t unknow1;		//0x0
	uint32_t trigger_count; // 0x4	���д���������
	char unknow2[0xC];		// 0x8
	uint32_t categoriy_count; //0x14 
	Categoriy** categories;	  //0x18
	uint32_t unknow3;		 //0x1c
	VariableData* variables;    //0x20
	char unknow4[0x10]; // 0x24
	const char global_jass_comment[0x800];//0x34
	uint32_t unknow5; //0x834
	uint32_t globals_jass_size; //0x838
	const char* globals_jass_script;//0x83c
	char unknow6[0xc];//0x840 
	uint32_t updage_flag;//0x84c ���±�ǩ Ϊ1ʱ��ʾ���¸Ķ�
};



struct RandomGroup
{
	uint32_t rate;//0x0 �ٷֱ�
	const char names[10][4]; //һ���������10������
};

struct RandomGroupData
{
	uint32_t unknow1;//0x0
	const char name[0x64];//0x4
	uint32_t param_count;//0x68//��ʾ ����������ֵ�����
	char unknow3[0x2c];//0x6c
	uint32_t group_count;//0x98
	RandomGroup* groups;//0x9c
	uint32_t unknow2;//0xA0
};//size 0xA4

struct ItemTableInfo
{
	const char name[0x4];//��Ʒid
	uint32_t rate;//0x4 ����
};
struct ItemTableSetting
{
	uint32_t info_count;//0x0
	uint32_t info_count2;//0x4
	ItemTableInfo* item_infos;//0x8
	uint32_t unknow;//0xc
};//0x10

struct ItemTable
{
	uint32_t unknow1;//0x0
	const char name[0x64];//0x4;
	uint32_t setting_count;//0x68
	uint32_t setting_count2;//0x6c
	ItemTableSetting* item_setting;//0x70
	uint32_t unknow2;//0x74
};//size 0x78

struct UnitItem
{
	uint32_t slot_id;//0x0
	const char name[0x4];//0x4 ��Ʒid
};//size 0x8

struct UnitSkill
{
	const char name[0x4];//0x0 ����id
	uint32_t is_enable;//0x4 ���� �Ƿ��ʼ������ü���
	uint32_t level;//0x8 ��ʼ���ȼ�
};//size 0xc

struct Unit
{
	uint32_t unknow1;//0x0
	const char name[0x4];//0x4 ��λ���id
	uint32_t variation;//0x8 ��ʽ
	float x;//0xc	//�ڵ����е�����
	float y;//0x10
	float z;//0x14
	float angle;//0x18 ������ Ҫת�ؽǶ� * 180 / pi = �Ƕ���
	float scale_x;//0x1c 
	float scale_y;//0x20
	float scale_z;//0x24
	float sacle;//0x28
	char unknow2[0x54];//0x2c
	uint32_t color;//0x80 ��ɫ �����䷶Χ-1ʱ��0xFFFFFFFF ��-2ʱ��0xFF1010FF
	char unknow24[0x3]; //0x84
	uint8_t type;//0x87 ���� ��Ʒ��1
	char unknow22[0x34];//0x88
	uint32_t player_id;//0xbc
	uint32_t unknow13;//0xc0
	uint32_t health;//0xc4	�����ٷֱ� ��С1 ���ڻ����100��Ϊ��1
	uint32_t mana;//0xc8	ħ���ٷֱ� ��С1 ���ڻ����100��Ϊ��1
	uint32_t level;//0xcc	�ȼ�
	uint32_t state_str;//0xd0 ���� ����
	uint32_t state_agi;//0xd4 ���� ����
	uint32_t state_int;//0xd8 ���� ����
	uint32_t item_table_index;//0xdc ��Ʒ�������
	uint32_t item_setting_count;//0xe0
	uint32_t item_setting_count2;//0xe4
	ItemTableSetting* item_setting;//0xe8
	char unknow4[0x4];//0xec
	int gold_count; //0xf0 ������� ��Ҫ�жϽ���ܲ�����Ч
	float warning_range;//0xf4 ���䷶Χ -1 ����ͨ -2 ��Ӫ��
	uint32_t item_count;//0xf8
	uint32_t item_count2;//0xfc
	UnitItem* items;//0x100
	uint32_t unknow14;//0x104
	uint32_t skill_count;//0x108
	uint32_t skill_count2;//0x10c
	UnitSkill* skills;//0x110
	char unknow3[0x4];//0x114

	//0x118 �����Ʒģʽ 0 Ϊ�κ���Ʒ ָ�� �ȼ�������  1 ��������� 2 �������Զ����б�
	uint32_t random_item_mode;//0x118

	uint8_t random_item_level;//0x11c
	char unknow23[0x2];//0x11d
	uint8_t random_item_type;//0x11f

	uint32_t random_group_index;//0x120 ������id �������ƷģʽΪ1ʱȡ��
	uint32_t random_group_child_index; // 0x124����� ����λ�� �����������Ϊ-1

	uint32_t random_item_count;//0x128
	uint32_t random_item_count2;//0x12c
	ItemTableInfo* random_items;//0x130
	uint32_t unknow134; //0x134
	uint32_t passive_color_index; //0x138  ��������������ɫ
	uint32_t pass_door_rect_index; //0x13c ����������
	char unknow25[0xc];//0x140
	uint32_t doodas_life;// 0x14c ���ƻ��������
	char unknow21[0x8];//0x150
	uint32_t index;//0x158 ȫ��Ԥ�������id
	char unknow28[0x2c];//0x15c
};//size 0x188


struct SpecialDoodas //����װ����  ���� �ƻ��ĵ��� ɽ������֮���
{
	const char name[0x4];//0x0 id 
	uint32_t variation;//0x4 
	uint32_t x;//0x8
	uint32_t y;//0xc
	char unknow[0x18];//0x10
};//size 0x28

struct  SpecialDoodasTable
{
	char unknow2[0x140]; //0x0
	uint32_t special_doodas_count;//0x140
	SpecialDoodas* special;//0x144
};
struct UnitData
{
	char unknow[0x5c];//0x0
	uint32_t unit_count;//0x5c
	Unit* array;//0x60
	char unknow2[0x7c]; //0x64
	SpecialDoodasTable* special_table;//0xe0
	char unknow3[0x8];//0xe4
	uint32_t updage_flag;//0xec ���±�ǩ Ϊ1ʱ��ʾ���¸Ķ�
};


struct Sound
{
	const char name[0x64];//0x0 ��gg_snd_ ǰ׺��ȫ�ֱ�����
	const char file[0x104];//0x64
	const char effect[0x34];//0x168
	uint32_t flag;//0x19c  1 | 2 | 4   1���Ƿ�ѭ�� 2 �Ƿ�3D��Ч 4 ������Χֹͣ
	uint32_t fade_out_rate;//0x1a0 ������
	uint32_t fade_in_rate;//0x1a4 ������
	uint32_t volume;//0x1a8 ����
	float pitch;//0x1ac ���� = ����
	char unknow1[0x8];//0x1b0
	uint32_t channel;//0x1b8 ͨ��
	float min_range;//0x1bc ��С˥����Χ
	float max_range;//0x1c0 ���˥����Χ
	float distance_cutoff;//0x1c4 �ضϾ���
	float inside;//0x1c8
	float outside;//0x1cc
	uint32_t outsideVolume;//0x1d0
	float x;//0x1d4
	float y;//0x1d8
	float z;//0x1dc
};//size 0x1E0

struct SoundData
{
	uint32_t unknow1;//0x0
	uint32_t sound_count;//0x4
	Sound* array;//0x8
};


struct MapInfo
{
	char unknow1[0xc4];
	float minY;//0xc4 ��ͼ��Сy
	float minX;//0xc8 ��ͼ��Сx
	float maxY;//0xcc ��ͼ���Y
	float maxX;//0xd0 ��ͼ���X
};

struct Region
{
	uint32_t unknow1;//0x0 
	const char name[0x80];//0x4;
	int bottom;//0x84 �� ��Ҫ��ͼ  * 32 - ��ͼ��С����
	int left;//0x88 ��
	int top;//0x8c��
	int right;//0x90��
	MapInfo* info;//0x94
	char unknow3[0x44];//98
	char weather_id[0x4];//0xdc
	char unknow[0x8];//0xe0
	char sound_name[0x64];//0xe8
	uint32_t color;//0x14c
};

struct RegionData
{
	char unknow[0x60];//0x0
	uint32_t region_count;//0x60
	Region** array; //0x64
	char unknow2[0x4];//0x68
	uint32_t updage_flag;//0x6C ���±�ǩ Ϊ1ʱ��ʾ���¸Ķ�
};


struct Camera
{
	uint32_t unknow1;//0x0
	float x;//0x4 
	float y;//0x8
	float z_offset;//0xc �߶�ƫ��
	float rotation;//0x10  z����ת
	float angle_of_attack;//0x14 x����תˮƽ
	float target_distance;//0x18 ��ͷ����
	float roll;//0x1c ����(����)
	float of_view;//0x20 �۲�����
	float farz;//0x24 Զ���ض�
	float unknow2;//0x28
	const char name[0x50];//0x2c
};//size 0x7c

struct CameraData
{
	char unknow[0x144]; //0x0
	uint32_t camera_count; //0x144
	Camera* array;//0x148
};

struct PlayerData
{
	uint32_t id;//0x0
	uint32_t controller_id;//0x4 ������id  0�� 1��� 2���� 3���� 4��Ӫ�ȵ�
	uint32_t race;//0x8 ���� 
	uint32_t is_lock;//0xc �Ƿ�������ʼ��
	const char name[0x20];//0x10
	char unknow2[0x4c];//0x30
	uint32_t low_level;//0x7c �����ȼ�  & ���id��2���� �жϸ�����Ƿ��ǵ����ȼ�
	uint32_t height_level;//0x80 �����ȼ�  & ���id��2���� �жϸ�����Ƿ��Ǹ����ȼ�
}; //size 80

struct TeamData
{
	uint32_t force_flags;//0x0 �����ǩ ��¼����ͬ������
	uint32_t player_masks;//0x4 ��Ҽ�¼�� �������������Щ���
	const char name[0x64];//0x8
};//0x6c

struct TriggerString
{
	uint32_t index;//0x0
	uint32_t unknow1;//0x4
	uint32_t unknow2;//0x8
	uint32_t unknow3;//0xc
	const char* str;//0x10
	char unknow[0x130];//0x14
};//size 0x144

struct TriggerStringData
{
	uint32_t unknow;//0x0
	uint32_t count;//0x4
	TriggerString* array;//0x8
};

struct TerrainData
{
	char unknow[0xc4];
	float map_rect_minx;
	float map_rect_miny;
	float map_rect_maxx;
	float map_rect_maxy;
};

struct EditorData
{
	uint32_t map_version;//0x0 ��ͼ�汾
	uint32_t map_save_count;//0x4 ��ͼ�������
	const char map_name[0x6D];// 0x8  ��ͼ����
	const char suggested_players[0x31];//0x75 ���������
	const char author[0x61];//0xa6 ������
	const char description[0x301];//0x107 ��ͼ˵��
	char unknow13[0x18];//0x408

	//��ͷ���������
	float camera_left_bottom_x;//0x420
	float camera_left_bottom_y;//0x424
	float camera_right_top_x;//0x428
	float camera_right_top_y;//0x42c
	float camera_left_top_x;//0x430
	float camera_left_top_y;//0x434
	float camera_right_bottom_x;//0x438
	float camera_right_bottom_y;//0x43c

	uint32_t mapset_flag;//0x440

	uint8_t tileset;//0x444 
	char unknow1[0x33fb];// 0x445
	uint32_t fog_type; //0x3840 ������ʽ
	float fog_z_start;//0x3844 
	float fog_z_end; //0x3848 
	float fog_density; //0x384c �ܶ�
	uint8_t fog_color[4]; //0x3850 ��ɫ
	uint32_t climate_id; //3854 ȫ������id
	char custorm_sound[0x20]; //0x3858 ������Ч
	uint8_t light;//0x3878 �Զ������ ���û����Ϊ0
	uint8_t water_color[4]; //0x3879 ˮ��ɫ
	char unknow15[0x7];//0x387d
	uint32_t player_count;//0x3884
	PlayerData* players;//0x3888
	char unknow11[0x8];//0x388c
	uint32_t steam_count;//0x3894
	TeamData* teams;//0x3898
	char unknow12[0x28];//0x389c
	uint32_t random_group_count;//0x38c4���������
	RandomGroupData* random_groups;//0x38c8//�����
	char unknow2[0x8];//0x38cc
	uint32_t item_table_count;//0x38d4 ��Ʒ�б�����
	ItemTable* item_table;//0x38d8		��Ʒ��
	char unknow3[0x4];//0x38dc
	TerrainData* terrain;//0x38e0
	UnitData* doodas;//0x38e4
	UnitData* units;//0x38e8
	RegionData* regions;//0x38ec
	struct TriggerData* triggers;//0x38f0 //�����༭������
	CameraData* cameras; //0x38f4
	void* objects;//0x38f8
	SoundData* sounds; //0x38fc
	TriggerStringData* strings;//0x3900

	char unknow4[0x128];//0x3904
	uint32_t is_test;	//0x3a2c ��ǰ�Ƿ��ǲ��Ա���ģʽ
	char unknow5[0x28];//0x3a30
	const char* mappath; //0x3a58 ��ǰ��ͼ·��ָ��
};
