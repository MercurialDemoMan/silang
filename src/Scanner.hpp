#pragma once

#include "types.hpp"

#include <string>
#include <stack>
#include <cstdio>

/**
 * \brief scanner class
 */
class Scanner
{
public:

	/**
	 * \brief token type
	 */
	enum class TokenType
	{
		Null,
		Acc,
		Ret,

		Eol, 
		Eof, 
		LeftCurlyBracket, 
		RightCurlyBracket,

		LeftBracket, 
		RightBracket, 
		Dot,
		Colon, 
		Comma, 
		SemiColon,

		Assign, 
		Less, 
		More, 
		LessEqu, 
		MoreEqu, 
		Equ,
		NonEqu, 
		Plus, 
		Minus, 
		Mul, 
		Div, 

		Id, 
		Int,
		Float, 
		String, 
		Keyword,
	};

	/**
	 * \brief keyword type
	 */
	enum class KeywordType
	{
		Null,

		Func, 
		Else, 
		If, 
		Return, 
		While, 
		For,
		Byte,
		Int, 
		Float,
		Pack,
		Void,
	};

	/**
	 * \brief token structure
	 */
	struct Token
	{
		TokenType type;

		struct TokenAttribute
		{
			std::string litString;
			i64			litInt;
			f64		    litFloat;
			KeywordType keyword;
		} attribute;

		Token() 		   { type = TokenType::Null; }
		Token(TokenType t) { type = t; }

		void print();
	};

private:

	/**
	 * \brief all scanner states
	 */
	enum class State
	{
		Start, 
		Id,
		Number,
		NumberHex,
		NumberOct,
		NumberBin,
		NumberFloatingPoint,
		NumberFraction,
		NumberExp,
		NumberExpSign,
		NumberExpTail,
		String,
		StringEscape,
		StringEscapeHex1,
		StringEscapeHex2,
		NonEqu,
		Less,
		More,
		Assign,
		StartComment,
		Comment,
		MulComment,
		EndMulComment,
	} pState;

	/**
	 * \brief helper enum for number bases
	 */
	enum class NumberBase
	{
		Hex, Dec, Oct, Bin
	};

	FILE*       pSource; /* input source file */
	std::string pBuffer; /* buffer for scanning numbers and identificators */

	/**
	 * \brief check if identificator is not keyword
	 */
	void parseId(std::string& id, Token& t);
	/**
	 * \brief convert number in string form to real number
	 */
	void parseInt(std::string& num, NumberBase base, Token& t);
	/**
	 * \brief convert number in string form to real number
	 */
	void parseFloat(std::string& num, Token& t);

	/**
	 * \brief read one byte from source file
	 */
	int  getCharFromSource();
	/**
	 * \brief return byte into the source file
	 */
	void ungetCharFromSource(int c);

public:

	/**
	 * \brief constructors
	 */
	Scanner();
	Scanner(FILE* input);

	/**
	 * \brief get next token from the source file
	 */
	Token getToken();
	
	/**
	 * \brief explicitly set source file
	 */
	void  setSource(FILE* input);
};












