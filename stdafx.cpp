// stdafx.cpp : 標準インクルード [!output PROJECT_NAME].pch のみを
// LightMath.pch はプリコンパイル済みヘッダーになります
// stdafx.obj にはプリコンパイル済み型情報が含まれます。

#include "stdafx.h"

// TODO: このファイルではなく、STDAFX.H で必要な
// 追加ヘッダーを参照してください。


/*
[文字列]
1.字句解析
[トークン列]
2.構文解析
[抽象構文木]
3.意味解析
[式、証明]





*/



/*
int op_preced(uint64 ident)
{
	switch (ident) {
	case CTS("!"): case CTS("#"):
		return 11;
	case CTS("^"):
		return 10;
	case CTS("*"): case CTS("/"): case CTS("%"):
		return 9;
	case CTS("+"): case CTS("-"):
		return 8;
	case CTS("="): case CTS("<"): case CTS(">"): case CTS("<="): case CTS(">="): case CTS("in"):
		return 7;
	case CTS("~"):
		return 6;
	case CTS("&"): case CTS("|"):
		return 5;
	case CTS("->"): case CTS("<->"):
		return 4;
	case CTS(":"):
		return 3;
	case CTS("=>"): case CTS("<=>"):
		return 2;
	case CTS("forall"): case CTS("exists"):
		return 1;
	}
	return 0;
}

bool op_left_assoc(uint64 ident)
{
	switch (ident) {
	// 左結合性
	case CTS("*"): case CTS("/"): case CTS("%"): case CTS("+"): case CTS("-"): case CTS("^"):
	case CTS("="): case CTS("<"): case CTS(">"): case CTS("<="): case CTS(">="): case CTS("in"):
	case CTS("~"): case CTS("&"): case CTS("|"): case CTS("->"): case CTS("<->"):
	case CTS(":"): case CTS("=>"): case CTS("<=>"):
		return true;
	// 右結合性
	case CTS("!"): case CTS("#"): case CTS("forall"): case CTS("exists"):
		return false;
	}
	return false;
}

unsigned int op_arg_count(uint64 ident)
{
	switch (ident) {
	case CTS("*"): case CTS("/"): case CTS("%"): case CTS("+"): case CTS("-"): case CTS("^"):
	case CTS("="): case CTS("<"): case CTS(">"): case CTS("<="): case CTS(">="): case CTS("in"):
	case CTS("~"): case CTS("&"): case CTS("|"): case CTS("->"): case CTS("<->"):
	case CTS(":"): case CTS("=>"): case CTS("<=>"):
		return 2;
	case CTS("!"): case CTS("#"): case CTS("forall"): case CTS("exists"):
		return 1;
	}
	return 0;
}

std::string Expression::to_str() const
{
	int ope_num = op_arg_count(this->name->ident);
	switch (ope_num) {
	case 1:
		return std::string("(") + name->to_str() + args[0]->to_str() + ")";
	case 2:
		return std::string("(") + args[0]->to_str() + name->to_str() + args[1]->to_str() + ")";
	default: {
		std::string buf = name->to_str();
		if (this->args.size() == 0)
			break;
		buf += '(';
		for (int i = 0; i < this->args.size(); i++) {
			buf += args[i]->to_str();
			if (i != this->args.size() - 1)
				buf += ',';
		}
		buf += ')';
		return buf;
	}
	}
}




// forall eps, x: posReal, f: R->R, n: N;
// exists n_0[eps]: N
VariableDeclaration parse_var_dec(ParseSection& data, Quantifier qtf)
{
	List<VariableTyping> typings;
	List<VariableIdent> idents;
	do {
		auto ident = data.read_ident();
		if (!ident)
			data.make_error_curr("識別子が必要です");

		UtilList<Token> ref_vars;
		if (data.maybe_token(CTS("["))) {
			if (qtf == Quantifier::Exists)
				data.add_error("全称化には必要ありません");
			ref_vars = parse_idents(data);
			data.expect_token(CTS("]"), CTS("|"));
		} else if (qtf == Quantifier::Exists) {
			data.add_error("存在化には、それをスコーレム化した際にマッピングする全称化された変数のリストが必要です");
		}

		idents.push_back(std::make_unique<VariableIdent>(ident, ref_vars));

		if (data.maybe_token(CTS(":"))) {
			auto type = parse_expression(data);
			if (!type)
				break;
			typings.push_back(std::make_unique<VariableTyping>(std::move(idents), std::move(type)));
			idents = List<VariableIdent>();
		}
	} while (data.read_token(CTS(",")));

	if (!idents.empty())
		data->msg.add_err("型指定が必要です", idents.back()->ident->area());

	return VariableDeclaration(std::move(typings), qtf);
}
*/