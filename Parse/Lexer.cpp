#include "token.h"

float parse_decimal(StringView str)
{
	float value = 0;
	int decimal_place = 0;
	value = 0;
	for (auto c : str) {
		int number = c - '0';
		// êÆêîïî
		if (decimal_place == 0) {
			if (number < 0) {
				decimal_place = 1;
			} else {
				value = value * 10 + number;
			}
		// è≠êîïî
		} else {
			float fnumber = number;
			for (int j = 0; j < decimal_place; j++)
				fnumber *= 0.1;
			value += fnumber;
			decimal_place++;
		}
	}
	return value;
}

int parse_int(StringView str)
{
	int value = 0;
	for(auto c : str)
		value = value * 10 + (c - '0');
	return value;
}

void app_value(Token* token)
{
	switch (token->ann) {
	case Annotator::Symbol:
	case Annotator::Operator:
	case Annotator::Identifer:
		token->ident = 0;
		memcpy(&token->ident, token->str.data(), sizeof(char) * std::min(token->str.size(), 8));
		break;
	case Annotator::Integer:
		token->int_value = parse_int(token->str);
		break;
	case Annotator::Decimal:
		token->float_value = parse_decimal(token->str);
		break;
	}
}

class AnalysisState
{
private:
	List<Token> tokens;
	Location loc;
	const char* src;
	const char* curr;
	bool no_end;

public:
	AnalysisState(const char* src)
		: src(src), curr(src), loc(Location(1, 1)), no_end(false) {}

	void push_token(Annotator anno)
	{
		end_token();
		tokens.push_back(std::make_unique<Token>(curr, anno, loc));
		no_end = true;
	}

	void push_token(Annotator anno, int offset)
	{
		end_token();
		tokens.push_back(std::make_unique<Token>(curr + offset, anno, loc));
		no_end = true;
	}

	void end_token()
	{
		if (no_end) {
			int len = curr - tokens.back()->str.data();
			tokens.back()->str.resize(len);
			app_value(tokens.back());
			no_end = false;
		}
	}

	void next_char()
	{
		curr++;
		loc.column++;
	}

	void next_line()
	{
		loc.column = 0;
		loc.row++;
	}

	Annotator curr_anno()
	{
		return no_end ? tokens.back()->ann : Annotator::Empty;
	}

	void change_curr_anno(Annotator anno)
	{
		tokens.back()->ann = anno;
	}

	char curr_char() const
	{
		return *curr;
	}

	IteratableEntity<Token> move_entity()
	{
		return tokens.move_entity();
	}
};

Iteratable<Token> lexical_analyze(const char* str)
{
	AnalysisState state(str);

	while (true) {
		switch (state.curr_char()) {
		case '{': case '}': case '(': case ')': case '[': case ']': case ';': case ',': case ':':
			state.push_token(Annotator::Symbol);
			break;

		case '+': case '-': case '*': case '\\': case '<': case '>': case '/':
		case '=': case '%': case '^': case '&': case '|': case '!': case '~':
			switch (state.curr_anno()) {
			case Annotator::String:
			case Annotator::Char:
			case Annotator::Operator:
				break;
			default:
				state.push_token(Annotator::Operator);
			}
			break;

		case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
			switch (state.curr_anno()) {
			case Annotator::String:
			case Annotator::Char:
			case Annotator::Identifer:
			case Annotator::Integer:
			case Annotator::Decimal:
				break;
			default:
				state.push_token(Annotator::Integer);
			}
			break;

		case 'a':case 'b':case 'c':case 'd':case 'e':case 'f':case 'g':
		case 'h':case 'i':case 'j':case 'k':case 'l':case 'n':case 'm':
		case 'o':case 'p':case 'q':case 'r':case 's':case 't':case 'u':
		case 'v':case 'w':case 'x':case 'y':case 'z':
		case 'A':case 'B':case 'C':case 'D':case 'E':case 'F':case 'G':
		case 'H':case 'I':case 'J':case 'K':case 'L':case 'N':case 'M':
		case 'O':case 'P':case 'Q':case 'R':case 'S':case 'T':case 'U':
		case 'V':case 'W':case 'X':case 'Y':case 'Z':case '_':case '$':
			switch (state.curr_anno()) {
			case Annotator::String:
			case Annotator::Char:
			case Annotator::Identifer:
				break;
			default:
				state.push_token(Annotator::Identifer);
			}
			break;

		case '.':
			switch (state.curr_anno()) {
			case Annotator::String:
			case Annotator::Char:
				break;
			case Annotator::Integer:
				state.change_curr_anno(Annotator::Decimal);
				break;
			default:
				state.push_token(Annotator::Symbol);
			}
			break;

		case '"':
			switch (state.curr_anno()) {
			case Annotator::String:
				state.end_token();
				break;
			default:
				state.push_token(Annotator::String, 1);
			}
			break;

		case '\'':
			switch (state.curr_anno()) {
			case Annotator::Char:
				state.end_token();
				break;
			default:
				state.push_token(Annotator::Char, 1);
			}
			break;

		case '#':
			state.end_token();
			while (state.curr_char() != '\n')
				state.next_char();
			state.next_line();
			break;

		case '\n':
			state.next_line();
			switch (state.curr_anno()) {
			case Annotator::EndLine:
				break;
			default:
				state.push_token(Annotator::EndLine);
			}
			break;
		
		case ' ': case '\t': case '\r':  case '\v': case '\f':
			state.end_token();
			break;

		case '\0':
			state.end_token();
			goto END_PARSE;

		default:
			switch (state.curr_anno()) {
			case Annotator::String:
			case Annotator::Char:
				break;
			default:
				state.push_token(Annotator::Error);
			}
			break;
		}
		state.next_char();
	}

END_PARSE:
	return state.move_entity();
}