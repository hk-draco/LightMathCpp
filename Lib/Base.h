#pragma once

enum class CompareResult
{
	Equal, Less, Larger
};

template<typename T>
CompareResult compare(T left, T right)
{
	if (left == right)
		return CompareResult::Equal;
	return left < right ? CompareResult::Less : CompareResult::Larger;
}