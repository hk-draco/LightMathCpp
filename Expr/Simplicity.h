#pragma once
#include "Expression.h"

enum class SimplityCompare
{
	Equal, OK, NG
};

class SimplicityManager
{
private:
	std::vector<Object*> simpls;
	MessageManager* msg;

	SimplityCompare check_simplity_same_func(const ExpressionRef left, const ExpressionRef right, Area area);
	SimplityCompare check_simplity(const ExpressionRef left, const ExpressionRef right, Area area);

public:
	SimplicityManager(MessageManager* msg) : msg(msg) {}

	SimplityCompare check_simplity(const RewritingRule* rule)
	{
		return check_simplity(rule->get_before(), rule->get_after(), rule->get_area());
	}
};