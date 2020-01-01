#pragma once

#include <string>
#include <unordered_map>
#include <cstdio>

/**
 * \brief preprocessor
 */
class Preprocessor
{
public:

	Preprocessor() {}

	/**
	 * \brief main preprocess function
	 */
	bool preprocess(FILE* in, FILE* out);

private:

	//scanner state
	enum class State
	{
		Start,
		Cmd,

		IncFileStart,
		IncFileScan,

		DefVarIdStart,
		DefVarId,
		DefVarValueStart,
		DefVarValue,

		Id,
	} pState;

	//token type
	enum class TokenType
	{
		Null,

		Eof,
		Inc,
		Def,
	};

	//token
	struct Token
	{
		TokenType   type;
		std::string arg;
		std::string arg2;

		Token() { type = TokenType::Null; }
		Token(TokenType t) { type = t; }
		Token(TokenType t, std::string a) { type = t; arg = a; }
		Token(TokenType t, std::string a, std::string a2) { type = t; arg = a; arg2 = a2; }
	} pToken;

	//scanner buffer
	std::string pBuffer;

	/**
	 * \brief check type of command
	 */
	TokenType parseCmd();
	/**
	 * \brief check if identificator isn't macro
	 */
	void 	  parseId(std::string& id);
	/**
	 * \brief get next preprocessor command
	 */
	Token     getToken();

	//input/output
	FILE* pIn;
	FILE* pOut;
};



