#pragma once
#include "Node.h"
#include "ActionNode.hpp"
namespace mh {

#pragma region //�������Ӻ��������εĶ�����
	//����ʵ����ͬ,�Ѻϲ�
	//��������׵�Ļ�� ��
	//�������-��λ�Ե�λ�� ��
	//�������-��Ч�Ե�λ�� ��
	class StarTriggerClosureTypeNode : public ClosureNode {
	public:
		REGISTER_FROM_CLOSUER(StarTriggerClosureTypeNode)

			virtual int getCrossDomainIndex() override { return 0; }//�������������� û���������ʱ����-1

			//�Ƿ��Զ���������ֲ�����  ���Ʊհ����������
		virtual bool isVariableCrossDomain() { return true; }

		//�Ƿ��Զ����� ��ȡ������λ ��ȡ������� ��Щ����ֵ
		virtual bool isFunctionCrossDomain() { return true; }

		std::string getHandleName() {
			if (getCurrentGroupId() <= getCrossDomainIndex()) {
				return "ydl_trigger";
			}
			return "GetTriggeringTrigger()";
		}

		virtual std::string getUpvalue(const Upvalue& info) override {
			if (!params_finish) { //����ǲ�����Ķ��� �������Ƿ�����һ��
				return getParentNode()->getUpvalue(info);
			}

			std::string result;

			bool is_get = info.uptype == Upvalue::TYPE::GET_LOCAL || info.uptype == Upvalue::TYPE::GET_ARRAY;
			if (getCrossDomainIndex() == getCurrentGroupId() && is_get) {	//�����ǰ�Ǵ����� ��ʹ����һ��ľֲ�����
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
			//��Ӿֲ�����
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
			result += upactions[0]; //������
			result += save_state;	//�Զ�����
			result += start;		//
			func->push(closure); // ����Ϊ��ǰ����
			closure->insert_end += closure->getSpaces() +
				"call FlushChildHashtable(YDHT, GetHandleId(GetTriggeringTrigger()))\n";
			closure->insert_end += closure->getSpaces() +
				"call DestroyTrigger(GetTriggeringTrigger())\n";
			func->pop(); //����
			func->addFunction(closure); //��ӵ�������

			return result;
		}

	};
	//�����Ӻ��������εĶ��� End
#pragma endregion

#pragma region //�����촥���������ද����
	//�����+��̬���������� - ������ֵ�� ��
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
			if (last_closure) { // �ڱհ���
				switch (last_closure->getNameId()) {
				case "StarChildrenBlockCreate"s_hash:
					ptype = "StarBlockKey";
					break;
				case "YDWETimerStartMultiple"s_hash: {//�����ʱ��
					auto closure = std::dynamic_pointer_cast<YDWETimerStartMultiple>(last_closure);
					ptype = closure->getHandleName();
					ptype = "GetHandleId(" + ptype + ")";
					break;
				}
				case "YDWERegisterTriggerMultiple"s_hash: {//���촥����
					auto closure = std::dynamic_pointer_cast<YDWERegisterTriggerMultiple>(last_closure);
					ptype = closure->getHandleName();
					ptype = "GetHandleId(" + ptype + ")";
					break;
				}
				case "EC_ShootReadyEx"s_hash:
				case "EC_ShootReady_EffectEX"s_hash:
				case "StarES_EX"s_hash: {//���촥����
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
					//���׽��溯������ָ��
					ptype = "StringHash(\"" + a->getFuncName() + "\")";
					break;
				}
				default:
					//�����հ�
					ptype = "YDHashH2I(GetTriggeringTrigger())*YDHashGet(YDLOC, integer, YDHashH2I(GetTriggeringTrigger()), 0xECE825E7)";
					break;
				}
			}
			else { //�ڸ�������
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
				//��Ӿֲ�����
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
			if (!params_finish) { //����ǲ�����Ķ��� �������Ƿ�����һ��
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
	//���ֵ�� ��
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
				//��Ӿֲ�����
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
			if (!params_finish) { //����ǲ�����Ķ��� �������Ƿ�����һ��
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
	//���Զ����¼���   ��
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
			if (last_closure) { // �ڱհ���
				switch (last_closure->getNameId()) {
				case "StarChildrenBlockCreate"s_hash:
					ptype = "StarBlockKey";
					break;
				case "YDWETimerStartMultiple"s_hash: {//�����ʱ��
					auto closure = std::dynamic_pointer_cast<YDWETimerStartMultiple>(last_closure);
					ptype = closure->getHandleName();
					ptype = "GetHandleId(" + ptype + ")";
					break;
				}
				case "YDWERegisterTriggerMultiple"s_hash: {//���촥����
					auto closure = std::dynamic_pointer_cast<YDWERegisterTriggerMultiple>(last_closure);
					ptype = closure->getHandleName();
					ptype = "GetHandleId(" + ptype + ")";
					break;
				}
				case "EC_ShootReadyEx"s_hash:
				case "EC_ShootReady_EffectEX"s_hash:
				case "StarES_EX"s_hash: {//���촥����
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
					//���׽��溯������ָ��
					ptype = "StringHash(\"" + a->getFuncName() + "\")";
					break;
				}
				default:
					//�����հ�
					ptype = "YDHashH2I(GetTriggeringTrigger())*YDHashGet(YDLOC, integer, YDHashH2I(GetTriggeringTrigger()), 0xECE825E7)";
					break;
				}
			}
			else { //�ڸ�������
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
				//��Ӿֲ�����
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
			if (!params_finish) { //����ǲ�����Ķ��� �������Ƿ�����һ��
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
	//�� ��λ�Զ����¼���   ��  //��ʵ�Ǿ�����;���
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
			if (last_closure) { // �ڱհ���
				switch (last_closure->getNameId()) {
				case "StarChildrenBlockCreate"s_hash:
					ptype = "StarBlockKey";
					break;
				case "YDWETimerStartMultiple"s_hash: {//�����ʱ��
					auto closure = std::dynamic_pointer_cast<YDWETimerStartMultiple>(last_closure);
					ptype = closure->getHandleName();
					ptype = "GetHandleId(" + ptype + ")";
					break;
				}
				case "YDWERegisterTriggerMultiple"s_hash: {//���촥����
					auto closure = std::dynamic_pointer_cast<YDWERegisterTriggerMultiple>(last_closure);
					ptype = closure->getHandleName();
					ptype = "GetHandleId(" + ptype + ")";
					break;
				}
				case "EC_ShootReadyEx"s_hash:
				case "EC_ShootReady_EffectEX"s_hash:
				case "StarES_EX"s_hash: {//���촥����
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
					//���׽��溯������ָ��
					ptype = "StringHash(\"" + a->getFuncName() + "\")";
					break;
				}
				default:
					//�����հ�
					ptype = "YDHashH2I(GetTriggeringTrigger())*YDHashGet(YDLOC, integer, YDHashH2I(GetTriggeringTrigger()), 0xECE825E7)";
					break;
				}
			}
			else { //�ڸ�������
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
				//��Ӿֲ�����
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
			if (!params_finish) { //����ǲ�����Ķ��� �������Ƿ�����һ��
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
	//���촥���������ද�� End
#pragma endregion
#pragma region //��ɱ����͵Ķ�����
	//�� �ַ������������Զ���ֵ ��� ��
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
	//�� �ַ������������Զ���ֵ ��� ��
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
	//�� ���������ͱ������ �� ��
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
			if (last_closure) { // �ڱհ���
				switch (last_closure->getNameId()) {
				case "YDWETimerStartMultiple"s_hash: {//�����ʱ��
					auto closure = std::dynamic_pointer_cast<YDWETimerStartMultiple>(last_closure);
					ptype = closure->getHandleName();
					break;
				}
				case "YDWERegisterTriggerMultiple"s_hash: {//���촥����
					auto closure = std::dynamic_pointer_cast<YDWERegisterTriggerMultiple>(last_closure);
					ptype = closure->getHandleName();
					break;
				}
				case "EC_ShootReadyEx"s_hash:
				case "EC_ShootReady_EffectEX"s_hash:
				case "StarES_EX"s_hash: {//���촥����
					auto closure = std::dynamic_pointer_cast<StarTriggerClosureTypeNode>(last_closure);
					ptype = closure->getHandleName();
					break;
				}
				default:
					//�����հ�
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
			else { //�ڸ�������
				result += "call DELLOC1(\"";
				result += params[0]->toString(func);
				result += "\")\n";
			}


			return result;
		}
	};
	//�� �ַ������������Զ���ֵ ���� ��
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
	//�� �ַ������������Զ���ֵ ��� ��
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
	//�� ��ϣ�� д ��  ��
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
	//�� ��ϣ�� �� ��  ��
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
	//=========����ֻ��Ҫ�ɱ�����
	//"SUTL_UnitAddEventCallBack", //ֻ��Ҫ�ɱ�����
	//"SUTL_UnitRemoveEventCallBack", //ֻ��Ҫ�ɱ�����
	//"SH_SaveAny", //ֻ��Ҫ�ɱ�����
	//"SH_LoadAny", //ֻ��Ҫ�ɱ�����
	//"SH_HaveAny", //ֻ��Ҫ�ɱ�����
	//"StarAny2I", //ֻ��Ҫ�ɱ�����
	//"ExLOCSET",//ֻ��Ҫ�ɱ�����
	//"ExLOC"//ֻ��Ҫ�ɱ�����
	//"STypes_Types2I",//ֻ��Ҫ�ɱ�����
	//�ɱ����͵Ķ��� End
#pragma endregion
#pragma region //���鶯���� ��Ч��+����Ч����
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
#pragma region //�̬�������
	//�̬�������
	class StarCodeBlockCreate : public ClosureNode {
	public:
		REGISTER_FROM_CLOSUER(StarCodeBlockCreate)

			virtual int getCrossDomainIndex() override { return -1; }//�������������� û���������ʱ����-1

		virtual std::string getUpvalue(const Upvalue& info) override {
			if (!params_finish) { //����ǲ�����Ķ��� �������Ƿ�����һ��
				return getParentNode()->getUpvalue(info);
			}
			std::string result;
			bool is_get = info.uptype == Upvalue::TYPE::GET_LOCAL || info.uptype == Upvalue::TYPE::GET_ARRAY;
			if (getCrossDomainIndex() == getCurrentGroupId() && is_get) {	//�����ǰ�Ǵ����� ��ʹ����һ��ľֲ�����
				result = getParentNode()->getUpvalue(info);
				return result;
			}
			//�ڲ����������Ϊ��������������
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
			//�������ֵ��key
			closure->insert_end += closure->getSpaces() + "call RemoveSavedInteger(YDHT, GetHandleId(GetTriggeringTrigger()), SKey_PIndex)\n";
			closure->insert_end += closure->getSpaces() + "call YDLocal1Release()\n";
			func->pop();
			func->addFunction(closure); //��ӵ�������

			closure = FunctionPtr(new Function(init_func_name, "nothing")); //�պ�������
			func->push(closure); // ����Ϊ��ǰ����
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
	//todo ����ʱ����һ��key �� StarBlockKey ����
#pragma region //�̬�Ӷ������
	//�̬�������
	class StarChildrenBlockCreate : public ClosureNode {
	public:
		REGISTER_FROM_CLOSUER(StarChildrenBlockCreate)

			virtual int getCrossDomainIndex() override { return -1; }//�������������� û���������ʱ����-1

		virtual std::string getUpvalue(const Upvalue& info) override {
			if (!params_finish) { //����ǲ�����Ķ��� �������Ƿ�����һ��
				return getParentNode()->getUpvalue(info);
			}
			std::string result;
			bool is_get = info.uptype == Upvalue::TYPE::GET_LOCAL || info.uptype == Upvalue::TYPE::GET_ARRAY;
			if (getCrossDomainIndex() == getCurrentGroupId() && is_get) {	//�����ǰ�Ǵ����� ��ʹ����һ��ľֲ�����
				result = getParentNode()->getUpvalue(info);
				return result;
			}
			//�ڲ����������Ϊ�ڵ�����
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

			//�������ֵ��key
			//closure->insert_end += closure->getSpaces() + "call RemoveSavedInteger(YDHT, GetHandleId(GetTriggeringTrigger()), SKey_PIndex)\n";

			func->pop();
			func->addFunction(closure); //��ӵ�������

			closure = FunctionPtr(new Function(init_func_name, "nothing")); //�պ�������
			func->push(closure); // ����Ϊ��ǰ����
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
			//=========�����������촥��������
			"StarCodeBlockExecute",
			"StarExecuteEvent",
			"StarExecuteEventII",
			"StarExecuteUnitEvent",
			"StarExecuteUnitEventII",
			//=================================
			"StarDelLoc",//�ɱ�����+����Ӧhandle
			//=================================
			"LG_ForGroupSW",//�������쵥λ��ѡȡ
			"EG_ForGroupSW",//�������쵥λ��ѡȡ
			//=========�������������Զ���ֵ====
			"StarSaveAnyTypeDataByUserData",
			"StarLoadAnyTypeDataByUserData",
			"StarHaveSavedAnyTypeDataByUserData",
			"StarFlushAnyTypeDataByUserData",
			//��֪����: ���촥����������ض����޷���ȷ��Ϊ���촥�������ݲ���

	*/
#pragma region ���ļ�ʱ��

	//���ļ�ʱ��
	class StarCenterTimerStartMultiple : public ClosureNode {
	public:
		REGISTER_FROM_CLOSUER(StarCenterTimerStartMultiple)

			virtual int getCrossDomainIndex() override { return 0; }

		//�Ƿ��Զ���������ֲ�����  ���Ʊհ����������
		virtual bool isVariableCrossDomain() { return true; }

		//�Ƿ��Զ����� ��ȡ������λ ��ȡ������� ��Щ����ֵ
		virtual bool isFunctionCrossDomain() { return false; }

		virtual std::string getUpvalue(const Upvalue& info) override {
			std::string result;


			if (!params_finish) { //����ǲ�����Ķ��� �������Ƿ�����һ��
				return getParentNode()->getUpvalue(info);
			}

			bool is_get = info.uptype == Upvalue::TYPE::GET_LOCAL || info.uptype == Upvalue::TYPE::GET_ARRAY;
			if ((getCrossDomainIndex() == getCurrentGroupId() && is_get) || !params_finish) {	//�����ǰ�Ǵ����� ��ʹ����һ��ľֲ�����
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

			func->push(closure); // ����Ϊ��ǰ����
			*closure << closure->getSpaces() + "return false\n";

			func->pop();
			result += "set Star_PIndex = SST_GetPIndex()\n";
			result += upactions[0]; //������
			result += upvalues; //�Զ����δ���
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
	//lua������
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
			if (last_closure) { // �ڱհ���
				switch (last_closure->getNameId()) {
				case "StarChildrenBlockCreate"s_hash:
					ptype = "StarBlockKey";
					break;
				case "YDWETimerStartMultiple"s_hash: {//�����ʱ��
					auto closure = std::dynamic_pointer_cast<YDWETimerStartMultiple>(last_closure);
					ptype = closure->getHandleName();
					ptype = "GetHandleId(" + ptype + ")";
					break;
				}
				case "YDWERegisterTriggerMultiple"s_hash: {//���촥����
					auto closure = std::dynamic_pointer_cast<YDWERegisterTriggerMultiple>(last_closure);
					ptype = closure->getHandleName();
					ptype = "GetHandleId(" + ptype + ")";
					break;
				}
				case "EC_ShootReadyEx"s_hash:
				case "EC_ShootReady_EffectEX"s_hash:
				case "StarES_EX"s_hash: {//���촥����
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
					//���׽��溯������ָ��
					ptype = "StringHash(\"" + a->getFuncName() + "\")";
					break;
				}
				default:
					//�����հ�
					ptype = "YDHashH2I(GetTriggeringTrigger())*YDHashGet(YDLOC, integer, YDHashH2I(GetTriggeringTrigger()), 0xECE825E7)";
					break;
				}
			}
			else { //�ڸ�������
				ptype = "GetHandleId(GetTriggeringTrigger()) * ydl_localvar_step";
			}
			return ptype;
		}

			virtual std::string getUpvalue(const Upvalue& info) override {
			std::string result;


			if (!params_finish) { //����ǲ�����Ķ��� �������Ƿ�����һ��
				return getParentNode()->getUpvalue(info);
			}

			bool is_get = info.uptype == Upvalue::TYPE::GET_LOCAL || info.uptype == Upvalue::TYPE::GET_ARRAY;
			if ((getCrossDomainIndex() == getCurrentGroupId() && is_get) || !params_finish) {	//�����ǰ�Ǵ����� ��ʹ����һ��ľֲ�����
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
			result += "set StarLuaKey = " + GetParentKey(this->getParentNode()) + "\n";//��������ָ��
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
			if (last_closure) { // �ڱհ���
				switch (last_closure->getNameId()) {
				case "StarChildrenBlockCreate"s_hash:
					ptype = "StarBlockKey";
					break;
				case "YDWETimerStartMultiple"s_hash: {//�����ʱ��
					auto closure = std::dynamic_pointer_cast<YDWETimerStartMultiple>(last_closure);
					ptype = closure->getHandleName();
					ptype = "GetHandleId(" + ptype + ")";
					break;
				}
				case "YDWERegisterTriggerMultiple"s_hash: {//���촥����
					auto closure = std::dynamic_pointer_cast<YDWERegisterTriggerMultiple>(last_closure);
					ptype = closure->getHandleName();
					ptype = "GetHandleId(" + ptype + ")";
					break;
				}
				case "EC_ShootReadyEx"s_hash:
				case "EC_ShootReady_EffectEX"s_hash:
				case "StarES_EX"s_hash: {//���촥����
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
					//���׽��溯������ָ��
					ptype = "StringHash(\"" + a->getFuncName() + "\")";
					break;
				}
				default:
					//�����հ�
					ptype = "YDHashH2I(GetTriggeringTrigger())*YDHashGet(YDLOC, integer, YDHashH2I(GetTriggeringTrigger()), 0xECE825E7)";
					break;
				}
			}
			else { //�ڸ�������
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