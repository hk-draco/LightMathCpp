#pragma once
#include "Token.h"
#include "RewritingInfo.h"


struct FunctionProperty
{
	bool is_commutative;
	bool is_associative;

	FunctionProperty() : is_commutative(false), is_associative(false) {}
};

enum class ExpressionType
{
	None,
	Step,   // [最初の式, ..., 最新の式]
	Lambda, // [束縛変数, 本体]
	Constant, Variable,		 // [(空)]
	Function, CFunction, ACFunction, // [引数1, ..., 引数n]
	Type,         // [引数1, ..., 引数n]
	FunctionType, // [[引数1の型, ..., 引数nの型], [戻り値の型]]
	TypeArgs
};

struct ACRuleMaking
{
	ExpressionRef before_bind_var;
	ExpressionRef after_bind_var;
};

enum class ExpressionShowMode
{
	Latest, Step
};

class Expression
{
private:
	ExpressionType type;
	RewritingInfo* rewriting;
	const ExpressionRef eval_type;

	union
	{
		std::intptr_t ident;
		Token* head;
		Object* obj;
		Variable* var;
		ExpressionRef parent_step;
		int bind_var;
	};


	static void get_exprs(StringView func_name, RefArray<Expression> args, FixedArray<Expression>* buf);

	void make_func(const ExpressionRef eval_type, std::intptr_t ident, ExpressionType type)
	{
		this->type = type;
		this->eval_type = eval_type;
		this->ident = ident;
	}

	template<typename TFunc>
	ExpressionRef find_unique_arg(TFunc cond)
	{
		ExpressionRef res = nullptr;
		for (auto arg : Range(args)) {
			if (cond(arg)) {
				if (res)
					return nullptr;
				res = arg;
			}
		}
		return res;
	}

	template<typename TFunc>
	ExpressionRef find_arg(TFunc cond)
	{
		for (auto arg : Range(args)) {
			if (cond(arg))
				return arg;
		}
		return nullptr;
	}

	Expression() {}
	Expression(Token* head, int arg_size) 
		: head(head), args(arg_size), type(ExpressionType::None) {}
	Expression(ExpressionType type, Token* head, std::intptr_t ident, RefArray<Expression> args)
		: type(type), head(head), ident(ident), args(args) {}

	bool unify_lambda_args(AssignTable* asg, const ExpressionRef trg) const;

	void make_step(ExpressionRef new_expr);

public:
	void marge_steps(ExpressionRef trg);
	static ExpressionRef* get_latest_step(ExpressionRef* trg);

	static bool commute_ac(ExpressionRef* trg, ExpressionRef rewrote_arg);
	static void rewrite(ExpressionRef* trg, ExpressionRef new_expr, RewritingInfo* info = nullptr);
	static void marge_steps(ExpressionRef* trg2, ExpressionRef steps);

	static ExpressionRef create(Token* head, int arg_size = 0)
	{
		return new(arg_size) Expression(head, arg_size);
	}

	void set_arg(int idx, ExpressionRef arg)
	{
		args[idx] = arg;
	}

	void complete(const GlobalScope& glb, const Scope& scope);


	//Expression(List<Expression>&& args) : args(std::move(args)), parent_step(nullptr) {}

	// bind_varを使用したときtrueを返す
	static bool make_ac_rule(ExpressionRef before, ExpressionRef* after_ent, int bind_var);


	// クローンを作成する。
	ExpressionRef clone_latest() const;


	// 束縛変数を書き換えた式を返す。
	// ラムダ式の場合のみ使用可能。
	ExpressionRef rename_bind_var(ExpressionRef new_var);


	//======================================================================
	// 置き換え
	//======================================================================

	// 変数[ident]を式[expr]のクローンに一度だけ置き換える。
	// 置き換える対象がないときや置き換える対象が2つ以上あるときは置き換えずにnullptrを返す。置き換えに成功したときは、置き換えた後の式を返す。
	// 例) this=a*(x+y)^2, ident=x, expr=f(x) => this=a*(f(x)+y)^2, return=thisのf(x)のポインター
	ExpressionRef replace_once(std::intptr_t ident, const ExpressionRef expr);

	void replace(std::intptr_t ident, ExpressionRef expr);

	//======================================================================
	// 検索
	//======================================================================

	// 式[trg]の内部の式[trg_child]と同じ位置のこの式の内部の式を返す。
	// 同じ位置の式がstepであるときは、step中の最も新しい式ではなく、step自体を返す。
	// 式[trg_child]がstepであるときは、そのstepの中の最も新しい式を指定したのと同じ結果を返す。
	// この式と式[trg]は単一化可能である必要があり、式[trg]は式[trg_child]を含んでいる必要がある。
	// この前提条件が満たされていないときは、nullptrを返す。
	// 例) this=x+y>f(z, w), trg=a+b>f(c, d), trg_child=c => return=thisのzのポインター
	ExpressionRef find(const ExpressionRef trg, const ExpressionRef trg_child);

	struct ExpressionDifference find_diff(const ExpressionRef trg) const;

	//======================================================================
	// Step関係
	//======================================================================

	// 最も新しいStepを返す。
	ExpressionRef get_latest_step();

	// 書き換える。
	void rewrite(ExpressionRef new_expr, RewritingInfo rewriting);

	// ステップを逆にする。
	void reverse_steps();

	// ステップを統合する。
	// [A, B, C, D] + [D, P, Q] => [A, B, P, Q]
	void marge_steps(ExpressionRef trg);

	std::tuple<bool, std::vector<ExpressionRef>> has_step(ExpressionRef trg);

	void cut_next()
	{
		ZeroMemory(this, sizeof(std::intptr_t));
	}

	//======================================================================
	// Getter
	//======================================================================

	const ExpressionRef eval() const
	{
		return eval_type;
	}

	class Object* get_obj() const
	{
		assert(type == ExpressionType::Function || type == ExpressionType::CFunction || type == ExpressionType::ACFunction);
		return (class Object*)ident;
	}

	class Type* get_type() const
	{
		assert(type == ExpressionType::Type);
		return (class Type*)ident;
	}

	Token* get_head() const
	{
		return head;
	}

	Token* get_func_name() const
	{
		return head->ope.get_func_name();
	}

	Area get_area() const
	{
		if (!args)
			return head->area();
		return Area(head->begin(), args.back()->get_area().end);
	}

	RefArray<ExpressionRef> get_args() const
	{
		return args;
	}

	std::intptr_t get_ident() const
	{
		return ident;
	}

	bool is_var() const
	{
		return type == ExpressionType::Variable;
	}

	bool is_func() const
	{
		return type == ExpressionType::Function || type == ExpressionType::CFunction || type == ExpressionType::ACFunction;
	}

	bool is_ac_func() const
	{
		return type == ExpressionType::ACFunction;
	}

	void write(std::string* buf, ExpressionShowMode mode) const;

	std::string to_str() const
	{
		std::string str;
		write(&str, ExpressionShowMode::Latest);
		return str;
	}
};
