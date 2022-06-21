#pragma once


namespace mh {

	//������Ҫ����������εĽڵ㶼Ӧ�ü̳�ClosureNode

	//����һ����̬���� + ����ClosureNode�Ĺ��췽��
#define REGISTER_FROM_CLOSUER(name) REGISTER_FROM(name,	Action*); name(Action* param, uint32_t index, NodePtr parent): ClosureNode(param, index, parent) { } ;

	typedef std::shared_ptr<class ClosureNode> ClosureNodePtr;

	class ClosureNode : public ActionNode {
	public:
		REGISTER_FROM_ACTION(ClosureNode)
	
		//�������������
		virtual int getCrossDomainIndex() { return 0; }

		//�Ƿ��Զ���������ֲ�����  ���Ʊհ����������
		virtual bool isVariableCrossDomain() {  return true; }

		//�Ƿ��Զ����� ��ȡ������λ ��ȡ������� ��Щ����ֵ
		virtual bool isFunctionCrossDomain() { return true; }

		virtual TYPE getType() override {  return TYPE::CLOSURE; }

		virtual std::string toString(TriggerFunction* func) override { 
			return std::string();
		}
	
		int getCurrentGroupId() { return m_current_group_id; }

		virtual FunctionPtr getBlock(TriggerFunction* func, 
			const std::string& closure_name, 
			const std::string& return_type,
			std::string& upvalues, 
			std::vector<std::string>& upactions
		) {
			std::vector<NodePtr> upvalues_scope; //���в������Ľڵ�
			std::vector<NodePtr> closure_scope;  //���ж������Ľڵ�

			m_current_group_id = -1;

			//����
			for (auto& node : getChildList()) {
				Action* action = (Action*)node->getData();
				if ((int)action->group_id <= getCrossDomainIndex() && getCrossDomainIndex() >= 0) {
					upvalues_scope.push_back(node);
				} else {
					closure_scope.push_back(node);
				}
			}

			//�����¼��Լ��������Ĵ��� �������������������ֲ�������
			for (auto& node : upvalues_scope) {
				Action* action = (Action*)node->getData();
				m_current_group_id = action->group_id;
	
				if (m_current_group_id >= 0 && getCrossDomainIndex() >= 0 && (int)upactions.size() > m_current_group_id) {
					upactions[m_current_group_id] += node->toString(func);
				}
			}

			std::map<std::string, Upvalue>* prev_upvalue_map_ptr = nullptr;

			//�ҵ���һ��հ� ������ֲ�������
			getValue([&](const NodePtr ptr) {
				if (ptr.get() != this) {
					if (ptr->getType() == TYPE::CLOSURE) {
						auto node = std::dynamic_pointer_cast<ClosureNode>(ptr);
						prev_upvalue_map_ptr = &node->upvalue_map;
						return true;
					} 
				}
				return false;
			});
			FunctionPtr closure;
			if (return_type == "StarBool") {
				closure = FunctionPtr(new Function(closure_name, "boolean"));
			}
			else {
				closure = FunctionPtr(new Function(closure_name, return_type));
			}
			
			func->push(closure);
			//���ɱհ�����
			int i = 0;
			for (auto& node : closure_scope) {
				m_current_group_id = ((Action*)node->getData())->group_id;
				if (++i == closure_scope.size() && return_type != "nothing"&& return_type !="StarBool") { //���һ��
					*closure << func->getSpaces(-1) + "__RETURN__";
					*closure << func->getSpaces() + "return " + node->toString(func) + "\n";
				} else {
					*closure << node->toString(func);
				}
			}
			func->pop();

			
			//���������ڲ�����������ֲ����� ���ô���
			for (auto&& [n, v] : define_upvalue_map) {
				upvalue_map.erase(n);
			}
			
			for (auto&& [n, v] : upvalue_map) {
				//ֻ����2������ֲ����� ���Զ�����
				if (v.uptype != Upvalue::TYPE::GET_LOCAL && v.uptype != Upvalue::TYPE::SET_LOCAL) {
					continue;
				}

				//��ǰ���������� ������������Զ����δ��롣
				if ((!v.is_func && isVariableCrossDomain()) || (v.is_func && isFunctionCrossDomain())) {

					//���ɱ���״̬����
					Upvalue upvalue = { };
					auto set = Upvalue::TYPE::SET_LOCAL;
					auto get = Upvalue::TYPE::GET_LOCAL;
					//if (v.uptype == Upvalue::TYPE::GET_ARRAY || v.uptype == Upvalue::TYPE::SET_ARRAY) {
					//	set = Upvalue::TYPE::SET_ARRAY;
					//	get = Upvalue::TYPE::GET_ARRAY;
					//}
					upvalue.uptype = set;
					upvalue.name = v.name;
					upvalue.type = v.type;
					upvalue.is_func = v.is_func;
					upvalue.index = v.index;

					m_current_group_id = getCrossDomainIndex();
					upvalue.value = getUpvalue({ get, v.name, v.type, "", v.index, v.is_func });

					if (getCrossDomainIndex() >= 0) { //ӵ�в������� �Ž��б������
						upvalues += func->getSpaces() + "call " + getUpvalue(upvalue) + "\n";
					}
					m_current_group_id = -1;

					if (prev_upvalue_map_ptr && prev_upvalue_map_ptr != &upvalue_map) {
						//��֪ͨ��һ��հ� �����Ǳ���״̬
						prev_upvalue_map_ptr->emplace(n, upvalue);
					}
				}
			}

			upvalue_map.clear();

			m_current_group_id = -1;

			return closure;
		}

		virtual std::string getUpvalue(const Upvalue& info) override  {
			return std::string();
		}


	public:
		//����ֲ�������
		std::map<std::string, Upvalue> upvalue_map;

		//��������������������ľֲ�����
		std::map<std::string, Upvalue> define_upvalue_map;

		int m_current_group_id = -1;

		bool params_finish;
	
	};
}