#pragma once
#include "MathEntity.h"
#include "ExpressionType.h"
#include "Token.h"

enum class LogicOperation
{
	None, And, Or, Implies, Not, Forall, Exists
};

enum class Quantifier
{
	ForAll, Exists
};

class Variable
{
private:
	TokenString name;
	std::vector<ExpressionRef> refvars;
	Quantifier qtf;
	ExpressionType* type;

public:
	Variable(TokenString str, ExpressionRef type, Quantifier qtf)
		: str(str), qtf(qtf)
	{}

	TokenString get_name() const
	{
		return name;
	}

	ExpressionType* get_type() const
	{
		return type;
	}

	Quantifier get_qtf() const
	{
		return qtf;
	}
};

class Object : public MathEntity, public Variable
{
private:
	Token* name;
	RewritingRuleSet equiv_rules;
	RewritingRuleSet imp_rules;

	ExpressionRef def;
	FunctionProperty prop;

	std::vector<Object*>::const_iterator begin;
	std::vector<Object*>::const_iterator my_itr;

public:
	Object(Token* name, Scope&& scope, ExpressionRef type, ExpressionRef def)
		: GlobalEntity(std::move(scope)), Evaluatable(type), name(name), def(def)
	{}


	/*void set_defin(ExpressionRef defin)
	{
		this->eval_type = defin->eval();
	}*/

	const ExpressionRef get_def() const
	{
		return def;
	}

	Token* get_name()
	{
		return name;
	}

	RewritingRuleSet get_equiv_rules()
	{
		return equiv_rules;
	}

	RewritingRuleSet get_imp_rules()
	{
		return imp_rules;
	}

	void add_rewriting_rule(std::unique_ptr<RewritingRule>&&rule)
	{
		rules.push_back(std::move(rule));
	}

	FunctionProperty get_prop() const
	{
		return prop;
	}

	void enable_commutative()
	{
		prop.is_commutative = true;
	}

	void enable_associative()
	{
		prop.is_associative = true;
	}

	int get_simplity() const
	{
		return my_itr - begin;
	}
};
