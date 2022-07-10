		//☆StarUI Start☆
		{"StarES_EX",								StarTriggerClosureTypeNode::From},//和下面那个实现相同
		{"EC_ShootReadyEx",							StarTriggerClosureTypeNode::From},//和上面那个实现相同
		{"EC_ShootReady_EffectEX",					StarTriggerClosureTypeNode::From},//和上面那个实现相同
			
		{"StarCodeBlockExecute",					StarExecuteTriggerEx::From },//和下方实现合并
		{"StarExecuteTriggerEx",					StarExecuteTriggerEx::From},//和上方实现合并
	
		{"LG_ForGroupSW",							StarGroupClosureTypeNode::From },//和下方实现合并
		{"EG_ForGroupSW",							StarGroupClosureTypeNode::From },//和上方实现合并

		{"StarExecuteEvent",						StarExecuteEvent::From },//和下方实现合并
		{"StarExecuteEventII",						StarExecuteEvent::From },//和上方实现合并

		{"StarExecuteUnitEvent",					StarExecuteHanleEvent::From },//和下方实现合并
		{"StarExecuteUnitEventII",					StarExecuteHanleEvent::From },//和上方实现合并

		{ "StarExecuteTriggerExReturn",				StarExecuteTriggerExReturn::From },
		{ "StarCodeBlockCreate",					StarCodeBlockCreate::From },

		{ "StarSaveAnyTypeDataByUserData",			StarSaveAnyTypeDataByUserData::From },//
		{ "StarLoadAnyTypeDataByUserData",			StarLoadAnyTypeDataByUserData::From },//
		{ "StarHaveSavedAnyTypeDataByUserData",		StarHaveSavedAnyTypeDataByUserData::From },//
		{ "StarFlushAnyTypeDataByUserData",			StarFlushAnyTypeDataByUserData::From },//
		{"StarAccumulator",                         StarAccumulator::From},
		{ "StarDelLoc",								StarDelLoc::From },//
		{ "SH_SaveAny",								SH_SaveAny::From },//
		{ "SH_LoadAny",								SH_LoadAny::From },//
		{ "StarAny2I",								StarAny2I::From },//
		{ "StarCenterTimerStartMultiple",			StarCenterTimerStartMultiple::From },//	
		{"StarCenterTimerExit",						StarCenterTimerExit::From },
		{ "StarLuaRegionMultiple",					StarLuaRegionMultiple::From },
		{ "StarChildrenBlockCreate",					StarChildrenBlockCreate::From },
		{ "StarChildrenBlockExecute",					StarChildrenBlockExecute::From },
		//☆StarUI End  ☆
		//该文件配置为 不参与生成