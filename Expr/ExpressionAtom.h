#pragma once
#include "Expression.h"
#include "RewritingInfo.h"
#include "Token.h"

class ExpressionHeadHolder : public Expression
{
protected:
	union
	{
		std::intptr_t ident;
		const Token* token;
		const Variable* var;
	};

public:
	ExpressionHeadHolder(std::intptr_t ident) : ident(ident) {}
	ExpressionHeadHolder(Token* token) : token(token) {}
	ExpressionHeadHolder(Variable* var) : var(var) {}

	const Variable* get_var() const
	{
		return var;
	}
};

class ExpressionAtom : public ExpressionHeadHolder
{
protected:
	bool is_constant;

public:
	ExpressionAtom(std::intptr_t ident, bool is_constant) : ExpressionHeadHolder(ident), is_constant(is_constant) {}
	ExpressionAtom(Token* head) : ExpressionHeadHolder(head) {}
	ExpressionAtom(Variable* var) : ExpressionHeadHolder(var) {}
	ExpressionAtom(Object* obj) : ExpressionHeadHolder(obj) {}

	ExpressionRef clone_latest() const
	{
		return new ExpressionAtom(ident, is_constant);
	}

	bool equals(const ExpressionRef trg) const
	{
		auto atom = trg.cast<ExpressionAtom>();
		return atom && obj == atom->obj;
	}

	bool unify(const ExpressionRef trg, AssignTable* asg) const
	{
		if (is_constant) {
			return equals(trg);
		} else {
			if (auto expr = (*asg)[var])
				return expr->equals(trg);
			else
				(*asg)[var] = trg;
			return true;
		}
	}

	ExpressionRef assign(const AssignTable& asg) const
	{
		if (is_constant)
			return (ExpressionRef)this;
		else
			return asg[var]->clone();
	}

	CompareResult compare(const ExpressionRef trg) const
	{
		if (auto atom = trg.cast<ExpressionAtom>())
			return obj->get_name() < atom->obj->get_name();
		return false;
	}

	bool complete(Scope* scope, MessageManager* msg)
	{
	}

	Expression* evaluate() const override
	{

	}
};