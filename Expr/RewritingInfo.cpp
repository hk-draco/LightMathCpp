#include "RewritingInfo.h"

RewritingInfo::RewritingInfo(RewritingType type, RewritingRule* rule, const AssignTable& _asg)
	: type(type), rule(rule), asg(_asg.size())
{
	auto scope = rule->get_scope();
	for (auto[var, idx] : scope->get_args().range().indexed())
		asg[idx] = Assignation(var, _asg[idx]);

	for (auto[var, idx] : scope->get_locals().range().indexed())
		asg[idx + scope->get_arg_num()] = Assignation(var, _asg[idx]);
}