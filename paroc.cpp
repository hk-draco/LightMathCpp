#include "program.h"
#include "entity.h"
#include "rewrite.h"

Scope* scope;
MessageManager* msg;

//======================================================================
// 交換律と結合律
//======================================================================


//======================================================================
// 型の比較
//======================================================================


bool check_arg_types(Iteratable<Expression> arg_values, Iteratable<Variable> arg_decs)
{
	for (auto[value, dec] : Zip(arg_values.range(), arg_decs.range())) {
		if (!value) {
			msg->add_err("引数が足りません", value->get_area());
			return false;
		}
		if (!dec) {
			msg->add_err("引数が多すぎます", value->get_area());
			return false;
		}
		if (!evaluatable(value, dec->evaluate()))
			msg->add_err("$Tに評価できません", value->get_area())->add(dec->evaluate()->to_str());
	}
	return true;
}

// 引数の型を確認し、戻り値の型を返す
const Expression* eval_func_call(const Expression* call, const Object* obj)
{
	if(!check_arg_types(call->get_args(), obj->get_scope()->get_args()))
		return nullptr;
	return obj->evaluate();
}

//======================================================================
// 式の生成
//======================================================================

//======================================================================
// 通常のステートメント
//======================================================================
Scope create_scope(VariableDeclaration* arg_dec)
{
	Scope scope;
	for (auto typing : arg_dec->typings.range()) {
		auto var = new EntityList(Variable(typing->ident->str, typing->type, arg_dec->qtf));
		if (make_type_expr(var->get_type(), scope))
			scope.add_arg(std::move(var));
	}

	return scope;
}

void load_var_dec(VariableDeclaration* arg_dec, Scope* scope)
{
	for (auto typing : arg_dec->typings.range()) {
		scope->add_local(new EntityList(Variable(typing->ident->str, typing->type, arg_dec->qtf)));
	}
}

void run_statement(Statement* stm, Scope* scope)
{
	switch (stm->type) {
	case StatementType::Variable:
		load_var_dec(stm->var_dec, scope);
		break;
	}
}

//======================================================================
// 証明のステートメント
//======================================================================


void run_proof_statement(Expression* trg, const Expression* where, Statement* stm, Scope* scope)
{
	const WhereInserter wit(stm->arg, where);
	make_expr(wit.inserted, *scope);

	switch (stm->type) {
	case StatementType::Step:
		if (!rewrite_by_equiv(trg, wit.inserted, 0))
			msg->add_err("導出できません", stm->area);
		break;

	case StatementType::Impl:
		if (!rewrite_by_derivating(trg, wit.inserted, 0))
			msg->add_err("導出できません", stm->area);
		break;

	case StatementType::Where:
		
	}
}

//======================================================================
// 取得
//======================================================================

Expression* make_proof(Expression* proof, Iteratable<Statement> stms, Scope* scope)
{
	Expression* trg = proof->get_args().front();
	for (auto stm : stms.range()) {
		switch (stm->type) {
		case StatementType::Target:
			switch (stm->keyword->ident) {
			case CTS("all"):
				trg = proof;
				break;
			case CTS("left"):
				trg = proof->get_args().front();
				break;
			case CTS("right"):
				trg = proof->get_args().next().front();
				break;
			default:
				msg->add_err("$Tは使用できません", stm->keyword->area())->add(stm->keyword->str);
			}
			break;
		default:
			run_proof_statement(trg, nullptr, stm, scope);
		}
	}

#ifdef _DEBUG
	printf("\n===proof===\n");
	std::string str;
	proof->write(&str, ExpressionShowMode::Step);
	printf(str.data());
	printf("\n\n");
#endif

	return std::move(proof);
}


//======================================================================
// 命題・項書換え規則・定義
//======================================================================

//======================================================================
// 宣言
//======================================================================

struct DeclarationInfo
{
	bool need_proof;

	DeclarationInfo() : need_proof(false) {}
	DeclarationInfo(bool need_proof) : need_proof(need_proof) {}
};

enum class ProofType
{
	RightImplies, LeftImplies, 
};

void load_block(GlobalEntity* ent, LabeledBlock* block, DeclarationInfo info)
{
	auto rule = (const RewritingRule*)ent;
	bool res = true;
	switch (block->label->type) {
	case StatementType::LabelProof:
		if (info.need_proof) {
			ExpressionRef prop;
			Object* ope;
			auto keyword = block->label->keyword;
			switch (List<Expression> args; keyword ? keyword->ident : 0) {
			case CTS("->"):
				args.push_back(rule->get_before()->clone());
				args.push_back(rule->get_after()->clone());
				prop = std::make_unique<Expression>(keyword, std::move(args));
				goto IMPLIES_PROOF;
			case CTS("<-"):
				args.push_back(rule->get_after()->clone());
				args.push_back(rule->get_before()->clone());
				prop = std::make_unique<Expression>(keyword, std::move(args));
			IMPLIES_PROOF:
				if (!(ope = glb->get_obj("implies"_st)))
					msg->add_err("impliesが定義されていません", block->label->area);
				if (!rule->get_is_implies())
					msg->add_err("命題が同値形ではありません", block->label->keyword->area());
				break;
			default:
				args.push_back(rule->get_before()->clone());
				args.push_back(rule->get_after()->clone());
				if(!(ope = glb->get_obj("equals"_st)))
					msg->add_err("equalsが定義されていません", block->label->area);
				prop = std::make_unique<Expression>(ope->get_name(), std::move(args));
			}
			if (ope) {
				prop->make_func(ope->evaluate(), (std::intptr_t)ope, ope->get_prop());
				auto proof = make_proof(std::move(prop), block->stms.to_iteratable(), ent->get_scope());
				res = ent->set_proof(std::move(proof), block->label->area);
			}
		} else {
			msg->add_err("証明は不要です", block->label->area);
		}
		break;

	case StatementType::LabelName:
		res = ent->set_name(get_str_from_arg(block), get_str_from_stms(block));
		break;
	case StatementType::LabelLatex:
		break;
	}
	if (!res)
		msg->add_err("このラベルは既に使用されています", block->label->area);
}

std::tuple<GlobalEntity*, DeclarationInfo> create_entity(const Declaration* dec)
{
	if (dec->name && glb->exists_obj(dec->name->str)) {
		msg->add_err("$Iは既に定義されています", dec->name->area())->add(dec->name->str);
		return { nullptr, DeclarationInfo() };
	}

	switch (dec->type) {
	case DeclarationType::UndefinedTerm:
		return { add_obj(dec, false), DeclarationInfo() };
	case DeclarationType::Definition:
		return { add_obj(dec, true), DeclarationInfo() };
	case DeclarationType::Axiom:
		return { add_prop(dec), DeclarationInfo() };
	case DeclarationType::Theorim:
		return { add_prop(dec), DeclarationInfo(true) };
	case DeclarationType::Type:
		return { add_type(dec), DeclarationInfo() };
	}
}

void load_declaration(const Declaration* dec)
{
	auto[ent, info] = create_entity(dec);
	if (ent) {
		for (auto block : dec->blocks.range())
			load_block(ent, block, info);
	}
}

GlobalScope analyze_program(MessageManager* msg, const ProgramEntity& prg)
{
	GlobalScope glb(msg);
	::glb = &glb;
	::msg = msg;

	for (auto dec : prg.decs.range())
		load_declaration(dec);
	return glb;
}

char* convert_com_str(std::string&& str)
{
	auto ptr = (char*)::CoTaskMemAlloc(str.size());
	str.copy(ptr, str.size());
	return ptr;
}

void make_expression(TypedExpressionRef src, Scope* scope, MessageManager* msg)
{
	::glb = glb;
	::msg = msg;
	auto scope = create_scope(src->var_dec);
	src->has_error = !make_expr(src->expr.get(), scope);
}

/*
extern "C" __declspec(dllexport) char* compile(const char** nodes, const int* node_ids, int node_num)
{
	return convert_com_str(compile_impl(nodes, node_ids, node_num));
}

extern "C" __declspec(dllexport) char* manipulate(const char** nodes, const int* node_ids, int node_num, char* input)
{
	return convert_com_str(manipulate_impl(nodes, node_ids, node_num, input));
}
*/