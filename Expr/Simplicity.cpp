#include "Simplicity.h"

SimplityCompare SimplicityManager::check_simplity_same_func(const ExpressionRef left, const ExpressionRef right, Area area)
{
	auto res = SimplityCompare::Equal;
	for (auto arg : Zip(left->get_args().range(), right->get_args().range())) {
		if (!arg.first->is_func() && !arg.second->is_func()) { // 両方が非関数
			continue;
		}
		if (!arg.first->is_func()) { // 左が非関数で右が関数
			msg->add_err("$Fよりも$Fの方が簡略的です", area);
			return SimplityCompare::NG;
		}
		if (!arg.second->is_func()) { // 左が関数で右が非関数
			res = SimplityCompare::OK;
			continue;
		}

		auto cmp = check_simplity(arg.first, arg.second, area);
		if (cmp == SimplityCompare::OK)
			res = SimplityCompare::OK;
		else if (cmp == SimplityCompare::NG)
			return SimplityCompare::NG;
	}
	return res;
}

SimplityCompare SimplicityManager::check_simplity(const ExpressionRef left, const ExpressionRef right, Area area)
{
	if (!right->is_func())
		return SimplityCompare::OK;

	auto left_func = left->get_obj();
	auto right_func = right->get_obj();

	if (left_func == right_func)
		return check_simplity_same_func(left, right, area);

	auto left_simp = std::find(simpls.begin(), simpls.end(), left_func);
	auto right_simp = std::find(simpls.begin(), simpls.end(), right_func);

	if (left_simp == simpls.end() && right_simp == simpls.end()) {
		simpls.push_back(right_func);
		simpls.push_back(left_func);

	} else if (left_simp == simpls.end()) {
		simpls.insert(right_simp, left_func);

	} else if (right_simp == simpls.end()) {
		simpls.insert(left_simp - 1, right_func);

	} else if (left_simp < right_simp) {
		msg->add_err("$Fよりも$Fの方が簡略的です", area);
		return SimplityCompare::NG;
	}

	return SimplityCompare::OK;
}