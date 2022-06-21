#pragma once
#include "Node.h"
#include "ActionNode.hpp"
namespace mh {

#pragma region //☆生成子函数并传参的动作☆
	//以下实现相同,已合并
	//☆贝塞尔二阶弹幕☆ √
	//☆弹道计算-单位对单位☆ √
	//☆弹道计算-特效对单位☆ √
	class StarTriggerClosureTypeNode : public ClosureNode {
	public:
		REGISTER_FROM_CLOSUER(StarTriggerClosureTypeNode)

			virtual int getCrossDomainIndex() override { return 0; }//参数区的索引号 没有逆天参数时填入-1

			//是否自动传递逆天局部变量  类似闭包里跨域引用
		virtual bool isVariableCrossDomain() { return true; }

		//是否自动传递 获取触发单位 获取触发玩家 这些函数值
		virtual bool isFunctionCrossDomain() { return true; }

		std::string getHandleName() {
			if (getCurrentGroupId() <= getCrossDomainIndex()) {
				return "ydl_trigger";
			}
			return "GetTriggeringTrigger()";
		}

		virtual std::string getUpvalue(const Upvalue& info) override {
			if (!params_finish) { //如果是参数里的动作 就让他们访问上一级
				return getParentNode()->getUpvalue(info);
			}

			std::string result;

			bool is_get = info.uptype == Upvalue::TYPE::GET_LOCAL || info.uptype == Upvalue::TYPE::GET_ARRAY;
			if (getCrossDomainIndex() == getCurrentGroupId() && is_get) {	//如果当前是传参区 则使用上一层的局部变量
				result = getParentNode()->getUpvalue(info);
				return result;
			}

			switch (info.uptype) {
			case Upvalue::TYPE::SET_LOCAL:
				result = std::format("YDLocalSet({}, {}, \"{}\", {})", getHandleName(), info.type, info.name, info.value);
				break;
			case Upvalue::TYPE::GET_LOCAL:
				result = std::format("YDLocalGet({}, {}, \"{}\")", getHandleName(), info.type, info.name);
				break;
			case Upvalue::TYPE::SET_ARRAY:
				result = std::format("YDLocalArraySet({}, {}, \"{}\", {}, {})", getHandleName(), info.type, info.name, info.index, info.value);
				break;
			case Upvalue::TYPE::GET_ARRAY:
				result = std::format("YDLocalArrayGet({}, {}, \"{}\", {})", getHandleName(), info.type, info.name, info.index);
				break;
			default:
				break;
			}
			return result;
		}

		virtual std::string toString(TriggerFunction* func) override {

			auto params = getParameterList();
			auto& editor = get_trigger_editor();
			//添加局部变量
			func->current()->addLocal("ydl_trigger", "trigger", std::string(), false);

			std::string result = "";
			std::string save_state;
			std::string start;
			std::string name = editor.getScriptName(m_action);
			std::string func_name = getFuncName() + "T2";
			params_finish = false;
			result += func->getSpaces() + "set ydl_trigger = CreateTrigger()\n";

			//result += func->getSpaces() + "call TriggerAddCondition(ydl_trigger,Condition(function "
			//	+ func_name + "))\n";
			if (!(getParentNode()->getNameId() == "StarLuaRegionMultiple"s_hash)) {
				result += func->getSpaces() + "call TriggerAddCondition( ydl_trigger, Condition(function " + func_name + "))\n";
			}
			else {
				result += func->getSpaces() + "call TriggerAddCondition( ydl_trigger, Condition(code." + func_name + "))\n";
			}
			start = func->getSpaces() + "call " + name + "(";
			for (auto& param : getParameterList()) {
				start += " " + param->toString(func) + ",";
			}
			start += "ydl_trigger)\n";
			params_finish = true;

			std::vector<std::string> upactions(getCrossDomainIndex() + 1);

			FunctionPtr closure = getBlock(func, func_name, "nothing", save_state, upactions);
			result += upactions[0]; //参数区
			result += save_state;	//自动传参
			result += start;		//
			func->push(closure); // 设置为当前函数
			closure->insert_end += closure->getSpaces() +
				"call FlushChildHashtable(YDHT, GetHandleId(GetTriggeringTrigger()))\n";
			closure->insert_end += closure->getSpaces() +
				"call DestroyTrigger(GetTriggeringTrigger())\n";
			func->pop(); //弹出
			func->addFunction(closure); //添加到代码里

			return result;
		}

	};
	//生成子函数并传参的动作 End
#pragma endregion

#pragma region //☆逆天触发器运行类动作☆
	//☆触发器+静态动作块运行 - 带返回值☆ √
	class StarExecuteTriggerEx : public ClosureNode {
	public:
		REGISTER_FROM_CLOSUER(StarExecuteTriggerEx)
			std::string GetParentKey(NodePtr a) {
			NodePtr last_closure;
			getValue([&](NodePtr ptr) {
				if (ptr.get() != this && ptr->getType() == TYPE::CLOSURE) {
					last_closure = ptr;
					return true;
				}
				return false;
				});
			std::string ptype = "";
			if (last_closure) { // 在闭包里
				switch (last_closure->getNameId()) {
				case "StarChildrenBlockCreate"s_hash:
					ptype = "StarBlockKey";
					break;
				case "YDWETimerStartMultiple"s_hash: {//逆天计时器
					auto closure = std::dynamic_pointer_cast<YDWETimerStartMultiple>(last_closure);
					ptype = closure->getHandleName();
					ptype = "GetHandleId(" + ptype + ")";
					break;
				}
				case "YDWERegisterTriggerMultiple"s_hash: {//逆天触发器
					auto closure = std::dynamic_pointer_cast<YDWERegisterTriggerMultiple>(last_closure);
					ptype = closure->getHandleName();
					ptype = "GetHandleId(" + ptype + ")";
					break;
				}
				case "EC_ShootReadyEx"s_hash:
				case "EC_ShootReady_EffectEX"s_hash:
				case "StarES_EX"s_hash: {//逆天触发器
					auto closure = std::dynamic_pointer_cast<StarTriggerClosureTypeNode>(last_closure);
					ptype = closure->getHandleName();
					ptype = "GetHandleId(" + ptype + ")";
					break;
				}
				case "DzTriggerRegisterMouseEventMultiple"s_hash:
				case "DzTriggerRegisterKeyEventMultiple"s_hash:
				case "DzTriggerRegisterMouseWheelEventMultiple"s_hash:
				case "DzTriggerRegisterMouseMoveEventMultiple"s_hash:
				case "DzTriggerRegisterWindowResizeEventMultiple"s_hash:
				case "DzFrameSetUpdateCallbackMultiple"s_hash:
				case "DzFrameSetScriptMultiple"s_hash: {
					//网易界面函数逆天指针
					ptype = "StringHash(\"" + a->getFuncName() + "\")";
					break;
				}
				default:
					//其他闭包
					ptype = "YDHashH2I(GetTriggeringTrigger())*YDHashGet(YDLOC, integer, YDHashH2I(GetTriggeringTrigger()), 0xECE825E7)";
					break;
				}
			}
			else { //在根触发里
				ptype = "GetHandleId(GetTriggeringTrigger()) * ydl_localvar_step";
			}
			return ptype;
		}

		virtual std::string toString(TriggerFunction* func) override {

			auto params = getParameterList();

			NodePtr last_closure;
			getValue([&](NodePtr ptr) {
				if (ptr.get() != this && ptr->getType() == TYPE::CLOSURE) {
					last_closure = ptr;
					return true;
				}
				return false;
				});
			std::string result;
			if (last_closure && last_closure->getNameId() == "StarLuaRegionMultiple"s_hash) {
				result += "local ydl_trigger\n";
				result += "local ydl_triggerstep\n";
			}
			else {
				//添加局部变量
				func->current()->addLocal("ydl_triggerstep", "integer");
				func->current()->addLocal("ydl_trigger", "trigger", std::string(), false);
			}

			params_finish = false;
			if (getNameId() == "StarCodeBlockExecute"s_hash) {
				result += func->getSpaces()
					+ "set ydl_trigger = SCB_GetTrigger(" + params[0]->toString(func) + ")\n";
			}
			else {
				result += func->getSpaces()
					+ "set ydl_trigger = " + params[0]->toString(func) + "\n";
			}
			result += func->getSpaces() + "YDLocalExecuteTrigger(ydl_trigger)\n";


			if (getParentNode()->getType() == TYPE::ROOT) {
				result += func->getSpaces() + "call SaveInteger(YDHT,GetHandleId(ydl_trigger),SKey_PIndex,";
				result += "StarIndex1";
				result += ")\n";
			}
			else {
				std::string s1 = GetParentKey(this->getParentNode());
				if (!s1.empty()) {
					result += func->getSpaces() + "call SaveInteger(YDHT,GetHandleId(ydl_trigger),SKey_PIndex,";
					result += s1;
					result += ")\n";
				}
			}

			params_finish = true;
			for (auto& node : getChildList()) {
				result += node->toString(func);
			}
			if (params.size() > 1)
			{
				result += func->getSpaces()
					+ "call YDTriggerExecuteTrigger(ydl_trigger, "
					+ params[1]->toString(func) + ")\n";
			}
			else {
				result += func->getSpaces()
					+ "call YDTriggerExecuteTrigger(ydl_trigger, false)\n";
			}

			return result;
		}

		virtual std::string getUpvalue(const Upvalue& info) override {
			if (!params_finish) { //如果是参数里的动作 就让他们访问上一级
				return getParentNode()->getUpvalue(info);
			}

			std::string result;

			switch (info.uptype) {
			case Upvalue::TYPE::SET_LOCAL:
				result = std::format("YDLocal5Set({}, \"{}\", {})", info.type, info.name, info.value);
				break;
			case Upvalue::TYPE::GET_LOCAL:
				result = getParentNode()->getUpvalue(info);
				break;
			case Upvalue::TYPE::SET_ARRAY:
				result = std::format("YDLocal5ArraySet({}, \"{}\", {}, {})", info.type, info.name, info.index, info.value);
				break;
			case Upvalue::TYPE::GET_ARRAY:
				result = getParentNode()->getUpvalue(info);
				break;
			default:
				break;
			}
			return result;
		}
	};
	//☆返回值☆ √
	class StarExecuteTriggerExReturn : public ClosureNode {
	public:
		REGISTER_FROM_CLOSUER(StarExecuteTriggerExReturn)

			virtual std::string toString(TriggerFunction* func) override {

			auto params = getParameterList();

			NodePtr last_closure;
			getValue([&](NodePtr ptr) {
				if (ptr.get() != this && ptr->getType() == TYPE::CLOSURE) {
					last_closure = ptr;
					return true;
				}
				return false;
				});
			std::string result = "";
			if (getParentNode()->getNameId() == "StarLuaRegionMultiple"s_hash) {
				result += "local ydl_trigger\n";
				result += "local ydl_triggerstep\n";
			}
			else {
				//添加局部变量
				func->current()->addLocal("ydl_triggerstep", "integer");
				func->current()->addLocal("ydl_trigger", "trigger", std::string(), false);
			}

			params_finish = false;

			result += func->getSpaces() + "set ydl_trigger = GetTriggeringTrigger()\n";
			result += func->getSpaces() + "set Star_PIndex = LoadInteger(YDHT,GetHandleId(GetTriggeringTrigger()),SKey_PIndex)\n";

			params_finish = true;

			for (auto& node : getChildList()) {
				result += node->toString(func);
			}
			result += func->getSpaces() + "call RemoveSavedInteger(YDHT,GetHandleId(GetTriggeringTrigger()),SKey_PIndex)\n";

			return result;
		}


		virtual std::string getUpvalue(const Upvalue& info) override {
			if (!params_finish) { //如果是参数里的动作 就让他们访问上一级
				return getParentNode()->getUpvalue(info);
			}

			std::string result;

			switch (info.uptype) {
			case Upvalue::TYPE::SET_LOCAL:
				result = std::format("YDLocal7Set({}, \"{}\", {})", info.type, info.name, info.value);
				break;
			case Upvalue::TYPE::GET_LOCAL:
				result = getParentNode()->getUpvalue(info);
				break;
			case Upvalue::TYPE::SET_ARRAY:
				result = std::format("YDLocal7ArraySet({}, \"{}\", {}, {})", info.type, info.name, info.index, info.value);
				break;
			case Upvalue::TYPE::GET_ARRAY:
				result = getParentNode()->getUpvalue(info);
				break;
			default:
				break;
			}
			return result;
		}
	};
	//☆自定义事件☆   √
	class StarExecuteEvent : public ClosureNode {
	public:
		REGISTER_FROM_CLOSUER(StarExecuteEvent)
			std::string GetParentKey(NodePtr a) {
			NodePtr last_closure;
			getValue([&](NodePtr ptr) {
				if (ptr.get() != this && ptr->getType() == TYPE::CLOSURE) {
					last_closure = ptr;
					return true;
				}
				return false;
				});
			std::string ptype = "";
			if (last_closure) { // 在闭包里
				switch (last_closure->getNameId()) {
				case "StarChildrenBlockCreate"s_hash:
					ptype = "StarBlockKey";
					break;
				case "YDWETimerStartMultiple"s_hash: {//逆天计时器
					auto closure = std::dynamic_pointer_cast<YDWETimerStartMultiple>(last_closure);
					ptype = closure->getHandleName();
					ptype = "GetHandleId(" + ptype + ")";
					break;
				}
				case "YDWERegisterTriggerMultiple"s_hash: {//逆天触发器
					auto closure = std::dynamic_pointer_cast<YDWERegisterTriggerMultiple>(last_closure);
					ptype = closure->getHandleName();
					ptype = "GetHandleId(" + ptype + ")";
					break;
				}
				case "EC_ShootReadyEx"s_hash:
				case "EC_ShootReady_EffectEX"s_hash:
				case "StarES_EX"s_hash: {//逆天触发器
					auto closure = std::dynamic_pointer_cast<StarTriggerClosureTypeNode>(last_closure);
					ptype = closure->getHandleName();
					ptype = "GetHandleId(" + ptype + ")";
					break;
				}
				case "DzTriggerRegisterMouseEventMultiple"s_hash:
				case "DzTriggerRegisterKeyEventMultiple"s_hash:
				case "DzTriggerRegisterMouseWheelEventMultiple"s_hash:
				case "DzTriggerRegisterMouseMoveEventMultiple"s_hash:
				case "DzTriggerRegisterWindowResizeEventMultiple"s_hash:
				case "DzFrameSetUpdateCallbackMultiple"s_hash:
				case "DzFrameSetScriptMultiple"s_hash: {
					//网易界面函数逆天指针
					ptype = "StringHash(\"" + a->getFuncName() + "\")";
					break;
				}
				default:
					//其他闭包
					ptype = "YDHashH2I(GetTriggeringTrigger())*YDHashGet(YDLOC, integer, YDHashH2I(GetTriggeringTrigger()), 0xECE825E7)";
					break;
				}
			}
			else { //在根触发里
				ptype = "GetHandleId(GetTriggeringTrigger()) * ydl_localvar_step";
			}
			return ptype;
		}

		virtual std::string toString(TriggerFunction* func) override {
			auto params = getParameterList();

			NodePtr last_closure;
			getValue([&](NodePtr ptr) {
				if (ptr.get() != this && ptr->getType() == TYPE::CLOSURE) {
					last_closure = ptr;
					return true;
				}
				return false;
				});
			std::string result = "";
			if (getParentNode()->getNameId() == "StarLuaRegionMultiple"s_hash) {
				result += "local ydl_trigger\n";
				result += "local ydl_triggerstep\n";
			}
			else {
				//添加局部变量
				func->current()->addLocal("ydl_triggerstep", "integer");
				func->current()->addLocal("ydl_trigger", "trigger", std::string(), false);
			}

			params_finish = false;
			result += func->getSpaces() + "set STES_Hash = StringHash( " + params[0]->toString(func) + ")\n";

			result += func->getSpaces() + "set STES_Index = LoadInteger(STES_GetTable(),STES_Hash,skey_index)\n";
			result += func->getSpaces() + "set STES_LoopA = 0\n";
			result += func->getSpaces() + "loop\n";
			func->addSpace();
			result += func->getSpaces() + "exitwhen STES_LoopA>=STES_Index\n";
			result += func->getSpaces() + "set ydl_trigger = LoadTriggerHandle(STES_GetTable(),STES_Hash,STES_LoopA) \n";
			result += func->getSpaces() + "YDLocalExecuteTrigger(ydl_trigger)\n";

			if (getNameId() == "StarExecuteEventII"s_hash) {
				result += func->getSpaces() + "call SaveInteger(YDHT,GetHandleId(ydl_trigger),SKey_PIndex,";
				if (getParentNode()->getType() == TYPE::ROOT) {
					result += "StarIndex1";
				}
				else {
					result += GetParentKey(this->getParentNode());
				}
				result += ")\n";
			}
			params_finish = true;
			for (auto& node : getChildList()) {
				result += node->toString(func);
			}
			result += func->getSpaces() + "call YDTriggerExecuteTrigger(ydl_trigger, " + params[1]->toString(func) + ")\n";
			result += func->getSpaces() + "set STES_LoopA = STES_LoopA + 1\n";
			func->subSpace();
			result += func->getSpaces() + "endloop\n";

			return result;
		}

		virtual std::string getUpvalue(const Upvalue& info) override {
			if (!params_finish) { //如果是参数里的动作 就让他们访问上一级
				return getParentNode()->getUpvalue(info);
			}

			std::string result;

			switch (info.uptype) {
			case Upvalue::TYPE::SET_LOCAL:
				result = std::format("YDLocal5Set({}, \"{}\", {})", info.type, info.name, info.value);
				break;
			case Upvalue::TYPE::GET_LOCAL:
				result = getParentNode()->getUpvalue(info);
				break;
			case Upvalue::TYPE::SET_ARRAY:
				result = std::format("YDLocal5ArraySet({}, \"{}\", {}, {})", info.type, info.name, info.index, info.value);
				break;
			case Upvalue::TYPE::GET_ARRAY:
				result = getParentNode()->getUpvalue(info);
				break;
			default:
				break;
			}
			return result;
		}
	};
	//☆ 单位自定义事件☆   √  //其实是句柄类型均可
	class StarExecuteHanleEvent : public ClosureNode {
	public:
		REGISTER_FROM_CLOSUER(StarExecuteHanleEvent)
			std::string GetParentKey(NodePtr a) {
			NodePtr last_closure;
			getValue([&](NodePtr ptr) {
				if (ptr.get() != this && ptr->getType() == TYPE::CLOSURE) {
					last_closure = ptr;
					return true;
				}
				return false;
				});
			std::string ptype = "";
			if (last_closure) { // 在闭包里
				switch (last_closure->getNameId()) {
				case "StarChildrenBlockCreate"s_hash:
					ptype = "StarBlockKey";
					break;
				case "YDWETimerStartMultiple"s_hash: {//逆天计时器
					auto closure = std::dynamic_pointer_cast<YDWETimerStartMultiple>(last_closure);
					ptype = closure->getHandleName();
					ptype = "GetHandleId(" + ptype + ")";
					break;
				}
				case "YDWERegisterTriggerMultiple"s_hash: {//逆天触发器
					auto closure = std::dynamic_pointer_cast<YDWERegisterTriggerMultiple>(last_closure);
					ptype = closure->getHandleName();
					ptype = "GetHandleId(" + ptype + ")";
					break;
				}
				case "EC_ShootReadyEx"s_hash:
				case "EC_ShootReady_EffectEX"s_hash:
				case "StarES_EX"s_hash: {//逆天触发器
					auto closure = std::dynamic_pointer_cast<StarTriggerClosureTypeNode>(last_closure);
					ptype = closure->getHandleName();
					ptype = "GetHandleId(" + ptype + ")";
					break;
				}
				case "DzTriggerRegisterMouseEventMultiple"s_hash:
				case "DzTriggerRegisterKeyEventMultiple"s_hash:
				case "DzTriggerRegisterMouseWheelEventMultiple"s_hash:
				case "DzTriggerRegisterMouseMoveEventMultiple"s_hash:
				case "DzTriggerRegisterWindowResizeEventMultiple"s_hash:
				case "DzFrameSetUpdateCallbackMultiple"s_hash:
				case "DzFrameSetScriptMultiple"s_hash: {
					//网易界面函数逆天指针
					ptype = "StringHash(\"" + a->getFuncName() + "\")";
					break;
				}
				default:
					//其他闭包
					ptype = "YDHashH2I(GetTriggeringTrigger())*YDHashGet(YDLOC, integer, YDHashH2I(GetTriggeringTrigger()), 0xECE825E7)";
					break;
				}
			}
			else { //在根触发里
				ptype = "GetHandleId(GetTriggeringTrigger()) * ydl_localvar_step";
			}
			return ptype;
		}

		virtual std::string toString(TriggerFunction* func) override {
			auto params = getParameterList();

			NodePtr last_closure;
			getValue([&](NodePtr ptr) {
				if (ptr.get() != this && ptr->getType() == TYPE::CLOSURE) {
					last_closure = ptr;
					return true;
				}
				return false;
				});
			std::string result = "";
			if (getParentNode()->getNameId() == "StarLuaRegionMultiple"s_hash) {
				result += "local ydl_trigger\n";
				result += "local ydl_triggerstep\n";
			}
			else {
				//添加局部变量
				func->current()->addLocal("ydl_triggerstep", "integer");
				func->current()->addLocal("ydl_trigger", "trigger", std::string(), false);
			}

			params_finish = false;
			result += func->getSpaces()
				+ "set STES_Hash = " + params[2]->toString(func) + " + "
				+ "GetHandleId("
				+ params[1]->toString(func) + ")\n";

			result += func->getSpaces() + "set STES_Index = LoadInteger(SUTL_HT,STES_Hash,skey_index)\n";
			result += func->getSpaces() + "set STES_LoopA = 0\n";
			result += func->getSpaces() + "loop\n";
			func->addSpace();
			result += func->getSpaces() + "exitwhen STES_LoopA>=STES_Index\n";
			result += func->getSpaces() + "set ydl_trigger = LoadTriggerHandle(SUTL_HT,STES_Hash,STES_LoopA) \n";
			result += func->getSpaces() + "YDLocalExecuteTrigger(ydl_trigger)\n";
			if (getNameId() == "StarExecuteUnitEventII"s_hash) {
				result += func->getSpaces() + "call SaveInteger(YDHT,GetHandleId(ydl_trigger),SKey_PIndex,";
				if (getParentNode()->getType() == TYPE::ROOT) {
					result += "StarIndex1";
				}
				else {
					result += GetParentKey(this->getParentNode());
				}
				result += ")\n";
			}
			params_finish = true;
			for (auto& node : getChildList()) {
				result += node->toString(func);
			}
			result += func->getSpaces() + "call YDTriggerExecuteTrigger(ydl_trigger," + params[3]->toString(func) + ")\n";
			result += func->getSpaces() + "set STES_LoopA = STES_LoopA + 1\n";
			func->subSpace();
			result += func->getSpaces() + "endloop\n";

			return result;
		}

		virtual std::string getUpvalue(const Upvalue& info) override {
			if (!params_finish) { //如果是参数里的动作 就让他们访问上一级
				return getParentNode()->getUpvalue(info);
			}

			std::string result;

			switch (info.uptype) {
			case Upvalue::TYPE::SET_LOCAL:
				result = std::format("YDLocal5Set({}, \"{}\", {})", info.type, info.name, info.value);
				break;
			case Upvalue::TYPE::GET_LOCAL:
				result = getParentNode()->getUpvalue(info);
				break;
			case Upvalue::TYPE::SET_ARRAY:
				result = std::format("YDLocal5ArraySet({}, \"{}\", {}, {})", info.type, info.name, info.index, info.value);
				break;
			case Upvalue::TYPE::GET_ARRAY:
				result = getParentNode()->getUpvalue(info);
				break;
			default:
				break;
			}
			return result;
		}
	};
	//逆天触发器运行类动作 End
#pragma endregion
#pragma region //☆可变类型的动作☆
	//☆ 字符串参数逆天自定义值 存☆ √
	class StarSaveAnyTypeDataByUserData : public ActionNode {
	public:
		REGISTER_FROM_ACTION(StarSaveAnyTypeDataByUserData)

			virtual std::string toString(TriggerFunction* func) override {
			auto params = getParameterList();

			//typename_01_integer  + 11 = integer
			std::string result = func->getSpaces();

			result += "call YDUserDataSet2(";
			result += std::string(m_action->parameters[0]->value + 11) + ", ";
			result += params[1]->toString(func) + ",";
			result += params[2]->toString(func) + ", ";
			result += std::string(m_action->parameters[3]->value + 11) + ", ";
			result += params[4]->toString(func) + ")\n";
			return  result;
		}
	};
	//☆ 字符串参数逆天自定义值 清☆ √
	class StarFlushAnyTypeDataByUserData : public ActionNode {
	public:
		REGISTER_FROM_ACTION(StarFlushAnyTypeDataByUserData)

			virtual std::string toString(TriggerFunction* func) override {

			auto params = getParameterList();

			//typename_01_integer  + 11 = integer
			std::string result = func->getSpaces();

			result += "call YDUserDataClear2(";
			result += std::string(m_action->parameters[0]->value + 11) + ", ";
			result += params[1]->toString(func) + ",";
			result += std::string(m_action->parameters[2]->value + 11) + ", ";
			result += params[3]->toString(func) + ")\n";

			return result;
		}
	};
	//☆ 逆天句柄类型变量清除 ☆ √
	class StarDelLoc : public ActionNode {
	public:
		REGISTER_FROM_ACTION(StarDelLoc)

			virtual std::string toString(TriggerFunction* func) override {

			auto params = getParameterList();

			//typename_01_integer  + 11 = integer
			std::string result = func->getSpaces();
			NodePtr last_closure;
			getValue([&](NodePtr ptr) {
				if (ptr.get() != this && ptr->getType() == TYPE::CLOSURE) {
					last_closure = ptr;
					return true;
				}
				return false;
				});
			std::string ptype = "";
			if (last_closure) { // 在闭包里
				switch (last_closure->getNameId()) {
				case "YDWETimerStartMultiple"s_hash: {//逆天计时器
					auto closure = std::dynamic_pointer_cast<YDWETimerStartMultiple>(last_closure);
					ptype = closure->getHandleName();
					break;
				}
				case "YDWERegisterTriggerMultiple"s_hash: {//逆天触发器
					auto closure = std::dynamic_pointer_cast<YDWERegisterTriggerMultiple>(last_closure);
					ptype = closure->getHandleName();
					break;
				}
				case "EC_ShootReadyEx"s_hash:
				case "EC_ShootReady_EffectEX"s_hash:
				case "StarES_EX"s_hash: {//逆天触发器
					auto closure = std::dynamic_pointer_cast<StarTriggerClosureTypeNode>(last_closure);
					ptype = closure->getHandleName();
					break;
				}
				default:
					//其他闭包
					break;
				}
				if (!ptype.empty())
				{
					result += "call StarDelLoc(";
					result += ptype + ",\"";
					result += params[0]->toString(func);
					result += "\")\n";
				}


			}
			else { //在根触发里
				result += "call DELLOC1(\"";
				result += params[0]->toString(func);
				result += "\")\n";
			}


			return result;
		}
	};
	//☆ 字符串参数逆天自定义值 读☆ √
	class StarLoadAnyTypeDataByUserData : public ActionNode {
	public:
		REGISTER_FROM_ACTION(StarLoadAnyTypeDataByUserData)

			virtual std::string toString(TriggerFunction* func) override {

			auto params = getParameterList();

			ParameterNodePtr parent = std::dynamic_pointer_cast<ParameterNode>(getParentNode());
			Parameter* parent_parameter = (Parameter*)parent->getData();

			std::string result;
			result += "YDUserDataGet2(";
			result += m_action->parameters[0]->value + 11;//typename_01_integer  + 11 = integer
			result += ", ";
			result += params[1]->toString(func);
			result += ",";
			result += params[2]->toString(func);
			result += ", ";
			result += parent_parameter->type_name;
			result += ")";
			return result;
		}
	};
	//☆ 字符串参数逆天自定义值 查☆ √
	class StarHaveSavedAnyTypeDataByUserData : public ActionNode {
	public:
		REGISTER_FROM_ACTION(StarHaveSavedAnyTypeDataByUserData)

			virtual std::string toString(TriggerFunction* func) override {
			auto params = getParameterList();

			std::string result;
			result += "YDUserDataHas2(";
			result += m_action->parameters[0]->value + 11;//typename_01_integer  + 11 = integer
			result += ", ";
			result += params[1]->toString(func);
			result += ", ";
			result += m_action->parameters[2]->value + 11;
			result += ",";
			result += params[3]->toString(func);
			result += ")";
			return result;
		}
	};
	//☆ 哈希表 写 ☆  √
	class SH_SaveAny : public ActionNode {
	public:
		REGISTER_FROM_ACTION(SH_SaveAny)

			virtual std::string toString(TriggerFunction* func) override {
			auto params = getParameterList();
			std::string result = func->getSpaces();
			result += "call SH_SaveAnyValue(";
			result += params[0]->toString() + ",";
			result += params[1]->toString() + ",";
			result += params[2]->toString() + ",startype2ID(";
			result += params[3]->toString() + "),";
			result += params[4]->toString(); +",";
			result += ")\n";
			return  result;
		}
	};
	//☆ 哈希表 读 ☆  √
	class SH_LoadAny : public ActionNode {
	public:
		REGISTER_FROM_ACTION(SH_LoadAny)

			virtual std::string toString(TriggerFunction* func) override {

			auto params = getParameterList();

			ParameterNodePtr parent = std::dynamic_pointer_cast<ParameterNode>(getParentNode());
			Parameter* parent_parameter = (Parameter*)parent->getData();

			std::string result;
			result += "SH_LoadAnyValue(";
			result += params[0]->toString() + ",";
			result += params[1]->toString() + ",";
			result += params[2]->toString() + ",startype2ID(";
			result += params[3]->toString() + ")";
			result += ")";
			return result;
		}
	};
	class StarAny2I : public ActionNode {
	public:
		REGISTER_FROM_ACTION(StarAny2I)

			virtual std::string toString(TriggerFunction* func) override {

			auto params = getParameterList();

			ParameterNodePtr parent = std::dynamic_pointer_cast<ParameterNode>(getParentNode());
			Parameter* parent_parameter = (Parameter*)parent->getData();

			std::string result;
			result += "StarAny2I(";
			result += m_action->parameters[0]->value + 11;
			result += "," + params[1]->toString() + "";
			result += ")";
			return result;
		}
	};

	//
	//=========以下只需要可变类型
	//"SUTL_UnitAddEventCallBack", //只需要可变类型
	//"SUTL_UnitRemoveEventCallBack", //只需要可变类型
	//"SH_SaveAny", //只需要可变类型
	//"SH_LoadAny", //只需要可变类型
	//"SH_HaveAny", //只需要可变类型
	//"StarAny2I", //只需要可变类型
	//"ExLOCSET",//只需要可变类型
	//"ExLOC"//只需要可变类型
	//"STypes_Types2I",//只需要可变类型
	//可变类型的动作 End
#pragma endregion
#pragma region //☆组动作☆ 特效组+闪电效果组
	class StarGroupClosureTypeNode : public ActionNode {
	public:
		REGISTER_FROM_ACTION(StarGroupClosureTypeNode)

			virtual std::string toString(TriggerFunction* func) override {

			auto params = getParameterList();
			params_finish = false;
			std::string name;
			std::string target;
			if (getNameId() == "LG_ForGroupSW"s_hash) {
				name = "LG";
				target = "Lightning";
			}
			else if (getNameId() == "EG_ForGroupSW"s_hash) {
				name = "EG";
				target = "Effect";
			}
			else {
				return "";
			}
			std::string result = "";
			result += func->getSpaces() + "set Star_" + name + " = " + params[0]->toString(func) + "\n";
			result += func->getSpaces() + "set Star_" + name + "_ForValue = 0\n";
			result += func->getSpaces() + "set Star_" + name + "_ForIndex = " + name + "_GetCount(Star_" + name + ")\n";
			params_finish = true;
			result += func->getSpaces() + "loop\n";
			func->addSpace();
			result += func->getSpaces() + "set " + name + "_Callback" + target + " = " + name + "_GetAt(Star_" + name + ",Star_" + name + "_ForValue)\n";
			for (auto& node : getChildList()) {
				result += node->toString(func);
			}
			result += func->getSpaces() + "set Star_" + name + "_ForValue = Star_" + name + "_ForValue + 1\n";
			result += func->getSpaces() + "exitwhen Star_" + name + "_ForValue>=Star_" + name + "_ForIndex\n";
			func->subSpace();
			result += func->getSpaces() + "endloop\n";

			return result;
		}

	public:
		bool params_finish = false;
	};
#pragma endregion
#pragma region //☆静态动作块☆
	//☆静态动作块☆
	class StarCodeBlockCreate : public ClosureNode {
	public:
		REGISTER_FROM_CLOSUER(StarCodeBlockCreate)

			virtual int getCrossDomainIndex() override { return -1; }//参数区的索引号 没有逆天参数时填入-1

		virtual std::string getUpvalue(const Upvalue& info) override {
			if (!params_finish) { //如果是参数里的动作 就让他们访问上一级
				return getParentNode()->getUpvalue(info);
			}
			std::string result;
			bool is_get = info.uptype == Upvalue::TYPE::GET_LOCAL || info.uptype == Upvalue::TYPE::GET_ARRAY;
			if (getCrossDomainIndex() == getCurrentGroupId() && is_get) {	//如果当前是传参区 则使用上一层的局部变量
				result = getParentNode()->getUpvalue(info);
				return result;
			}
			//内部逆天变量视为在正常触发器内
			switch (info.uptype) {
			case Upvalue::TYPE::SET_LOCAL:
				result = std::format("YDLocal2Set({}, \"{}\", {})", info.type, info.name, info.value);
				break;
			case Upvalue::TYPE::GET_LOCAL:
				result = std::format("YDLocal2Get({}, \"{}\")", info.type, info.name);
				break;
			case Upvalue::TYPE::SET_ARRAY:
				result = std::format("YDLocal2ArraySet({}, \"{}\", {}, {})", info.type, info.name, info.index, info.value);
				break;
			case Upvalue::TYPE::GET_ARRAY:
				result = std::format("YDLocal2ArrayGet({}, \"{}\", {})", info.type, info.name, info.index);
				break;
			default:
				break;
			}
			return result;
		}

		virtual std::string toString(TriggerFunction* func) override {
			auto root = std::dynamic_pointer_cast<TriggerNode>(getRootNode());
			auto params = getParameterList();

			std::string result;
			std::string save_state;
			std::string func_name = getFuncName() + "TI";
			std::string init_func_name = "StarBlockInit_" + func_name + "I";
			params_finish = false;

			params_finish = true;

			std::vector<std::string> upactions(getCrossDomainIndex() + 1);

			FunctionPtr closure = getBlock(func, func_name, "nothing", save_state, upactions);
			func->push(closure);
			closure->insert_begin += closure->getSpaces() + "YDLocalInitialize()\n";
			//清除返回值的key
			closure->insert_end += closure->getSpaces() + "call RemoveSavedInteger(YDHT, GetHandleId(GetTriggeringTrigger()), SKey_PIndex)\n";
			closure->insert_end += closure->getSpaces() + "call YDLocal1Release()\n";
			func->pop();
			func->addFunction(closure); //添加到代码里

			closure = FunctionPtr(new Function(init_func_name, "nothing")); //空函数对象
			func->push(closure); // 设置为当前函数
			*closure << closure->getSpaces() + "local trigger t = CreateTrigger()\n";
			*closure << closure->getSpaces() + "call SCB_Register(" + params[0]->toString(func) + ",\"" + init_func_name + "\",t)" + "\n";
			*closure << closure->getSpaces() + "call TriggerAddAction(t,function " + func_name + ")" + "\n";
			*closure << closure->getSpaces() + "set t = null\n";

			func->pop();
			func->addFunction(closure);

			return result;
		}


	};
#pragma endregion
	//todo 调用时传递一个key 到 StarBlockKey 即可
#pragma region //☆静态子动作块☆
	//☆静态动作块☆
	class StarChildrenBlockCreate : public ClosureNode {
	public:
		REGISTER_FROM_CLOSUER(StarChildrenBlockCreate)

			virtual int getCrossDomainIndex() override { return -1; }//参数区的索引号 没有逆天参数时填入-1

		virtual std::string getUpvalue(const Upvalue& info) override {
			if (!params_finish) { //如果是参数里的动作 就让他们访问上一级
				return getParentNode()->getUpvalue(info);
			}
			std::string result;
			bool is_get = info.uptype == Upvalue::TYPE::GET_LOCAL || info.uptype == Upvalue::TYPE::GET_ARRAY;
			if (getCrossDomainIndex() == getCurrentGroupId() && is_get) {	//如果当前是传参区 则使用上一层的局部变量
				result = getParentNode()->getUpvalue(info);
				return result;
			}
			//内部逆天变量视为在调用者
			switch (info.uptype) {
			case Upvalue::TYPE::SET_LOCAL:
				result = std::format("YDLocal9Set({}, \"{}\", {})", info.type, info.name, info.value);
				break;
			case Upvalue::TYPE::GET_LOCAL:
				result = std::format("YDLocal9Get({}, \"{}\")", info.type, info.name);
				break;
			case Upvalue::TYPE::SET_ARRAY:
				result = std::format("YDLocal9ArraySet({}, \"{}\", {}, {})", info.type, info.name, info.index, info.value);
				break;
			case Upvalue::TYPE::GET_ARRAY:
				result = std::format("YDLocal9ArrayGet({}, \"{}\", {})", info.type, info.name, info.index);
				break;
			default:
				break;
			}
			return result;
		}

		virtual std::string toString(TriggerFunction* func) override {
			auto root = std::dynamic_pointer_cast<TriggerNode>(getRootNode());
			auto params = getParameterList();

			std::string result;
			std::string save_state;
			std::string func_name = getFuncName() + "TCI";
			std::string init_func_name = "StarBlockInit_" + func_name + "IC";
			params_finish = false;

			params_finish = true;

			std::vector<std::string> upactions(getCrossDomainIndex() + 1);

			FunctionPtr closure = getBlock(func, func_name, "nothing", save_state, upactions);
			func->push(closure);

			//清除返回值的key
			//closure->insert_end += closure->getSpaces() + "call RemoveSavedInteger(YDHT, GetHandleId(GetTriggeringTrigger()), SKey_PIndex)\n";

			func->pop();
			func->addFunction(closure); //添加到代码里

			closure = FunctionPtr(new Function(init_func_name, "nothing")); //空函数对象
			func->push(closure); // 设置为当前函数
			*closure << closure->getSpaces() + "local trigger t = CreateTrigger()\n";
			*closure << closure->getSpaces() + "call SCBC_Register(" + params[0]->toString(func) + ",\"" + init_func_name + "\",t)" + "\n";
			*closure << closure->getSpaces() + "call TriggerAddAction(t,function " + func_name + ")" + "\n";
			*closure << closure->getSpaces() + "set t = null\n";

			func->pop();
			func->addFunction(closure);

			return result;
		}


	};
#pragma endregion
	/*
			TODO List
			//=========以下类似逆天触发器运行
			"StarCodeBlockExecute",
			"StarExecuteEvent",
			"StarExecuteEventII",
			"StarExecuteUnitEvent",
			"StarExecuteUnitEventII",
			//=================================
			"StarDelLoc",//可变类型+自适应handle
			//=================================
			"LG_ForGroupSW",//类似逆天单位组选取
			"EG_ForGroupSW",//类似逆天单位组选取
			//=========以下类似逆天自定义值====
			"StarSaveAnyTypeDataByUserData",
			"StarLoadAnyTypeDataByUserData",
			"StarHaveSavedAnyTypeDataByUserData",
			"StarFlushAnyTypeDataByUserData",
			//已知问题: 逆天触发器运行相关动作无法正确的为逆天触发器传递参数

	*/
#pragma region 中心计时器

	//中心计时器
	class StarCenterTimerStartMultiple : public ClosureNode {
	public:
		REGISTER_FROM_CLOSUER(StarCenterTimerStartMultiple)

			virtual int getCrossDomainIndex() override { return 0; }

		//是否自动传递逆天局部变量  类似闭包里跨域引用
		virtual bool isVariableCrossDomain() { return true; }

		//是否自动传递 获取触发单位 获取触发玩家 这些函数值
		virtual bool isFunctionCrossDomain() { return false; }

		virtual std::string getUpvalue(const Upvalue& info) override {
			std::string result;


			if (!params_finish) { //如果是参数里的动作 就让他们访问上一级
				return getParentNode()->getUpvalue(info);
			}

			bool is_get = info.uptype == Upvalue::TYPE::GET_LOCAL || info.uptype == Upvalue::TYPE::GET_ARRAY;
			if ((getCrossDomainIndex() == getCurrentGroupId() && is_get) || !params_finish) {	//如果当前是传参区 则使用上一层的局部变量
				result = getParentNode()->getUpvalue(info);
				return result;
			}

			std::string func_name = getFuncName();

			switch (info.uptype) {
			case Upvalue::TYPE::SET_LOCAL:
				result = std::format("YDLocal7Set({}, \"{}\", {})", info.type, info.name, info.value);
				break;
			case Upvalue::TYPE::GET_LOCAL:
				result = std::format("YDLocal7Get({}, \"{}\")", info.type, info.name);
				break;
			case Upvalue::TYPE::SET_ARRAY:
				result = std::format("YDLocal7ArraySet({}, \"{}\", {}, {})", info.type, info.name, info.index, info.value);
			case Upvalue::TYPE::GET_ARRAY:
				result = std::format("YDLocal7ArrayGet({}, \"{}\", {})", info.type, info.name, info.index);
				break;
			default:
				break;
			}
			return result;
		}

		virtual std::string toString(TriggerFunction* func) override {

			auto params = getParameterList();

			std::string action_name = getName();
			action_name = action_name.replace(action_name.find("Multiple"), -1, "ByCode");

			std::string result;

			params_finish = false;

			std::string action;
			std::string func_name = "StarTimer_" + getFuncName();
			std::string start = func->getSpaces() + "call SST_CreateTimer("
				+ params[0]->toString(func) + ", "
				+ "function " + func_name + ")\n";
			std::string upvalues;

			params_finish = true;

			std::vector<std::string> upactions(getCrossDomainIndex() + 1);

			FunctionPtr closure = getBlock(func, func_name, "StarBool", upvalues, upactions);

			func->addFunction(closure);

			func->push(closure); // 设置为当前函数
			*closure << closure->getSpaces() + "return false\n";

			func->pop();
			result += "set Star_PIndex = SST_GetPIndex()\n";
			result += upactions[0]; //参数区
			result += upvalues; //自动传参代码
			result += start;

			return result;
		}

	};


	class StarCenterTimerExit : public ActionNode {
	public:
		REGISTER_FROM_ACTION(StarCenterTimerExit)

			virtual std::string toString(TriggerFunction* func) override {
			auto params = getParameterList();
			std::string result = func->getSpaces();
			result += "call FlushChildHashtable(YDLOC,Star_PIndex)\n";
			result += "return true\n";
			return  result;
		}
	};
	//lua动作块
	class StarLuaRegionMultiple : public ClosureNode {
	public:
		REGISTER_FROM_CLOSUER(StarLuaRegionMultiple)

		std::string GetParentKey(NodePtr a) {
			NodePtr last_closure;
			getValue([&](NodePtr ptr) {
				if (ptr.get() != this && ptr->getType() == TYPE::CLOSURE) {
					last_closure = ptr;
					return true;
				}
				return false;
				});
			std::string ptype = "";
			if (last_closure) { // 在闭包里
				switch (last_closure->getNameId()) {
				case "StarChildrenBlockCreate"s_hash:
					ptype = "StarBlockKey";
					break;
				case "YDWETimerStartMultiple"s_hash: {//逆天计时器
					auto closure = std::dynamic_pointer_cast<YDWETimerStartMultiple>(last_closure);
					ptype = closure->getHandleName();
					ptype = "GetHandleId(" + ptype + ")";
					break;
				}
				case "YDWERegisterTriggerMultiple"s_hash: {//逆天触发器
					auto closure = std::dynamic_pointer_cast<YDWERegisterTriggerMultiple>(last_closure);
					ptype = closure->getHandleName();
					ptype = "GetHandleId(" + ptype + ")";
					break;
				}
				case "EC_ShootReadyEx"s_hash:
				case "EC_ShootReady_EffectEX"s_hash:
				case "StarES_EX"s_hash: {//逆天触发器
					auto closure = std::dynamic_pointer_cast<StarTriggerClosureTypeNode>(last_closure);
					ptype = closure->getHandleName();
					ptype = "GetHandleId(" + ptype + ")";
					break;
				}
				case "DzTriggerRegisterMouseEventMultiple"s_hash:
				case "DzTriggerRegisterKeyEventMultiple"s_hash:
				case "DzTriggerRegisterMouseWheelEventMultiple"s_hash:
				case "DzTriggerRegisterMouseMoveEventMultiple"s_hash:
				case "DzTriggerRegisterWindowResizeEventMultiple"s_hash:
				case "DzFrameSetUpdateCallbackMultiple"s_hash:
				case "DzFrameSetScriptMultiple"s_hash: {
					//网易界面函数逆天指针
					ptype = "StringHash(\"" + a->getFuncName() + "\")";
					break;
				}
				default:
					//其他闭包
					ptype = "YDHashH2I(GetTriggeringTrigger())*YDHashGet(YDLOC, integer, YDHashH2I(GetTriggeringTrigger()), 0xECE825E7)";
					break;
				}
			}
			else { //在根触发里
				ptype = "GetHandleId(GetTriggeringTrigger()) * ydl_localvar_step";
			}
			return ptype;
		}

			virtual std::string getUpvalue(const Upvalue& info) override {
			std::string result;


			if (!params_finish) { //如果是参数里的动作 就让他们访问上一级
				return getParentNode()->getUpvalue(info);
			}

			bool is_get = info.uptype == Upvalue::TYPE::GET_LOCAL || info.uptype == Upvalue::TYPE::GET_ARRAY;
			if ((getCrossDomainIndex() == getCurrentGroupId() && is_get) || !params_finish) {	//如果当前是传参区 则使用上一层的局部变量
				result = getParentNode()->getUpvalue(info);
				return result;
			}

			std::string func_name = getFuncName();

			switch (info.uptype) {
			case Upvalue::TYPE::SET_LOCAL:
				result = std::format("YDLocal8Set({}, \"{}\", {})", info.type, info.name, info.value);
				break;
			case Upvalue::TYPE::GET_LOCAL:
				result = std::format("YDLocal8Get({}, \"{}\")", info.type, info.name);
				break;
			case Upvalue::TYPE::SET_ARRAY:
				result = std::format("YDLocal8ArraySet({}, \"{}\", {}, {})", info.type, info.name, info.index, info.value);
			case Upvalue::TYPE::GET_ARRAY:
				result = std::format("YDLocal8ArrayGet({}, \"{}\", {})", info.type, info.name, info.index);
				break;
			default:
				break;
			}
			return result;
		}

		virtual std::string toString(TriggerFunction* func) override {

			auto params = getParameterList();
			params_finish = false;
			std::string action;
			std::string upvalues;
			params_finish = true;
			//--------------------------------------------------
			std::string result = "";
			result += "set StarLuaKey = " + GetParentKey(this->getParentNode()) + "\n";//传递逆天指针
			result += "//!StarLua@" + params[0]->toString(func) + "\n";
			for (auto& node : getChildList()) {
				result += node->toString(func);
			}
			result += "//!EndStarLua\n";

			return result;
		}

	};


#pragma endregion

	class StarChildrenBlockExecute : public ActionNode {
	public:
		REGISTER_FROM_ACTION(StarChildrenBlockExecute)
			std::string GetParentKey(NodePtr a) {
			NodePtr last_closure;
			getValue([&](NodePtr ptr) {
				if (ptr.get() != this && ptr->getType() == TYPE::CLOSURE) {
					last_closure = ptr;
					return true;
				}
				return false;
				});
			std::string ptype = "";
			if (last_closure) { // 在闭包里
				switch (last_closure->getNameId()) {
				case "StarChildrenBlockCreate"s_hash:
					ptype = "StarBlockKey";
					break;
				case "YDWETimerStartMultiple"s_hash: {//逆天计时器
					auto closure = std::dynamic_pointer_cast<YDWETimerStartMultiple>(last_closure);
					ptype = closure->getHandleName();
					ptype = "GetHandleId(" + ptype + ")";
					break;
				}
				case "YDWERegisterTriggerMultiple"s_hash: {//逆天触发器
					auto closure = std::dynamic_pointer_cast<YDWERegisterTriggerMultiple>(last_closure);
					ptype = closure->getHandleName();
					ptype = "GetHandleId(" + ptype + ")";
					break;
				}
				case "EC_ShootReadyEx"s_hash:
				case "EC_ShootReady_EffectEX"s_hash:
				case "StarES_EX"s_hash: {//逆天触发器
					auto closure = std::dynamic_pointer_cast<StarTriggerClosureTypeNode>(last_closure);
					ptype = closure->getHandleName();
					ptype = "GetHandleId(" + ptype + ")";
					break;
				}
				case "DzTriggerRegisterMouseEventMultiple"s_hash:
				case "DzTriggerRegisterKeyEventMultiple"s_hash:
				case "DzTriggerRegisterMouseWheelEventMultiple"s_hash:
				case "DzTriggerRegisterMouseMoveEventMultiple"s_hash:
				case "DzTriggerRegisterWindowResizeEventMultiple"s_hash:
				case "DzFrameSetUpdateCallbackMultiple"s_hash:
				case "DzFrameSetScriptMultiple"s_hash: {
					//网易界面函数逆天指针
					ptype = "StringHash(\"" + a->getFuncName() + "\")";
					break;
				}
				default:
					//其他闭包
					ptype = "YDHashH2I(GetTriggeringTrigger())*YDHashGet(YDLOC, integer, YDHashH2I(GetTriggeringTrigger()), 0xECE825E7)";
					break;
				}
			}
			else { //在根触发里
				ptype = "GetHandleId(GetTriggeringTrigger()) * ydl_localvar_step";
			}
			return ptype;
		}
		virtual std::string toString(TriggerFunction* func) override {
			auto params = getParameterList();

			std::string result = func->getSpaces();
			result += "set StarBlockKey = " + GetParentKey(this->getParentNode());
			result += "\ncall TriggerExecute(SCBC_GetTrigger(" + params[0]->toString(func) + "))\n";
			//result += "call StarChildrenBlockExecute(";
			//result += params[0]->toString(func) + "\n";
			return  result;
		}
	};
}