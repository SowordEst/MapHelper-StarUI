	case "StarLoadAnyTypeDataByUserData"s_hash:
	case "StarAccumulator"s_hash:
	case "StarFlushAnyTypeDataByUserData"s_hash:
	case "StarHaveSavedAnyTypeDataByUserData"s_hash:
	case "SUTL_UnitAddEventCallBack"s_hash:
	case "SUTL_UnitRemoveEventCallBack"s_hash:
	case "StarExecuteUnitEvent"s_hash:
	case "StarAny2I"s_hash:
	case "STypes_Types2I"s_hash:
	case "StarExecuteUnitEventII"s_hash:
		setParamerType(action, flag, 0, 1);
		break;
	case "SH_SaveAny"s_hash:
		setParamerType(action, flag, 3, 4);
		break;
	case "StarSaveAnyTypeDataByUserData"s_hash:
		setParamerType(action, flag, 0, 1);
		setParamerType(action, flag, 3, 4);
		break;
	case "ExLOC"s_hash:
		setParamerType(action, flag, 0, 2);
		break;
	case "ExLOCSET"s_hash:
		setParamerType(action, flag, 0, 3);
		break;
		//该文件配置为 不参与生成