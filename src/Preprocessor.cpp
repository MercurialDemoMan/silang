#include "Preprocessor.hpp"
#include "types.hpp"

//variables
static std::unordered_map<std::string, std::string> gVariables;

/**
 * \brief check type of command
 */
Preprocessor::TokenType Preprocessor::parseCmd()
{
	if(this->pBuffer == "inc")
	{
		return Preprocessor::TokenType::Inc;
	}
	else if(this->pBuffer == "def")
	{
		return Preprocessor::TokenType::Def;
	}
	else
	{
		return Preprocessor::TokenType::Null;
	}
}

/**
 * \brief check if identificator isn't macro
 */
void Preprocessor::parseId(std::string& id)
{
	auto it = gVariables.find(id);

	if(it == gVariables.end())
	{
		fwrite(id.c_str(), 1, id.size(), this->pOut);
	}
	else
	{
		fwrite(it->second.c_str(), 1, it->second.size(), this->pOut);
	}
}

/**
 * \brief get next preprocessor command
 */
//TODO: ignore comments
Preprocessor::Token Preprocessor::getToken()
{
	//create char buffer
	int charBuffer;

	//clear buffer
	this->pBuffer.clear();

	//reset state
	this->pState = Preprocessor::State::Start;

	//create token buffer
	Preprocessor::Token t;

	//main scanner loop
	while(true)
	{
		charBuffer = fgetc(this->pIn);

		switch(this->pState)
		{
			case Preprocessor::State::Start:
			{
				if(charBuffer == '$')
				{
					this->pState = Preprocessor::State::Cmd;
				}
				else if(charBuffer == EOF)
				{
					return Token(Preprocessor::TokenType::Eof);
				}
				else if(std::ext::ischar(charBuffer))
				{
					this->pBuffer.push_back((char)charBuffer);
					this->pState = Preprocessor::State::Id;
				}
				else
				{
					fputc(charBuffer, this->pOut);
					this->pState = Preprocessor::State::Start;
				}

				break;
			}

			case Preprocessor::State::Cmd:
			{
				if(std::isspace(charBuffer))
				{
					TokenType type = this->parseCmd();

					this->pBuffer.clear();

					switch(type)
					{
						case Preprocessor::TokenType::Inc:
						{
							this->pState = Preprocessor::State::IncFileStart;
							break;
						}
						case Preprocessor::TokenType::Def:
						{
							this->pState = Preprocessor::State::DefVarIdStart;
							break;
						}
						default: { throw std::runtime_error("Unexpected preprocessor command\n"); break; }
					}
				}
				else
				{
					this->pBuffer.push_back((char)charBuffer);
				}

				break;
			}

			case Preprocessor::State::IncFileStart:
			{
				if(std::isspace(charBuffer))
				{
					this->pState = Preprocessor::State::IncFileStart;
				}
				else if(charBuffer == '\"')
				{
					this->pState = Preprocessor::State::IncFileScan;
				}
				else
				{
					throw std::runtime_error("Expected file to include\n");
				}

				break;
			}

			case Preprocessor::State::IncFileScan:
			{
				if(charBuffer == '\"')
				{
					return Preprocessor::Token(Preprocessor::TokenType::Inc, this->pBuffer);
				}
				else if(charBuffer == EOF)
				{
					throw std::runtime_error("Expected \"\"\" when including file\n");
					return Preprocessor::Token();
				}
				else
				{
					this->pBuffer.push_back((char)charBuffer);
					this->pState = Preprocessor::State::IncFileScan;
				}

				break;
			}

			case Preprocessor::State::DefVarIdStart:
			{
				if(std::isspace(charBuffer))
				{
					this->pState = Preprocessor::State::DefVarIdStart;
				} 
				else if(charBuffer == EOF)
				{
					throw std::runtime_error("Expected varible name to be defined\n");
				}
				else
				{
					ungetc(charBuffer, this->pIn);
					this->pState = Preprocessor::State::DefVarId;
				}

				break;
			}

			case Preprocessor::State::DefVarId:
			{
				if(std::isspace(charBuffer))
				{
					t.type = Preprocessor::TokenType::Def;
					t.arg  = this->pBuffer;
					this->pBuffer.clear();
					this->pState = Preprocessor::State::DefVarValueStart;
				}
				else if(charBuffer == EOF || charBuffer == '\n')
				{
					return Preprocessor::Token(Preprocessor::TokenType::Def, this->pBuffer);
				}
				else
				{
					this->pBuffer.push_back((char)charBuffer);
					this->pState = Preprocessor::State::DefVarId;
				}

				break;
			}

			case Preprocessor::State::DefVarValueStart:
			{
				if(std::isspace(charBuffer))
				{
					this->pState = Preprocessor::State::DefVarValueStart;
				} 
				else if(charBuffer == EOF)
				{
					throw std::runtime_error("Expected varible name to be defined\n");
				}
				else
				{
					ungetc(charBuffer, this->pIn);
					this->pState = Preprocessor::State::DefVarValue;
				}

				break;
			}

			case Preprocessor::State::DefVarValue:
			{
				if(charBuffer == EOF || charBuffer == '\n')
				{
					t.arg2 = this->pBuffer;
					return t;
				}
				else
				{
					this->pBuffer.push_back((char)charBuffer);
					this->pState = Preprocessor::State::DefVarValue;
				}

				break;
			}

			case Preprocessor::State::Id:
			{
				if(std::ext::ischar(charBuffer) && charBuffer != ';')
				{
					this->pBuffer.push_back((char)charBuffer);
				}
				else
				{
					ungetc(charBuffer, this->pIn);
					this->parseId(pBuffer);
					return t;
				}

				break;
			}
		}
	}
}

bool Preprocessor::preprocess(FILE* in, FILE* out)
{
	this->pIn  = in;
	this->pOut = out;

	this->pState = Preprocessor::State::Start;

	do
	{
		try
		{
			this->pToken = this->getToken();
		}
		catch(std::exception& e)
		{
			std::printf("\nerror: %s", e.what());
			return false;
		}

		if(this->pToken.type == Preprocessor::TokenType::Eof)
		{
			break;
		}
		else if(this->pToken.type == Preprocessor::TokenType::Inc)
		{
			FILE* inc = fopen(this->pToken.arg.c_str(), "rb");

			if(inc == NULL)
			{
				std::printf("\nerror: cannot find included file [%s]\n", this->pToken.arg.c_str());
				return false;
			}

			Preprocessor* inc_preprocessor = new Preprocessor();
			inc_preprocessor->preprocess(inc, out);
			delete inc_preprocessor;

			fclose(inc);
		}
		else if(this->pToken.type == Preprocessor::TokenType::Def)
		{
			std::printf("prep: %s = %s\n", this->pToken.arg.c_str(), this->pToken.arg2.c_str());
			gVariables[this->pToken.arg] = this->pToken.arg2;
		}
	} while(true);



	return true;
}



