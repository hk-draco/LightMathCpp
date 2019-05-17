#pragma once
#include "RewritingInfo.h"
#include "Message.h"

using AssignTable = StackMap<class Variable*, class ExpressionRef>;

class Expression
{
public:
	virtual ExpressionRef clone_latest() const = 0;
	virtual ExpressionRef assign(const AssignTable& asg) const = 0;
	virtual bool unify(const ExpressionRef trg, AssignTable* asg) const = 0;
	virtual bool equals(const ExpressionRef trg) const = 0;
	virtual bool complete(Scope* scope, MessageManager* msg) = 0;
	//virtual bool contains(std::intptr_t ident) const = 0;
	//virtual bool assigned_equals(const AssignTable& asg, const ExpressionRef trg) const = 0;
	virtual CompareResult compare(const ExpressionRef trg) const = 0;
	virtual const ExpressionType* evaluate() const = 0;
};

struct ExpressionRewritingPair
{
	ExpressionRef expr; 
	RewritingInfo* rewriting;
};

class ExpressionRef : public PtrHolder<Expression>
{
private:
	void lookup_rewriters(FixedArray<struct ExpressionRewriter>* rewriters);
	bool derivate_whole(const ExpressionRef goal);
	bool merge_steps(ExpressionRef trg, struct ExpressionJunction junction);

public:
	ExpressionRef() {}
	ExpressionRef(std::nullptr_t) : PtrHolder(nullptr) {}
	ExpressionRef(Expression& ref) : PtrHolder(&ref) {}
	ExpressionRef(Expression* ptr) : PtrHolder(ptr) {}

	ExpressionRef* get_latest_step();
	void rewrite(ExpressionRef new_expr, RewritingInfo* rewriting);
	bool simplify();
	bool derivate(const ExpressionRef goal);
	bool transform(const ExpressionRef goal);
	bool unfold(const ExpressionRef goal, Scope* scope);
	bool extract_forall(const ExpressionRef goal);
	bool app_exists(const ExpressionRef goal);
	bool typing_unfold(const ExpressionRef goal);
	ExpressionDifference* difference(const ExpressionRef trg);
	ExpressionRewritingPair pop_step();
	bool merge_steps(ExpressionRef trg);
};

struct ExpressionDifference
{
	ExpressionRef* expr;
	const ExpressionRef trg_expr;
	ExpressionDifference* parent;

	ExpressionDifference() {}
	ExpressionDifference(std::nullptr_t) : expr(nullptr) {}
	ExpressionDifference(ExpressionRef* expr, const ExpressionRef trg_expr, ExpressionDifference* parent = nullptr)
		: expr(expr), trg_expr(trg_expr), parent(parent)
	{}
};