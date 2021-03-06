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
			ROOT, //根节点
			CALL, //不需要返回值
			GET,  //需要返回值
			PARAM, //是一个参数器
			CLOSURE, //一个逆天闭包
		};

		//获取当前触发器
		virtual void* getData() = 0;

		//获取当前节点名字
		virtual const std::string& getName() = 0;

		//当前名字id
		virtual uint32_t getNameId() = 0;

		//节点类型
		virtual TYPE getType() = 0;

		virtual void setType(TYPE type) = 0;

		//获取父节点
		virtual NodePtr getParentNode() = 0;

		//获取根节点
		virtual NodePtr getRootNode() = 0;

		//获取子动作列表
		virtual std::vector<NodePtr> getChildList() = 0;

		//过滤器
		typedef std::function<bool(NodePtr node)> NodeFilter;
		//获取值
		virtual bool getValue(const NodeFilter& filter) = 0;

		//获取文本
		virtual std::string toString(class TriggerFunction* func = nullptr) = 0;

		//生成函数名 
		virtual std::string getFuncName() = 0;

		//触发变量名
		virtual const std::string& getTriggerVariableName() = 0;

		//逆天局部变量的信息
		struct Upvalue{

			//逆天局部变量类型
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

		//生成逆天局部变量代码 
		virtual std::string getUpvalue(const Upvalue& info) = 0;
	};


	NodePtr NodeFromTrigger(Trigger* trigger);

	NodePtr NodeFromAction(Action* action, uint32_t childId, NodePtr parent);

	NodePtr NodeFramParameter(Parameter* parameter, uint32_t index, NodePtr parent);




	//需要初始化的触发器
	extern std::unordered_map<Trigger*, bool> g_initTriggerMap;
	//需要屏蔽的触发器
	extern std::unordered_map<Trigger*, bool> g_disableTriggerMap;


	typedef NodePtr(*MakeNode)(void* action, uint32_t childId, NodePtr parent);

	extern std::unordered_map<std::string, MakeNode> MakeActionNodeMap;
	extern std::unordered_map<std::string, MakeNode> MakeParameterNodeMap;

}

