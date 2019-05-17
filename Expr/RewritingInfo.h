#pragma once
#include "Expression.h"
#include "RewritingRule.h"
#include "Scope.h"
#include "Variable.h"

enum class RewritingType
{
	User, Auto, Admitted, ErrorAdmitted, CommuteAC, Unfold
};

struct Assignation
{
	Variable* var;
	const ExpressionRef expr;

	Assignation(class Variable* var, const ExpressionRef expr) : var(var), expr(expr) {}
};

struct RewritingInfo
{
	RewritingType type;
	union
	{
		class RewritingRule* rule;
		Object* obj;
	};
	HeapArray<Assignation> asg;

	RewritingInfo(RewritingType type) : type(type) {}
	RewritingInfo(RewritingType type, RewritingRule* rule, const AssignTable& asg);
	RewritingInfo(RewritingType type, Object* obj, const AssignTable& asg);
};