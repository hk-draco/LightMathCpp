#pragma once
#include "RewritingInfo.h"
#include "Expression.h"
#include "Token.h"

class ExpressionStep : public Expression
{
	friend class StepIterator;

public:
	ExpressionRef steps[2];
	RewritingInfo* rewriting;

public:
	RewritingInfo* get_rewriting()
	{
		return rewriting;
	}

	ExpressionStep(ExpressionRef newer, ExpressionRef older, RewritingInfo* rewriting) : rewriting(rewriting)
	{
		steps[0] = newer;
		steps[1] = older;
	}

	ExpressionRef clone_latest() const override
	{
		return steps[0]->clone_latest();
	}
	
	bool unify(const ExpressionRef trg, AssignTable* asg) const
	{
		return steps[0]->unify(trg, asg);
	}

	ExpressionRef assign(const AssignTable& asg) const
	{
		return steps[0]->assign(asg);
	}

	bool equals(const ExpressionRef trg) const
	{
		return steps[0]->equals(trg);
	}

	bool complete(Scope* scope, MessageManager* msg) override
	{
		return get_latest_step()->complete(scope, msg);
	}

	CompareResult compare(const ExpressionRef trg) const override
	{
		return get_latest_step()->compare(trg);
	}

	const ExpressionRef get_latest_step() const
	{
		return steps[0];
	}

	ExpressionRef& get_latest_step() 
	{
		return steps[0];
	}

	ExpressionRef& get_older()
	{
		return steps[1];
	}

	IteratorRange<StepIterator> step_range()
	{
		return IteratorRange<StepIterator>(this, nullptr);
	}
};

class StepIterator
{
private:
	ExpressionStep* step;
	ExpressionRef expr;
	RewritingInfo* rewriting;

public:
	StepIterator(std::nullptr_t) : step(nullptr) {}
	StepIterator(ExpressionStep* step) : step(step), expr(step->steps[0]), rewriting(nullptr) {}

	void operator++()
	{
		rewriting = step->rewriting;
		auto next = step->steps[1];
		if (step = next.cast<ExpressionStep>())
			expr = step->steps[0];
		else 
			expr = next;
	}

	bool operator!=(const StepIterator& itr) const
	{
		return step != itr.step;
	}

	std::tuple<ExpressionRef, RewritingInfo*> operator*()
	{
		return { step, rewriting };
	}
};