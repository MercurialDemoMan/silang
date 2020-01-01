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
		Null,				// invalid token
		Acc,				// special data token for evaluating expressions
		Ret,				// special data token for evaluating expressions

		Eof, 				// end of file
		LeftCurlyBracket, 	// {
		RightCurlyBracket,  // }

		LeftBracket, 		// (
		RightBracket, 		// )
		Dot,				// .
		Colon, 				// :
		Comma, 				// ,
		SemiColon,			// ;

		Assign, 			// =
		Less, 				// <
		More, 				// >
		LessEqu, 			// <=
		MoreEqu, 			// >=
		Equ,				// ==
		NonEqu, 			// !=
		Plus, 				// +
		Minus, 				// -
		Mul, 				// *
		Div, 				// /

		Id, 				// identificator
		Int,				// integer
		Float, 				// floating point number
		String, 			// literal string
		Keyword,			// special identificator
	};

	/**
	 * \brief keyword type
	 */
	enum class KeywordType
	{
		Null,	// invalid keyword

		Func, 	// func
		Else, 	// else
		If, 	// if
		Return, // return
		While, 	// while
		For,	// for
		Byte,	// byte
		Int, 	// int
		Float,	// float
		Pack,	// pack
		Void,	// void
	};

	/**
	 * \brief token structure
	 */
	struct Token
	{
		//token type
		TokenType type;

		//based on type, token can have attribute
		struct TokenAttribute
		{
			std::string litString;
			i64			litInt;
			f64		    litFloat;
			KeywordType keyword;
		} attribute;

		//constructors
		Token() 		   { type = TokenType::Null; }
		Token(TokenType t) { type = t; }

		//debug print
		void print();
	};

private:

	/**
	 * \brief all scanner states
	 */
	enum class State
	{
		Start, 					// start scanning token
		Id,						// scanning identificator
		Number,					// scanning integer or float
		NumberHex,				// scanning integer in hexadecimal base
		NumberOct,				// scanning integer in octal base
		NumberBin,				// scanning integer in binary base
		NumberFloatingPoint,	// start scanning float fraction
		NumberFraction,			// scanning float fraction
		NumberExp,				// start scanning number exponent
		NumberExpSign,			// start scanning number exponent
		NumberExpTail,			// scanning number exponent
		String,					// scanning literal string
		StringEscape,			// start scanning escape sequence in string
		StringEscapeHex1,		// scanning hexadecimal escape sequence in string
		StringEscapeHex2,		// scanning hexadecimal escape sequence in string
		NonEqu,					// scanning "!=" token
		Less,					// scanning "<" or "<=" token
		More,					// scanning ">" or ">=" token
		Assign,					// scanning "=" or "==" token
		StartComment,			// skiping line
		Comment,				// skiping line
		MulComment,				// skiping multiple lines
		EndMulComment,			// skiping multiple lines
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












