#include "Scanner.hpp"

/**
 * \brief string form of token type for debug purposes
 */
static const char* ScannerTokenTypeString[] = 
{
	[(u32)Scanner::TokenType::Null] = "TokenType::Null",

	[(u32)Scanner::TokenType::Eol] = "TokenType::Eol", 
	[(u32)Scanner::TokenType::Eof] = "TokenType::Eof", 
	[(u32)Scanner::TokenType::LeftCurlyBracket] = "TokenType::LeftCurlyBracket", 
	[(u32)Scanner::TokenType::RightCurlyBracket] = "TokenType::RightCurlyBracket",

	[(u32)Scanner::TokenType::LeftBracket] = "TokenType::LeftBracket", 
	[(u32)Scanner::TokenType::RightBracket] = "TokenType::RightBracket", 
	[(u32)Scanner::TokenType::Colon] = "TokenType::Colon", 
	[(u32)Scanner::TokenType::Comma] = "TokenType::Comma", 
	[(u32)Scanner::TokenType::SemiColon] = "TokenType::SemiColon",

	[(u32)Scanner::TokenType::Assign] = "TokenType::Assign", 
	[(u32)Scanner::TokenType::Less] = "TokenType::Less", 
	[(u32)Scanner::TokenType::More] = "TokenType::More", 
	[(u32)Scanner::TokenType::LessEqu] = "TokenType::LessEqu",
	[(u32)Scanner::TokenType::MoreEqu] = "TokenType::MoreEqu", 
	[(u32)Scanner::TokenType::Equ] = "TokenType::Equ",
	[(u32)Scanner::TokenType::NonEqu] = "TokenType::NonEqu", 
	[(u32)Scanner::TokenType::Plus] = "TokenType::Plus", 
	[(u32)Scanner::TokenType::Minus] = "TokenType::Minus", 
	[(u32)Scanner::TokenType::Mul] = "TokenType::Nul", 
	[(u32)Scanner::TokenType::Div] = "TokenType::Div", 

	[(u32)Scanner::TokenType::Id] = "TokenType::Id", 
	[(u32)Scanner::TokenType::Int] = "TokenType::Int", 
	[(u32)Scanner::TokenType::Float] = "TokenType::Float", 
	[(u32)Scanner::TokenType::String] = "TokenType::String", 
	[(u32)Scanner::TokenType::Keyword] = "TokenType::Keyword",
};

/**
 * \brief string form of keywords for debug purposes
 */
static const char* ScannerTokenKeywordTypeString[] = 
{
	[(u32)Scanner::KeywordType::Null] = "KeywordType::Null",

	[(u32)Scanner::KeywordType::Func] = "KeywordType::Func", 
	[(u32)Scanner::KeywordType::Else] = "KeywordType::Else", 
	[(u32)Scanner::KeywordType::If] = "KeywordType::If", 
	[(u32)Scanner::KeywordType::Return] = "KeywordType::Return", 
	[(u32)Scanner::KeywordType::While] = "KeywordType::While",
	[(u32)Scanner::KeywordType::For] = "KeywordType::For",
	[(u32)Scanner::KeywordType::Byte] = "KeywordType::Byte", 
	[(u32)Scanner::KeywordType::Int] = "KeywordType::Int", 
	[(u32)Scanner::KeywordType::Pack] = "KeywordType::Pack",
	[(u32)Scanner::KeywordType::Void] = "KeywordType::Void",
};

/**
 * \brief print debug info about token
 */
void Scanner::Token::print()
{
	std::printf("[token: %s", ScannerTokenTypeString[(u32)this->type]);

	if(this->type == TokenType::Id) { std::printf(", id: \"%s\"]", this->attribute.litString.c_str()); } else
	if(this->type == TokenType::Int) { std::printf(", int: %lli]", this->attribute.litInt); } else
	if(this->type == TokenType::Float) { std::printf(", id: %lf]", this->attribute.litFloat); } else
	if(this->type == TokenType::String) { std::printf(", id: \"%s\"]", this->attribute.litString.c_str()); } else
	if(this->type == TokenType::Keyword) { std::printf(", id: %s]", ScannerTokenKeywordTypeString[(u32)this->attribute.keyword]); } else
	{
		std::printf("]");
	}

	std::printf("\n");
}

/**
 * \brief initialize scanner
 */
Scanner::Scanner()
{
	pSource = nullptr;
	pState  = Scanner::State::Start;
	pBuffer = "";
}

/**
 * \brief initialize scanner
 */
Scanner::Scanner(FILE* input)
{
	pSource = input;
	pState  = Scanner::State::Start;
	pBuffer = "";
}

/**
 * \brief check if identificator is not keyword
 */
void Scanner::parseId(std::string& id, Token& t)
{
	//assume it is a keyword
	t.type = TokenType::Keyword;

	if(id == "byte")   { t.attribute.keyword = KeywordType::Byte; } else
	if(id == "int")    { t.attribute.keyword = KeywordType::Int; } else
	if(id == "float")  { t.attribute.keyword = KeywordType::Float; } else
	if(id == "void")   { t.attribute.keyword = KeywordType::Void; } else
	if(id == "func")   { t.attribute.keyword = KeywordType::Func; } else
	if(id == "if")     { t.attribute.keyword = KeywordType::If; } else
	if(id == "else")   { t.attribute.keyword = KeywordType::Else; } else
	if(id == "return") { t.attribute.keyword = KeywordType::Return; } else
	if(id == "while")  { t.attribute.keyword = KeywordType::While; } else
	if(id == "for")    { t.attribute.keyword = KeywordType::For; } else
	if(id == "pack")   { t.attribute.keyword = KeywordType::Pack; } else
	//if all checks failed -> token is identificator
	{ t.type = TokenType::Id; t.attribute.litString = id; }
}

/**
 * \brief convert number in string form to real number
 */
void Scanner::parseInt(std::string& num, NumberBase base, Token& t)
{
	t.type = TokenType::Int;
	switch(base)
	{
		case NumberBase::Hex: { t.attribute.litInt = (i64)std::strtoll(num.c_str(), NULL, 16); break; }
		case NumberBase::Dec: { t.attribute.litInt = (i64)std::strtoll(num.c_str(), NULL, 10); break; }
		case NumberBase::Oct: { t.attribute.litInt = (i64)std::strtoll(num.c_str(), NULL,  8); break; }
		case NumberBase::Bin: { t.attribute.litInt = (i64)std::strtoll(num.c_str(), NULL,  2); break; }
	}
}

/**
 * \brief convert number in string form to real number
 */
void Scanner::parseFloat(std::string& num, Token& t)
{
	t.type = TokenType::Float;
	t.attribute.litFloat = (f64)std::strtod(num.c_str(), NULL);
}

/**
 * \brief read one byte from source file
 */
int  Scanner::getCharFromSource()
{
	return std::fgetc(pSource);
}

/**
 * \brief return byte into the source file
 */
void Scanner::ungetCharFromSource(int c)
{
	std::ungetc(c, pSource);
}

/**
 * \brief get next token from the source file
 */
Scanner::Token Scanner::getToken()
{
	//create token
	Token t;

	//clear char buffer
	this->pBuffer.clear();

	//reset scanner state
	this->pState = Scanner::State::Start;

	//create char buffer
	int charBuffer = 0;

	//create hex escape sequence buffer
	char hexBuffer[3] = { 0 };

	//main loop
	while(true)
	{
		//fetch char from source
		charBuffer = this->getCharFromSource();

		switch(pState)
		{
			case Scanner::State::Start:
			{
				//skip white spaces
				if(charBuffer == '\n' || std::isspace(charBuffer))
				{ 
					pState = Scanner::State::Start;
				}
				//start scanning identificator
				else if(std::isalpha(charBuffer) || charBuffer == '_')
				{
					pBuffer.push_back((char)charBuffer);
					pState = Scanner::State::Id;
				}
				//start scanning number
				else if(std::isdigit(charBuffer))
				{
					pBuffer.push_back((char)charBuffer);
					pState  = Scanner::State::Number;
				}
				//start scanning string
				else if(charBuffer == '\"')
				{
					pState = Scanner::State::String;
				}
				//start scanning != token
				else if(charBuffer == '!')
				{
					pState = Scanner::State::NonEqu;
				}
				//start scanning < or <= token
				else if(charBuffer == '<')
				{
					pState = Scanner::State::Less;
				}
				//start scanning > or >= token
				else if(charBuffer == '>')
				{
					pState = Scanner::State::More;
				}
				//start scanning = or == token
				else if(charBuffer == '=')
				{
					pState = Scanner::State::Assign;
				}
				//return + token
				else if(charBuffer == '+')
				{
					return Token(Scanner::TokenType::Plus);
				}
				//return - token
				else if(charBuffer == '-')
				{
					return Token(Scanner::TokenType::Minus);
				}
				//return * token
				else if(charBuffer == '*')
				{
					return Token(Scanner::TokenType::Mul);
				}
				//return / token
				else if(charBuffer == '/')
				{
					return Token(Scanner::TokenType::Div);
				}
				//return ( token
				else if(charBuffer == '(')
				{
					return Token(Scanner::TokenType::LeftBracket);
				}
				//return ) token
				else if(charBuffer == ')')
				{
					return Token(Scanner::TokenType::RightBracket);
				}
				//return { token
				else if(charBuffer == '{')
				{
					return Token(Scanner::TokenType::LeftCurlyBracket);
				}
				//return } token
				else if(charBuffer == '}')
				{
					return Token(Scanner::TokenType::RightCurlyBracket);
				}
				//return , token
				else if(charBuffer == ',')
				{
					return Token(Scanner::TokenType::Comma);
				}
				//return . token
				else if(charBuffer == '.')
				{
					return Token(Scanner::TokenType::Dot);
				}
				//return : token
				else if(charBuffer == ':')
				{
					return Token(Scanner::TokenType::Colon);
				}
				//return ; token
				else if(charBuffer == ';')
				{
					return Token(Scanner::TokenType::SemiColon);
				}
				//start ignoring line or multiple lies
				else if(charBuffer == '#')
				{
					pState = Scanner::State::StartComment;
				}
				//return end of file token
				else if(charBuffer == EOF)
				{
					return Token(Scanner::TokenType::Eof);
				}
				//any other characted is invalid
				else
				{
					std::printf("Unexpected symbol\n");
					return Token(Scanner::TokenType::Null);
				}

				break;
			}

			//scanning identificator
			case Scanner::State::Id:
			{
				//valid identificator characters
				if(std::isalnum(charBuffer) || charBuffer == '_')
				{
					pBuffer.push_back((char)charBuffer);
				}
				//any invalid characted stops scanner,and returns identificator or keyword
				else
				{
					this->ungetCharFromSource(charBuffer);
					this->parseId(pBuffer, t);
					return t;
				}

				break;
			}

			//scanning number
			case Scanner::State::Number:
			{
				//digits
				if(std::isdigit(charBuffer))
				{
					pBuffer.push_back((char)charBuffer);
				}
				//change base to hexadecimal
				else if(charBuffer == 'x')
				{
					if(pBuffer.size() == 1 && pBuffer[0] == '0')
					{
						pBuffer.clear();
						pState  = Scanner::State::NumberHex;
					}
					else
					{
						std::printf("Number in hex base must be lead by: \"0x\"\n");
						return Token(Scanner::TokenType::Null);
					}
				}
				//change base to octal
				else if(charBuffer == 'o')
				{
					if(pBuffer.size() == 1 && pBuffer[0] == '0')
					{
						pBuffer.clear();
						pState  = Scanner::State::NumberOct;
					}
					else
					{
						std::printf("Number in oct base must be lead by: \"0o\"\n");
						return Token(Scanner::TokenType::Null);
					}
				}
				//change base to binary
				else if(charBuffer == 'b')
				{
					if(pBuffer.size() == 1 && pBuffer[0] == '0')
					{
						pBuffer.clear();
						pState  = Scanner::State::NumberBin;
					}
					else
					{
						std::printf("Number in bin base must be lead by: \"0b\"\n");
						return Token(Scanner::TokenType::Null);
					}
				}
				//we are no longer scanning integer but float
				else if(charBuffer == '.')
				{
					pBuffer.push_back((char)charBuffer);
					pState = Scanner::State::NumberFloatingPoint;
				}
				//we are no longer scanning integer but float
				else if(std::tolower(charBuffer) == 'e')
				{
					pBuffer.push_back((char)charBuffer);
					pState = Scanner::State::NumberExp;
				}
				//any invalid character will cause integer to be returned
				else
				{
					ungetCharFromSource(charBuffer);
					parseInt(pBuffer, NumberBase::Dec, t);
					return t;
				}

				break;
			}

			//scanning hex integer
			case Scanner::State::NumberHex:
			{
				if(std::isdigit(charBuffer) || (std::tolower(charBuffer) >= 'a' && std::tolower(charBuffer) <= 'f'))
				{
					pBuffer.push_back(charBuffer);
					pState = Scanner::State::NumberHex;
				}
				else
				{
					this->ungetCharFromSource(charBuffer);
					parseInt(pBuffer, NumberBase::Hex, t);
					return t;
				}
				break;
			}

			//scanning oct integer
			case Scanner::State::NumberOct:
			{
				if(charBuffer >= '0' && charBuffer <= '7')
				{
					pBuffer.push_back(charBuffer);
					pState = Scanner::State::NumberOct;
				}
				else
				{
					this->ungetCharFromSource(charBuffer);
					parseInt(pBuffer, NumberBase::Oct, t);
					return t;
				}
				break;
			}

			//scanning bin integer
			case Scanner::State::NumberBin:
			{
				if(charBuffer >= '0' && charBuffer <= '1')
				{
					pBuffer.push_back(charBuffer);
					pState = Scanner::State::NumberBin;
				}
				else
				{
					this->ungetCharFromSource(charBuffer);
					parseInt(pBuffer, NumberBase::Bin, t);
					return t;
				}
				break;
			}

			//start scanning floating point fraction
			case Scanner::State::NumberFloatingPoint:
			{
				if(std::isdigit(charBuffer))
				{
					pBuffer.push_back((char)charBuffer);
					pState = Scanner::State::NumberFraction;
				}
				else
				{
					std::printf("Unexpected symbol near number, after floating point should be digit\n");
					return Token(Scanner::TokenType::Null);
				}

				break;
			}

			//scanning floating point fraction
			case Scanner::State::NumberFraction:
			{
				if(std::isdigit(charBuffer))
				{
					pBuffer.push_back((char)charBuffer);
				}
				else if(std::tolower(charBuffer) == 'e')
				{
					pBuffer.push_back((char)charBuffer);
					pState = Scanner::State::NumberExp;
				}
				else
				{
					ungetCharFromSource(charBuffer);
					parseFloat(pBuffer, t);
					return t;
				}

				break;
			}

			//scanning exponent
			case Scanner::State::NumberExp:
			{
				if(std::isdigit(charBuffer))
				{
					pBuffer.push_back((char)charBuffer);
					pState = Scanner::State::NumberExpTail;
				}
				else if(charBuffer == '+' || charBuffer == '-')
				{
					pBuffer.push_back((char)charBuffer);
					pState = Scanner::State::NumberExpSign;
				}
				else
				{
					std::printf("Unexpected symbol near exponential number, after \"e\" symbol should be digit or sign\n");
					return Token(Scanner::TokenType::Null);
				}

				break;
			}

			//scanning sign
			case Scanner::State::NumberExpSign:
			{
				if(std::isdigit(charBuffer))
				{
					pBuffer.push_back((char)charBuffer);
					pState = Scanner::State::NumberExpTail;
				}
				else
				{
					std::printf("Unexpected symbol near exponential number, after exponential sign should be digit\n");
					return Token(Scanner::TokenType::Null);
				}

				break;
			}

			//scanning exponent tail
			case Scanner::State::NumberExpTail:
			{
				if(std::isdigit(charBuffer))
				{
					pBuffer.push_back((char)charBuffer);
				}
				else
				{
					ungetCharFromSource(charBuffer);
					parseFloat(pBuffer, t);
					return t;
				}

				break;
			}

			//scanning string
			case Scanner::State::String:
			{
				if(std::ext::isprintable(charBuffer))
				{
					if(charBuffer == '\"')
					{
						t.type                = TokenType::String;
						t.attribute.litString = pBuffer;
						return t;
					}
					else if(charBuffer == '\\')
					{
						pState = Scanner::State::StringEscape;
					}
					else
					{
						pBuffer.push_back((char)charBuffer);
					}
				}
				else
				{
					std::printf("Non ascii symbol inside a string\n");
					return Token(Scanner::TokenType::Null);
				}

				break;
			}

			//scanning string escape sequence
			case Scanner::State::StringEscape:
			{
				if(charBuffer == 'a')
				{
					pBuffer.push_back('\a');
					pState = Scanner::State::String;
				}
				else if(charBuffer == 'b')
				{
					pBuffer.push_back('\b');
					pState = Scanner::State::String;
				}
				else if(charBuffer == 'e')
				{
					pBuffer.push_back('\e');
					pState = Scanner::State::String;
				}
				else if(charBuffer == 'f')
				{
					pBuffer.push_back('\f');
					pState = Scanner::State::String;
				}
				else if(charBuffer == 'n')
				{
					pBuffer.push_back('\n');
					pState = Scanner::State::String;
				}
				else if(charBuffer == 'r')
				{
					pBuffer.push_back('\r');
					pState = Scanner::State::String;
				}
				else if(charBuffer == 't')
				{
					pBuffer.push_back('\t');
					pState = Scanner::State::String;
				}
				else if(charBuffer == 'v')
				{
					pBuffer.push_back('\v');
					pState = Scanner::State::String;
				}
				else if(charBuffer == '\\')
				{
					pBuffer.push_back('\\');
					pState = Scanner::State::String;
				}
				else if(charBuffer == '\'')
				{
					pBuffer.push_back('\'');
					pState = Scanner::State::String;
				}
				else if(charBuffer == '\"')
				{
					pBuffer.push_back('\"');
					pState = Scanner::State::String;
				}
				else if(charBuffer == 'n')
				{
					pBuffer.push_back('\n');
					pState = Scanner::State::String;
				}
				else if(charBuffer == 'x')
				{
					pState = Scanner::State::StringEscapeHex1;
				}
				else
				{
					std::printf("Unknown escape sequence\n");
					return Token(Scanner::TokenType::Null);
				}

				break;
			}

			//scanning first digit of hex escape sequence
			case Scanner::State::StringEscapeHex1:
			{
				charBuffer = std::tolower(charBuffer);

				if(std::isdigit(charBuffer) || (charBuffer >= 'a' && charBuffer <= 'f'))
				{
					hexBuffer[0] = (char)charBuffer;
					pState = Scanner::State::StringEscapeHex2;
				}
				else
				{
					std::printf("Hex escape sequence expects two heaxadecimal numbers\n");
					return Token(Scanner::TokenType::Null);
				}

				break;
			}

			//scanning second digit of hex escape sequence
			case Scanner::State::StringEscapeHex2:
			{
				charBuffer = std::tolower(charBuffer);

				if(std::isdigit(charBuffer) || (charBuffer >= 'a' && charBuffer <= 'f'))
				{
					hexBuffer[1] = (char)charBuffer;
					pBuffer.push_back((char)strtol(hexBuffer, NULL, 16));
					pState = Scanner::State::String;
				}
				else
				{
					std::printf("Hex escape sequence expects two heaxadecimal numbers\n");
					return Token(Scanner::TokenType::Null);
				}

				break;
			}

			//scanning != token
			case Scanner::State::NonEqu:
			{
				if(charBuffer == '=')
				{
					return Token(Scanner::TokenType::NonEqu);
				}
				else
				{
					std::printf("Unexpected symbol after \"!\"\n");
					return Token(Scanner::TokenType::Null);
				}

				break;
			}

			//scanning < or <= token
			case Scanner::State::Less:
			{
				if(charBuffer == '=')
				{
					return Token(Scanner::TokenType::LessEqu);
				}
				else
				{
					ungetCharFromSource(charBuffer);
					return Token(Scanner::TokenType::Less);
				}

				break;
			}

			//scanning > or >= token
			case Scanner::State::More:
			{
				if(charBuffer == '=')
				{
					return Token(Scanner::TokenType::MoreEqu);
				}
				else
				{
					ungetCharFromSource(charBuffer);
					return Token(Scanner::TokenType::More);
				}

				break;
			}

			//scanning = or == token
			case Scanner::State::Assign:
			{
				if(charBuffer == '=')
				{
					return Token(Scanner::TokenType::Equ);
				}
				else
				{
					ungetCharFromSource(charBuffer);
					return Token(Scanner::TokenType::Assign);
				}

				break;
			}

			//scanning line comment or multiline comment
			case Scanner::State::StartComment:
			{
				if(charBuffer == '#')
				{
					pState = Scanner::State::MulComment;
				}
				else
				{
					pState = Scanner::State::Comment;
				}

				break;
			}

			//skipping line
			case Scanner::State::Comment:
			{
				if(charBuffer == '\n' || charBuffer == EOF)
				{
					pState = Scanner::State::Start;
				}
				else
				{
					pState = Scanner::State::Comment;
				}

				break;
			}

			//skipping multiple lines
			case Scanner::State::MulComment:
			{
				if(charBuffer == '#' || charBuffer == EOF)
				{
					pState = Scanner::State::EndMulComment;
				}
				else
				{
					pState = Scanner::State::MulComment;
				}

				break;
			}

			//end multiline comment
			case Scanner::State::EndMulComment:
			{
				if(charBuffer == '#' || charBuffer == EOF)
				{
					pState = Scanner::State::Start;
				}
				else
				{
					pState = Scanner::State::MulComment;
				}

				break;
			}
		}
	}

	return t;
}


















