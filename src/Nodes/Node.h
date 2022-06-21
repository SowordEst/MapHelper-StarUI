#pragma once
#include "..\stdafx.h"
#include <memory>
#include <functional>


int __fastcall fakeGetChildCount(Action* action);
Action::Type get_action_type(Action* action);



namespace mh {

	typedef std::shared_ptr<class Node> NodePtr;

	class Node :public std::enable_shared_from_this<Node> {
	public:
		enum class TYPE :int {
			ROOT, //���ڵ�
			CALL, //����Ҫ����ֵ
			GET,  //��Ҫ����ֵ
			PARAM, //��һ��������
			CLOSURE, //һ������հ�
		};

		//��ȡ��ǰ������
		virtual void* getData() = 0;

		//��ȡ��ǰ�ڵ�����
		virtual const std::string& getName() = 0;

		//��ǰ����id
		virtual uint32_t getNameId() = 0;

		//�ڵ�����
		virtual TYPE getType() = 0;

		virtual void setType(TYPE type) = 0;

		//��ȡ���ڵ�
		virtual NodePtr getParentNode() = 0;

		//��ȡ���ڵ�
		virtual NodePtr getRootNode() = 0;

		//��ȡ�Ӷ����б�
		virtual std::vector<NodePtr> getChildList() = 0;

		//������
		typedef std::function<bool(NodePtr node)> NodeFilter;
		//��ȡֵ
		virtual bool getValue(const NodeFilter& filter) = 0;

		//��ȡ�ı�
		virtual std::string toString(class TriggerFunction* func = nullptr) = 0;

		//���ɺ����� 
		virtual std::string getFuncName() = 0;

		//����������
		virtual const std::string& getTriggerVariableName() = 0;

		//����ֲ���������Ϣ
		struct Upvalue{

			//����ֲ���������
			enum class TYPE :int {
				SET_LOCAL,
				GET_LOCAL,
				SET_ARRAY,
				GET_ARRAY,
			};

			TYPE uptype;
			std::string name;
			std::string type;
			std::string value;
			std::string index;

			bool is_func = false;
		};

		//��������ֲ��������� 
		virtual std::string getUpvalue(const Upvalue& info) = 0;
	};


	NodePtr NodeFromTrigger(Trigger* trigger);

	NodePtr NodeFromAction(Action* action, uint32_t childId, NodePtr parent);

	NodePtr NodeFramParameter(Parameter* parameter, uint32_t index, NodePtr parent);




	//��Ҫ��ʼ���Ĵ�����
	extern std::unordered_map<Trigger*, bool> g_initTriggerMap;
	//��Ҫ���εĴ�����
	extern std::unordered_map<Trigger*, bool> g_disableTriggerMap;


	typedef NodePtr(*MakeNode)(void* action, uint32_t childId, NodePtr parent);

	extern std::unordered_map<std::string, MakeNode> MakeActionNodeMap;
	extern std::unordered_map<std::string, MakeNode> MakeParameterNodeMap;

}

