#pragma once
#include "MathEntity.h"
#include "Token.h"
#include "RewritingInfo.h"
#include "Expression.h"

class RewritingRule : public MathEntity
{
private:
	bool is_imply;
	ExpressionRef before;
	ExpressionRef after;
	Area area;

public:
	RewritingRule(Scope&& scope, ExpressionRef before, ExpressionRef after, bool is_imply, Area area)
		: MathEntity(std::move(scope)), before(before), after(after), is_imply(is_imply), area(area)
	{}

	bool rewrite(ExpressionRef* trg, RewritingType type);
	bool rewrite(ExpressionRef* trg, ExpressionRef goal, RewritingType type);

	bool is_commutative_law();
	bool is_associative_law();

	void set_is_imply(bool is_imply)
	{
		this->is_imply = is_imply;
	}

	void write(String* buf) const
	{
		before->write(buf, ExpressionShowMode::Latest);
		if (is_imply)
			buf->append(" -> ");
		else
			buf->append(" => ");
		after->write(buf, ExpressionShowMode::Latest);
	}

	Area get_area() const
	{
		return area;
	}

	bool get_is_implies() const
	{
		return is_imply;
	}

	const ExpressionRef get_before() const
	{
		return before;
	}

	const ExpressionRef get_after() const
	{
		return after;
	}
};

class RewritingRuleSet
{
private:
	ListBuilder<RewritingRule> rules;

public:
	bool rewrite(ExpressionRef* trg, RewritingType type);
	bool rewrite(ExpressionRef* trg, ExpressionRef goal, RewritingType type);
};