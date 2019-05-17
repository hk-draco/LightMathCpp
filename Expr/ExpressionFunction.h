#pragma once
#include "ExpressionAtom.h"

class ExpressionFunction : public ExpressionHeadHolder
{
protected:
	MemberArray<ExpressionRef> args;

	template<typename TFunc>
	ExpressionRef apply_args(TFunc func) const
	{
		auto expr = new(args.size()) ExpressionFunction(obj, args.size());
		for (auto[arg, idx] : args.range().indexed())
			expr->args[idx] = func(arg);
		return expr;
	}

	template<typename TFunc>
	bool for_each_args(const ExpressionRef trg, TFunc func) const
	{
		auto func_trg = trg.cast<ExpressionFunction>();
		if (!func_trg || obj != func_trg->obj)
			return false;
		for (auto[arg, trg_arg] : Zip(args.range(), func_trg->args.range())) {
			if (!func(arg, trg_arg))
				return false;
		}
		return true;
	}

	const ExpressionType* type;

	const ExpressionTypeFunction* complete_func(Scope* scope, MessageManager* msg);

public:
	ExpressionFunction(Token* head, int arg_num) : ExpressionHeadHolder(head), args(arg_num) {}
	ExpressionFunction(Object* obj, int arg_num) : ExpressionHeadHolder(obj), args(arg_num) {}
	ExpressionFunction(Object* obj, RefArray<ExpressionRef> args) : ExpressionHeadHolder(obj), args(args) {}

	ExpressionRef clone_latest() const override;
	bool equals(const ExpressionRef trg) const override;
	bool unify(const ExpressionRef trg, AssignTable* asg) const override;
	ExpressionRef assign(const AssignTable& asg) const override;
	CompareResult compare(const ExpressionRef trg) const override;
	bool complete(Scope* scope, MessageManager* msg) override;

	const ExpressionType* evaluate() const override
	{
		return type;
	}

	//bool replace(Variable* var, ExpressionRef expr) override;
	//bool contains(std::intptr_t ident) const override;
	//bool assigned_equals(const AssignTable& asg, const ExpressionRef trg) const override;

	const RefArray<ExpressionRef> get_args() const
	{
		return args;
	}
};

class ExpressionFunctionC : public ExpressionFunction
{
private:
	ExpressionFunctionC(Object* obj, int arg_num) : ExpressionFunction(obj, arg_num) {}

	bool unify(const ExpressionRef trg, AssignTable* asg) const override;
};

class ExpressionFunctionAC : public ExpressionFunction
{
private:
	ExpressionFunctionAC(Object* obj, int arg_num) : ExpressionFunction(obj, arg_num) {}
	ExpressionFunctionAC(Object* obj, RefArray<ExpressionRef> args) : ExpressionFunction(obj, args) {}

	bool complete(Scope* scope, MessageManager* msg) override;
	bool unify(const ExpressionRef trg, AssignTable* asg) const override;
	ExpressionRef assign(const AssignTable& asg) const override;
};