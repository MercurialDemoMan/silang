/**
 * \header expression evaluation using infix to postfix algorithm
 */

#include "Parser.hpp"

#include <vector>
#include <stack>

/**
 * \brief create operation plus manage operator priority
 */
static void ParserExprOperation(Scanner::Token& token, std::vector<Scanner::Token>& operationStack, std::vector<Scanner::Token>& postfixResult)
{
	//we have nothing to compare the operation with
	//just push the operator onto the stack
	if(operationStack.size() == 0)
	{
		operationStack.push_back(token); return;
	}

	//compare the operation with the top of the stack
	Scanner::Token topRef = operationStack.back();

	//left bracket indicates nested expression = no operations on the stack
	//just push it
	if(topRef.type == Scanner::TokenType::LeftBracket)
	{
		operationStack.push_back(token); return;
	}

	//manage operator priority
	//if current operation has higher priority than what's on the stack
	//just push it
	//TODO: use precedence table
	if((topRef.type == Scanner::TokenType::Plus || topRef.type == Scanner::TokenType::Minus) &&
	   (token.type  == Scanner::TokenType::Mul  || token.type  == Scanner::TokenType::Div))
	{
		operationStack.push_back(token); return;
	}
	if((topRef.type == Scanner::TokenType::Plus || topRef.type == Scanner::TokenType::Minus) &&
	   (token.type  == Scanner::TokenType::Equ  || token.type  == Scanner::TokenType::NonEqu))
	{
		operationStack.push_back(token); return;
	}
	if((topRef.type == Scanner::TokenType::Plus || topRef.type == Scanner::TokenType::Minus) &&
	   (token.type  == Scanner::TokenType::Less || token.type  == Scanner::TokenType::LessEqu))
	{
		operationStack.push_back(token); return;
	}
	if((topRef.type == Scanner::TokenType::Plus || topRef.type == Scanner::TokenType::Minus) &&
	   (token.type  == Scanner::TokenType::More || token.type  == Scanner::TokenType::MoreEqu))
	{
		operationStack.push_back(token); return;
	}
	if((topRef.type == Scanner::TokenType::Mul  || topRef.type == Scanner::TokenType::Div) &&
	   (token.type  == Scanner::TokenType::Equ  || token.type  == Scanner::TokenType::NonEqu))
	{
		operationStack.push_back(token); return;
	}
	if((topRef.type == Scanner::TokenType::Mul  || topRef.type == Scanner::TokenType::Div) &&
	   (token.type  == Scanner::TokenType::Less || token.type  == Scanner::TokenType::LessEqu))
	{
		operationStack.push_back(token); return;
	}
	if((topRef.type == Scanner::TokenType::Mul  || topRef.type == Scanner::TokenType::Div) &&
	   (token.type  == Scanner::TokenType::More || token.type  == Scanner::TokenType::MoreEqu))
	{
		operationStack.push_back(token); return;
	}

	//do the operation
	postfixResult.push_back(operationStack.back());
	operationStack.pop_back();

	//there are more operations to take care of -> recursively call itself
	ParserExprOperation(token, operationStack, postfixResult);
}

/**
 * \brief helper function to see better expr token content
 */
static void ParserExprTokenPrint(Scanner::Token& token)
{
	switch(token.type)
	{
		case Scanner::TokenType::String:
		case Scanner::TokenType::Id:  { std::printf("%s", token.attribute.litString.c_str()); break; }
		case Scanner::TokenType::Int: { std::printf("%lli", token.attribute.litInt); break; }
		case Scanner::TokenType::Float: { std::printf("%lf", token.attribute.litFloat); break; }
		case Scanner::TokenType::Plus: { std::printf(" + "); break; }
		case Scanner::TokenType::Minus: { std::printf(" - "); break; }
		case Scanner::TokenType::Mul: { std::printf(" * "); break; }
		case Scanner::TokenType::Div: { std::printf(" / "); break; }
		case Scanner::TokenType::Less: { std::printf(" < "); break; }
		case Scanner::TokenType::LessEqu: { std::printf(" <= "); break; }
		case Scanner::TokenType::More: { std::printf(" > "); break; }
		case Scanner::TokenType::MoreEqu: { std::printf(" >= "); break; }
		case Scanner::TokenType::Equ: { std::printf(" == "); break; }
		case Scanner::TokenType::NonEqu: { std::printf(" != "); break; }
		case Scanner::TokenType::Acc: { std::printf("pop()"); break; }
		case Scanner::TokenType::Ret: { std::printf("rr"); break; }
		default: { std::printf(" NaR "); break; }
	}
}

/**
 * \brief expression evaluation using infix to postfix algorithm
 * \note first token is expected to be fetched and last token is eaten
 */
Error Parser::expr(bool resOnStack)
{
	//data stack for operations and evaluation
	std::vector<Scanner::Token> operationStack;
	//stack for storing postfix expression
	std::vector<Scanner::Token> postfixResult;

	//keep track of bracket balance
	u64 ParserExprBracketBalance = 0;
	//flag for evaluating immediate constant
	bool immediateEvaluation     = true;

	//used for indexing registers and to see how complex the expression is
	u64 gExprStackSize = 0;

	//main infix to postfix loop
	//first token is expected to be fetched
	do
	{
		//add data to stack
		if(this->pToken.type == Scanner::TokenType::Id    ||
		   this->pToken.type == Scanner::TokenType::Int   ||
		   this->pToken.type == Scanner::TokenType::Float ||
		   this->pToken.type == Scanner::TokenType::String)
		{
			//if data is identificator, check what it is identifying
			if(this->pToken.type == Scanner::TokenType::Id)
			{
				//we are not longer evaluating immediate constant
				immediateEvaluation = false;

				//if the identificator is variable, continue in execution
				if(this->pVariables.find(this->pToken.attribute.litString) != this->pVariables.end())
				{
					(void)0;
				}
				//if the identificator is function, first evaluate expression for arguments and then call it
				else if(this->pFunctions.find(this->pToken.attribute.litString) != this->pFunctions.end())
				{
					//save function name so we can call it
					std::string functionName = this->pToken.attribute.litString;

					this->pToken = this->pScanner.getToken();
					//after function id must be LEFT BRACKET
					if(this->pToken.type == Scanner::TokenType::LeftBracket)
					{
						//evaluate arguments
						this->args();
						//call the function
						std::printf("	call %s\n", functionName.c_str());
						//push return data on the stack
						this->pToken.type = Scanner::TokenType::Ret;
					}
					else
					{
						return Error(Error::Type::Syntax, "Expected left bracket after function identificator");
					}
				}
				else
				{
					return Error(Error::Type::Syntax, "Refering to variable or function in expression that doesn't exists");
				}
			}

			//push data onto the stack
			postfixResult.push_back(this->pToken);
		}
		//start new nested expression
		else if(this->pToken.type == Scanner::TokenType::LeftBracket)
		{
			ParserExprBracketBalance++;
			operationStack.push_back(this->pToken);
		}
		//end nested expression but only if the right bracket is part of the expression
		else if(this->pToken.type == Scanner::TokenType::RightBracket && ParserExprBracketBalance != 0)
		{
			ParserExprBracketBalance--;
			while(operationStack.size() != 0)
			{
				Scanner::Token t = operationStack.back();
				operationStack.pop_back();

				if(t.type == Scanner::TokenType::LeftBracket)
				{
					break;
				}

				postfixResult.push_back(t);
			}
		}
		//evaluate operation
		else if(this->pToken.type == Scanner::TokenType::Plus    ||
		        this->pToken.type == Scanner::TokenType::Minus   ||
		        this->pToken.type == Scanner::TokenType::Mul     ||
		        this->pToken.type == Scanner::TokenType::Div     ||
		        this->pToken.type == Scanner::TokenType::Less    ||
		        this->pToken.type == Scanner::TokenType::LessEqu ||
		        this->pToken.type == Scanner::TokenType::More    ||
		        this->pToken.type == Scanner::TokenType::MoreEqu ||
		        this->pToken.type == Scanner::TokenType::Equ     ||
		        this->pToken.type == Scanner::TokenType::NonEqu)
		{
			ParserExprOperation(this->pToken, operationStack, postfixResult);
		}
		//anything else idicates end of the expression
		else
		{
			while(operationStack.size() != 0)
			{
				postfixResult.push_back(operationStack.back());
				operationStack.pop_back();
			}

			break;
		}

		//fetch next token
		this->pToken = this->pScanner.getToken();
	} while(true);

	//we converted infix to postfix
	//now we need to actually evaluate the expression

	//clear operation stack for further use
	operationStack.clear();

	//main evaluation loop
	for(u64 i = 0; i < postfixResult.size(); i++)
	{
		//if there are data, push it onto the operation stack
		if(postfixResult[i].type == Scanner::TokenType::Id     ||
		   postfixResult[i].type == Scanner::TokenType::Int    ||
		   postfixResult[i].type == Scanner::TokenType::Float  ||
		   postfixResult[i].type == Scanner::TokenType::String ||
		   postfixResult[i].type == Scanner::TokenType::Acc    ||
		   postfixResult[i].type == Scanner::TokenType::Ret)
		{
			operationStack.push_back(postfixResult[i]);
			if(gExprStackSize < ARCH_REG_NUM)
			{
				if(immediateEvaluation == false)
				{
					//we are working with variables
					std::printf("	r%llu = ", gExprStackSize++);
					ParserExprTokenPrint(postfixResult[i]);
					std::printf("\n");
				}
				else
				{
					gExprStackSize++;
				}
			}
			else
			{
				return Error(Error::Type::Syntax, "Expression is too complex");
			}
		}
		//if there is operation, pop 2 arguments from the operation stack, do the operation and result push back into the operation stack
		else if(postfixResult[i].type == Scanner::TokenType::Plus    ||
		        postfixResult[i].type == Scanner::TokenType::Minus   ||
		        postfixResult[i].type == Scanner::TokenType::Mul     ||
		        postfixResult[i].type == Scanner::TokenType::Div     ||
		        postfixResult[i].type == Scanner::TokenType::Less    ||
		        postfixResult[i].type == Scanner::TokenType::LessEqu ||
		        postfixResult[i].type == Scanner::TokenType::More    ||
		        postfixResult[i].type == Scanner::TokenType::MoreEqu ||
		        postfixResult[i].type == Scanner::TokenType::Equ     ||
		        postfixResult[i].type == Scanner::TokenType::NonEqu)
		{
			if(operationStack.size() < 2)
			{
				return Error(Error::Type::Syntax, "Expected 2 arguments for operation");
			}

			//pop second argument
			Scanner::Token sec_op = operationStack.back(); 
			operationStack.pop_back();
			//pop first argumnet
			Scanner::Token fir_op = operationStack.back();
			operationStack.pop_back();

			//TODO: do this better
			//if both operands are constants, we can immediately evaluate the operation in compile time
			if((fir_op.type == Scanner::TokenType::Int || fir_op.type == Scanner::TokenType::Float) && 
			   (sec_op.type == Scanner::TokenType::Int || sec_op.type == Scanner::TokenType::Float) && immediateEvaluation)
			{
				switch(postfixResult[i].type)
				{
					case Scanner::TokenType::Plus: 
					{ 
						if(fir_op.type != sec_op.type)
						{
							if(fir_op.type == Scanner::TokenType::Int)
							{
								operationStack.push_back(Scanner::Token(Scanner::TokenType::Float));
								operationStack[operationStack.size() - 1].attribute.litFloat = fir_op.attribute.litInt + sec_op.attribute.litFloat;
							}
							else
							{
								operationStack.push_back(Scanner::Token(Scanner::TokenType::Float));
								operationStack[operationStack.size() - 1].attribute.litFloat = fir_op.attribute.litFloat + sec_op.attribute.litInt;
							}
						}
						else
						{
							if(fir_op.type == Scanner::TokenType::Int)
							{
								operationStack.push_back(Scanner::Token(Scanner::TokenType::Int));
								operationStack[operationStack.size() - 1].attribute.litInt = fir_op.attribute.litInt + sec_op.attribute.litInt;
							}
							else
							{
								operationStack.push_back(Scanner::Token(Scanner::TokenType::Float));
								operationStack[operationStack.size() - 1].attribute.litFloat = fir_op.attribute.litFloat + sec_op.attribute.litFloat;
							}
						}
						break; 
					}
					case Scanner::TokenType::Minus: 
					{ 
						if(fir_op.type != sec_op.type)
						{
							if(fir_op.type == Scanner::TokenType::Int)
							{
								operationStack.push_back(Scanner::Token(Scanner::TokenType::Float));
								operationStack[operationStack.size() - 1].attribute.litFloat = fir_op.attribute.litInt - sec_op.attribute.litFloat;
							}
							else
							{
								operationStack.push_back(Scanner::Token(Scanner::TokenType::Float));
								operationStack[operationStack.size() - 1].attribute.litFloat = fir_op.attribute.litFloat - sec_op.attribute.litInt;
							}
						}
						else
						{
							if(fir_op.type == Scanner::TokenType::Int)
							{
								operationStack.push_back(Scanner::Token(Scanner::TokenType::Int));
								operationStack[operationStack.size() - 1].attribute.litInt = fir_op.attribute.litInt - sec_op.attribute.litInt;
							}
							else
							{
								operationStack.push_back(Scanner::Token(Scanner::TokenType::Float));
								operationStack[operationStack.size() - 1].attribute.litFloat = fir_op.attribute.litFloat - sec_op.attribute.litFloat;
							}
						}
						break; 
					}
					case Scanner::TokenType::Mul: 
					{ 
						if(fir_op.type != sec_op.type)
						{
							if(fir_op.type == Scanner::TokenType::Int)
							{
								operationStack.push_back(Scanner::Token(Scanner::TokenType::Float));
								operationStack[operationStack.size() - 1].attribute.litFloat = fir_op.attribute.litInt * sec_op.attribute.litFloat;
							}
							else
							{
								operationStack.push_back(Scanner::Token(Scanner::TokenType::Float));
								operationStack[operationStack.size() - 1].attribute.litFloat = fir_op.attribute.litFloat * sec_op.attribute.litInt;
							}
						}
						else
						{
							if(fir_op.type == Scanner::TokenType::Int)
							{
								operationStack.push_back(Scanner::Token(Scanner::TokenType::Int));
								operationStack[operationStack.size() - 1].attribute.litInt = fir_op.attribute.litInt * sec_op.attribute.litInt;
							}
							else
							{
								operationStack.push_back(Scanner::Token(Scanner::TokenType::Float));
								operationStack[operationStack.size() - 1].attribute.litFloat = fir_op.attribute.litFloat * sec_op.attribute.litFloat;
							}
						}

						break; 
					}
					case Scanner::TokenType::Div: 
					{ 
						if(fir_op.type != sec_op.type)
						{
							if(fir_op.type == Scanner::TokenType::Int)
							{
								if(sec_op.attribute.litFloat == 0.0) { return Error(Error::Type::Syntax, "Cannot divide by zero"); }

								operationStack.push_back(Scanner::Token(Scanner::TokenType::Float));
								operationStack[operationStack.size() - 1].attribute.litFloat = fir_op.attribute.litInt / sec_op.attribute.litFloat;
							}
							else
							{
								if(sec_op.attribute.litInt == 0) { return Error(Error::Type::Syntax, "Cannot divide by zero"); }

								operationStack.push_back(Scanner::Token(Scanner::TokenType::Float));
								operationStack[operationStack.size() - 1].attribute.litFloat = fir_op.attribute.litFloat / sec_op.attribute.litInt;
							}
						}
						else
						{
							if(fir_op.type == Scanner::TokenType::Int)
							{
								if(sec_op.attribute.litInt == 0.0) { return Error(Error::Type::Syntax, "Cannot divide by zero"); }

								operationStack.push_back(Scanner::Token(Scanner::TokenType::Int));
								operationStack[operationStack.size() - 1].attribute.litInt = fir_op.attribute.litInt / sec_op.attribute.litInt;
							}
							else
							{
								if(sec_op.attribute.litFloat == 0) { return Error(Error::Type::Syntax, "Cannot divide by zero"); }

								operationStack.push_back(Scanner::Token(Scanner::TokenType::Float));
								operationStack[operationStack.size() - 1].attribute.litFloat = fir_op.attribute.litFloat / sec_op.attribute.litFloat;
							}
						}
						break;
					}
					case Scanner::TokenType::Less: 
					{ 
						if(fir_op.type != sec_op.type)
						{
							if(fir_op.type == Scanner::TokenType::Int)
							{
								operationStack.push_back(Scanner::Token(Scanner::TokenType::Int));
								operationStack[operationStack.size() - 1].attribute.litInt = fir_op.attribute.litInt < sec_op.attribute.litFloat;
							}
							else
							{
								operationStack.push_back(Scanner::Token(Scanner::TokenType::Int));
								operationStack[operationStack.size() - 1].attribute.litInt = fir_op.attribute.litFloat < sec_op.attribute.litInt;
							}
						}
						else
						{
							if(fir_op.type == Scanner::TokenType::Int)
							{
								operationStack.push_back(Scanner::Token(Scanner::TokenType::Int));
								operationStack[operationStack.size() - 1].attribute.litInt = fir_op.attribute.litInt < sec_op.attribute.litInt;
							}
							else
							{
								operationStack.push_back(Scanner::Token(Scanner::TokenType::Int));
								operationStack[operationStack.size() - 1].attribute.litInt = fir_op.attribute.litFloat < sec_op.attribute.litFloat;
							}
						}
						break; 
					}
					case Scanner::TokenType::LessEqu:
					{ 
						if(fir_op.type != sec_op.type)
						{
							if(fir_op.type == Scanner::TokenType::Int)
							{
								operationStack.push_back(Scanner::Token(Scanner::TokenType::Int));
								operationStack[operationStack.size() - 1].attribute.litInt = fir_op.attribute.litInt <= sec_op.attribute.litFloat;
							}
							else
							{
								operationStack.push_back(Scanner::Token(Scanner::TokenType::Int));
								operationStack[operationStack.size() - 1].attribute.litInt = fir_op.attribute.litFloat <= sec_op.attribute.litInt;
							}
						}
						else
						{
							if(fir_op.type == Scanner::TokenType::Int)
							{
								operationStack.push_back(Scanner::Token(Scanner::TokenType::Int));
								operationStack[operationStack.size() - 1].attribute.litInt = fir_op.attribute.litInt <= sec_op.attribute.litInt;
							}
							else
							{
								operationStack.push_back(Scanner::Token(Scanner::TokenType::Int));
								operationStack[operationStack.size() - 1].attribute.litInt = fir_op.attribute.litFloat <= sec_op.attribute.litFloat;
							}
						}
						break; 
					}
					case Scanner::TokenType::More:
					{ 
						if(fir_op.type != sec_op.type)
						{
							if(fir_op.type == Scanner::TokenType::Int)
							{
								operationStack.push_back(Scanner::Token(Scanner::TokenType::Int));
								operationStack[operationStack.size() - 1].attribute.litInt = fir_op.attribute.litInt > sec_op.attribute.litFloat;
							}
							else
							{
								operationStack.push_back(Scanner::Token(Scanner::TokenType::Int));
								operationStack[operationStack.size() - 1].attribute.litInt = fir_op.attribute.litFloat > sec_op.attribute.litInt;
							}
						}
						else
						{
							if(fir_op.type == Scanner::TokenType::Int)
							{
								operationStack.push_back(Scanner::Token(Scanner::TokenType::Int));
								operationStack[operationStack.size() - 1].attribute.litInt = fir_op.attribute.litInt > sec_op.attribute.litInt;
							}
							else
							{
								operationStack.push_back(Scanner::Token(Scanner::TokenType::Int));
								operationStack[operationStack.size() - 1].attribute.litInt = fir_op.attribute.litFloat > sec_op.attribute.litFloat;
							}
						}
						break; 
					}
					case Scanner::TokenType::MoreEqu:
					{ 
						if(fir_op.type != sec_op.type)
						{
							if(fir_op.type == Scanner::TokenType::Int)
							{
								operationStack.push_back(Scanner::Token(Scanner::TokenType::Int));
								operationStack[operationStack.size() - 1].attribute.litInt = fir_op.attribute.litInt >= sec_op.attribute.litFloat;
							}
							else
							{
								operationStack.push_back(Scanner::Token(Scanner::TokenType::Int));
								operationStack[operationStack.size() - 1].attribute.litInt = fir_op.attribute.litFloat >= sec_op.attribute.litInt;
							}
						}
						else
						{
							if(fir_op.type == Scanner::TokenType::Int)
							{
								operationStack.push_back(Scanner::Token(Scanner::TokenType::Int));
								operationStack[operationStack.size() - 1].attribute.litInt = fir_op.attribute.litInt >= sec_op.attribute.litInt;
							}
							else
							{
								operationStack.push_back(Scanner::Token(Scanner::TokenType::Int));
								operationStack[operationStack.size() - 1].attribute.litInt = fir_op.attribute.litFloat >= sec_op.attribute.litFloat;
							}
						}
						break; 
					}
					case Scanner::TokenType::Equ:
					{ 
						if(fir_op.type != sec_op.type)
						{
							if(fir_op.type == Scanner::TokenType::Int)
							{
								operationStack.push_back(Scanner::Token(Scanner::TokenType::Int));
								operationStack[operationStack.size() - 1].attribute.litInt = fir_op.attribute.litInt == sec_op.attribute.litFloat;
							}
							else
							{
								operationStack.push_back(Scanner::Token(Scanner::TokenType::Int));
								operationStack[operationStack.size() - 1].attribute.litInt = fir_op.attribute.litFloat == sec_op.attribute.litInt;
							}
						}
						else
						{
							if(fir_op.type == Scanner::TokenType::Int)
							{
								operationStack.push_back(Scanner::Token(Scanner::TokenType::Int));
								operationStack[operationStack.size() - 1].attribute.litInt = fir_op.attribute.litInt == sec_op.attribute.litInt;
							}
							else
							{
								operationStack.push_back(Scanner::Token(Scanner::TokenType::Int));
								operationStack[operationStack.size() - 1].attribute.litInt = fir_op.attribute.litFloat == sec_op.attribute.litFloat;
							}
						}
						break; 
					}
					case Scanner::TokenType::NonEqu:
					{ 
						if(fir_op.type != sec_op.type)
						{
							if(fir_op.type == Scanner::TokenType::Int)
							{
								operationStack.push_back(Scanner::Token(Scanner::TokenType::Int));
								operationStack[operationStack.size() - 1].attribute.litInt = fir_op.attribute.litInt != sec_op.attribute.litFloat;
							}
							else
							{
								operationStack.push_back(Scanner::Token(Scanner::TokenType::Int));
								operationStack[operationStack.size() - 1].attribute.litInt = fir_op.attribute.litFloat != sec_op.attribute.litInt;
							}
						}
						else
						{
							if(fir_op.type == Scanner::TokenType::Int)
							{
								operationStack.push_back(Scanner::Token(Scanner::TokenType::Int));
								operationStack[operationStack.size() - 1].attribute.litInt = fir_op.attribute.litInt != sec_op.attribute.litInt;
							}
							else
							{
								operationStack.push_back(Scanner::Token(Scanner::TokenType::Int));
								operationStack[operationStack.size() - 1].attribute.litInt = fir_op.attribute.litFloat != sec_op.attribute.litFloat;
							}
						}
						break; 
					}
					default: 
					{ 
						return Error(Error::Type::Syntax, "Unexpected operator between constants");
						break; 
					}
				}

				gExprStackSize--;
			}
			//we evaluating operation baby!
			else
			{
				//check if we have enough registers
				if((gExprStackSize - 2) < ARCH_REG_NUM)
				{
					//result register
					std::printf("	r%llu = ", (gExprStackSize - 2));

					//first argument register
					std::printf("r%llu", (gExprStackSize - 2));
					//operation
					ParserExprTokenPrint(postfixResult[i]);
					//second argument register
					if((gExprStackSize - 1) < ARCH_REG_NUM)
					{
						std::printf("r%llu", (gExprStackSize - 1));
					}
					else
					{
						return Error(Error::Type::Syntax, "Expression is too complex");
					}
					std::printf("\n");
				}
				else
				{
					return Error(Error::Type::Syntax, "Expression is too complex");
				}

				//we push uncertain result
				operationStack.push_back(Scanner::Token(Scanner::TokenType::Acc));

				//we poped 2 and will push 1 -> -1
				gExprStackSize--;
			}
		}
	}

	//if necessary, assign the expression result
	if(immediateEvaluation == true)
	{
		if(resOnStack == true)
		{
			std::printf("	push(");
			ParserExprTokenPrint(operationStack[0]);
			std::printf(")\n");
		}
		else
		{
			std::printf("r0 = ");
			ParserExprTokenPrint(operationStack[0]);
			std::printf("\n");
		}
	}
	else
	{
		if(resOnStack == true)
		{
			std::printf("	push(r0)\n");
		}
	}

	//check bracket balance
	if(ParserExprBracketBalance != 0)
	{
		return Error(Error::Type::Syntax, "Invalid balance of parentheses");
	}

	return Error(Error::Type::Ok);
}