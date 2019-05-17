#pragma once
#include "Expression.h"
#include "Type.h"

class ExpressionTyping : public Expression
{
private:
	ExpressionRef expr;
	Expression* type;

public:
	ExpressionRef clone_latest() const = 0
	{

	}

	ExpressionRef assign(const AssignTable& asg) const = 0
	{

	}

	bool unify(const ExpressionRef trg, AssignTable* asg) const = 0
	{

	}

	bool equals(const ExpressionRef trg) const = 0
	{
		if (auto typing = trg.cast<ExpressionTyping>())
			return typing->expr->equals(expr) && typing->type->equals(type);
		return false;
	}
};

class ExpressionType : public Expression
{
public:
	virtual bool complete(Scope* scope, MessageManager* msg, const ExpressionRef trg) = 0;
};

class ExpressionTypeAtom : public ExpressionType
{
private:
	Type* type;
	MemberArray<ExpressionRef> args;

public:
	ExpressionTypeAtom(Type* type, int arg_num) : args(arg_num) {}

	void set_arg(int idx, ExpressionRef arg)
	{
		args[idx] = arg;
	}

	bool complete(Scope* scope, MessageManager* msg, const ExpressionRef trg) override
	{
		return type->cast(trg, args);
	}
};

class ExpressionTypeFunction : public ExpressionType
{
private:
	Scope scope;
	ExpressionType* ret_type;
	MemberArray<ExpressionType*> arg_types;

public:
	ExpressionTypeFunction(ExpressionType* ret_type, int arg_num) : arg_types(arg_num), ret_type(ret_type) {}

	void set_arg_type(int idx, ExpressionType* type)
	{
		arg_types[idx] = type;
	}

	bool complete(Scope* scope, MessageManager* msg, const ExpressionRef trg) override
	{
		// Int(T) -> T
		if (auto type = dynamic_cast<const ExpressionTypeFunction*>(trg->evaluate())) {
			
		}
	}

	const RefArray<ExpressionType*> get_arg_type() const
	{
		return arg_types;
	}

	const ExpressionType* get_return_type(RefArray<ExpressionType> args) const
	{
		AssignTable asg;
		for(auto[arg_type, arg] : Zip(arg_types.range(), args.range()))
			arg_type->unify(arg, &asg)
		return ret_type.assign(asg);
	}
};