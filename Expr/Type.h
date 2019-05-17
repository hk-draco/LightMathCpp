#pragma once
#include "MathEntity.h"

class TypingRule : public MathEntity
{
private:
	ExpressionRef expr;
	Type* type;
	MemberArray<ExpressionRef> type_args;

public:
	bool match(const ExpressionRef trg, RefArray<ExpressionRef> args) const;
};

class TypingRuleSet
{
private:
	ListBuilder<TypingRule> rules;

public:
	bool match(const ExpressionRef trg, RefArray<ExpressionRef> args) const;
};

class Type : public MathEntity
{
private:
	// src:    [Œ^]    T(a, b)
	// this:   [Œ^’è‹`] T(x, y: R)
	// base:   [Œ^]    U(x, y)
	// return: [Œ^]    U(a, b)
	TokenString name;
	ExpressionTypeFunction* base;
	ExpressionRef cond;
	TypingRuleSet typing;

public:
	Type(StringView name, Scope&& scope, ExpressionRef base, ExpressionRef cond)
		: MathEntity(std::move(scope)), name(name), base(base), cond(cond)
	{}

	bool cast(const ExpressionRef trg, RefArray<ExpressionRef> args);

	StringView get_name() const
	{
		return name;
	}

	const ExpressionRef get_base() const
	{
		return base;
	}

	const ExpressionRef get_cond() const
	{
		return cond;
	}
};
