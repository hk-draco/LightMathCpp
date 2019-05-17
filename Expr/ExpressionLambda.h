#pragma once
#include "ExpressionFunction.h"
#include "ExpressionType.h"
#include "Declaration.h"
#include "Token.h"

struct TypedVariable
{
	Token* ident;
	ExpressionType* type;
};

class ExpressionLambda : public Expression
{
private:
	Scope scope;
	ExpressionRef body;
	ExpressionTypeFunction* type;
	MemberArray<TypedVariable> args;

public:
	ExpressionLambda(const Scope& scope, ExpressionRef body, ExpressionTypeFunction* type, int arg_num) 
		: scope(scope), body(body), type(type), args(arg_num) 
	{}

	ExpressionLambda(VariableDeclaration* var_dec, ExpressionRef body)
	{
		for (auto[typing, idx] : var_dec->typings.range().indexed())
			args[idx] = { typing->ident, typing->type };
	}
	
	ExpressionRef clone_latest() const override;
	bool unify(const ExpressionRef trg, AssignTable* asg) const override;
	bool unify_lambda_args(AssignTable* asg, const ExpressionRef trg) const;
	ExpressionRef assign(const AssignTable& asg) const override;

	ExpressionRef clone_latest() const override;

	ExpressionRef assign(const AssignTable& asg) const override
	{
		return new(args.size()) ExpressionLambda(scope, body->assign(asg), type, args.size());
	}

	bool complete(Scope* parent, MessageManager* msg) override
	{
		scope = Scope(parent);
		type = new(args.size()) ExpressionTypeFunction(body->evaluate(), args.size());
		for (auto[arg, idx] : args.range().indexed()) {
			arg.type->complete(&scope, msg);
			scope.add_var(Variable(*arg.ident, arg.type, Quantifier::ForAll));
			type->set_arg_type(idx, arg.type);
		}

		body->complete(&scope, msg);
	}

	bool unify(const ExpressionRef trg, AssignTable* asg) const override
	{
		// (x->a+x).unify(y->b+y) = {a: b}
		auto lambda = trg.cast<ExpressionLambda>();
		auto res = body->unify(lambda->body, asg);
		for (auto[var, trg_var] : Zip(scope.get_vars().range(), lambda->scope.get_vars().range())) {
			auto& mat = (*asg)[var];
			if (!mat)
				return false;
			auto var = mat.cast<ExpressionAtom>();
			if (!var)
				return false;
			if (var->get_var() != trg_var)
				return false;
			mat = nullptr;
		}
		return true;
	}

	bool equals(const ExpressionRef trg) const override
	{
		auto lambda = trg.cast<ExpressionLambda>();
		if (type->equals(lambda->type))
			return true;
		
	}

	CompareResult compare(const ExpressionRef trg) const override
	{
		auto lambda = trg.cast<ExpressionLambda>();
		if (!lambda)
			return CompareResult::Larger;
		if (auto res = ::compare(args.size(), lambda->args.size()); res != CompareResult::Equal)
			return res;
		for (auto[arg, trg_arg] : Zip(args.range(), lambda->args.range())) {
			if (auto res = arg.type->compare(trg_arg.type); res != CompareResult::Equal)
				return res;
		}
		return body->compare(lambda->body);
	}

	const ExpressionType* evaluate() const override
	{
		return type;
	}
};

class ExpressionLambdaCapture : public ExpressionFunction
{
private:
	Variable* func;
	MemberArray<ExpressionAtom*> vars;

public:
	void unify()
};