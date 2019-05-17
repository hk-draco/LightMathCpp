#pragma once
#include "Simplicity.h"
#include "Object.h"
#include "Type.h"

class Scope
{
private:
	Scope* parent;
	CountableListBuilder<Variable> vars;

public:
	Scope() : parent(nullptr) {}
	Scope(Scope* parent) : parent(parent) {}

	void add_var(Variable&& var)
	{
		vars.push_back(Iteratable(std::move(var)));
	}

	Iteratable<Variable> get_vars() const
	{
		return vars;
	}

	const Variable* get_var(const TokenString& str) const
	{
		for (auto var : vars.range()) {
			if (var->get_name() == str)
				return var;
		}
		if (parent)
			return parent->get_var(str);
		return nullptr;
	}
};

enum class BuildInObject
{
	LogicAnd, LogicOr, LogicImplines
};

/*
class GlobalScope
{
private:
	std::map<TokenString, Object*> objs;
	std::map<TokenString, Type*> types;
	SimplicityManager simpl;

public:
	GlobalScope(MessageManager* msg) : simpl(msg) {}

	bool exists_obj(StringView name) const
	{
		return objs.count(name) > 0;
	}

	void add_obj(Object* obj)
	{
		objs[(TokenString)*obj->get_name()] = obj;
	}

	void add_type(Type* type)
	{
		types[(TokenString)*type->get_name()] = type;
	}

	void add_rule(RewritingRule* rule)
	{
		rule->get_before()->get_obj()->add_rewriting_rule(rule);
	}

	Object* get_obj(const TokenString& str) const
	{
		auto itr = objs.find(str);
		if (itr == objs.end())
			return nullptr;
		else
			return itr->second;
	}

	Type* get_type(const TokenString& str) const
	{
		auto itr = types.find(str);
		if (itr == types.end())
			return nullptr;
		else
			return itr->second;
	}

	SimplicityManager* get_simplicity()
	{
		return &simpl;
	}
};
*/