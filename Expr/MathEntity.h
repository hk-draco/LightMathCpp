#pragma once
#include "Expression.h"
#include "Scope.h"

struct MathEntity
{
private:
	Token* ident;
	ExpressionRef proof;
	std::map<StringView, StringView> names;
	StringView desc;
	ExpressionRef eval_type;

protected:
	Scope scope;

public:
	MathEntity() {}
	MathEntity(Scope&& scope) : scope(std::move(scope)) {}

	const ExpressionRef evaluate() const
	{
		return eval_type;
	}

	ExpressionRef get_type()
	{
		return eval_type;
	}

	Scope* get_scope()
	{
		return &scope;
	}

	const Scope* get_scope() const
	{
		return &scope;
	}

	bool set_desc(StringView desc)
	{
		if (this->desc.size())
			return false;
		this->desc = desc;
		return true;
	}

	bool set_proof(ExpressionRef proof, Area area)
	{
		if (this->proof)
			return false;
		this->proof = std::move(proof);
		return true;
	}

	bool set_name(StringView lang, StringView name)
	{
		if (names.count(lang)) {
			return false;
		}
		names[lang] = name;
		return true;
	}
};