#include "Parser.hpp"

/**
 * ybrief debug messages
 */
static const char* ParserReturnTypeString[] =
{
	[(u32)Parser::ReturnType::Byte] = "ReturnType::Byte",
	[(u32)Parser::ReturnType::Int] = "ReturnType::Int",
	[(u32)Parser::ReturnType::Float] = "ReturnType::Float",
	[(u32)Parser::ReturnType::Void] = "ReturnType::Void",
	[(u32)Parser::ReturnType::Pack] = "ReturnType::Pack",
};

/**
 * \brief manage creating of variable
 */
Error Parser::createVar(std::string& name, Parser::VarType type, u64 scope)
{
	//if the identificator is not already in the functions table -> continute
	if(this->pFunctions.find(name) == this->pFunctions.end())
	{
		auto it = this->pVariables.find(name);

		//if there is no variable with the same name -> add it into the table
		if(it == this->pVariables.end())
		{
			this->pVariables[name] = VariableItem();
			this->pVariables[name].varType = type;
			this->pVariables[name].scope   = scope;
		}
		//TODO: allow global and local variables with same name
		else
		{
			return Error(Error::Type::Syntax, "Cannot redefine variable");
		}
	}
	else
	{
		return Error(Error::Type::Syntax, "Cannot define variable with same name as function");
	}

	return Error(Error::Type::Ok);
}

/**
 * \brief utility for exiting scope
 */
void Parser::exitScope()
{
	//remove all variables from variable table with current scope
	auto it = this->pVariables.cbegin();

	while(it != this->pVariables.cend())
	{
		if(it->second.scope == this->pScope)
		{
			it = this->pVariables.erase(it);
		}
		else
		{
			it++;
		}
	}
}

#define ParserProcessState(s) do { Error e = s; if(e.type != Error::Type::Ok) { return e; } } while(0)

/**
 * \brief implementation of <prog> rule
 * \note see ll.grammar
 */
//TODO: when calling another state, check its return value
Error Parser::prog()
{
	this->pToken = this->pScanner.getToken();

	/* <prog> -> FUNC ID ( <def-args> : <type> { <body> <prog> */
	if(this->pToken.type              == Scanner::TokenType::Keyword && 
	   this->pToken.attribute.keyword == Scanner::KeywordType::Func)
	{
		this->pToken = this->pScanner.getToken();
		//after FUNC must be ID
		if(this->pToken.type == Scanner::TokenType::Id)
		{
			//save function id
			this->pCurrFunctionName = this->pToken.attribute.litString;

			//check if function name isn't in variable table
			if(this->pVariables.find(this->pCurrFunctionName) == this->pVariables.end())
			{
				//check if function isn't already defined
				if(this->pFunctions.find(this->pCurrFunctionName) == this->pFunctions.end())
				{
					//create new function in symbol table
					//its' attributes will be set later
					this->pFunctions[this->pCurrFunctionName] = FunctionItem();
				}
				else
				{
					return Error(Error::Type::Syntax, "Cannot redefine function [%s]", this->pCurrFunctionName.c_str());
				}
			}
			else
			{	
				return Error(Error::Type::Syntax, "Cannot define function with same name as variable [%s]", this->pCurrFunctionName.c_str());
			}

			this->pToken = this->pScanner.getToken();
			//after ID must be LEFT BRACKET
			if(this->pToken.type == Scanner::TokenType::LeftBracket)
			{
				//after LEFT BRACKET are argument definitions
				//it will modify the function in function table
				ParserProcessState(this->defArgs());

				this->pToken = this->pScanner.getToken();
				//after argument definitions must be COLON
				if(this->pToken.type == Scanner::TokenType::Colon)
				{
					//acter COLON must function TYPE
					//it will modify the function in symbol table
					ParserProcessState(this->funcType());

					this->pToken = this->pScanner.getToken();
					//after function TYPE must be CURLY LEFT BRACKET
					if(this->pToken.type == Scanner::TokenType::LeftCurlyBracket)
					{
						std::printf("Create function \"%s\" with return value of type \"%s\" and %llu arguments\n", 
							this->pCurrFunctionName.c_str(), 
							ParserReturnTypeString[(u32)this->pCurrFunctionReturnType], 
							this->pCurrFunctionArgumentNum);

						//after CURLY LEFT BRACKET must be function BODY
						pScope++;
						std::printf("Change scope to %llu\n", pScope);
						ParserProcessState(this->body());
						this->exitScope();
						pScope--;
						std::printf("Change scope to %llu\n", pScope);

						//continue with the PROGRAM
						ParserProcessState(this->prog());
					}
					else
					{
						return Error(Error::Type::Syntax, "Expected \"{\" after function type [%s]", this->pCurrFunctionName.c_str());
					}
				}
				else
				{
					return Error(Error::Type::Syntax, "Expected \":\" after argument definitions [%s]", this->pCurrFunctionName.c_str());
				}
			}
			else
			{
				return Error(Error::Type::Syntax, "Expected \"(\" after function identificator [%s]", this->pCurrFunctionName.c_str());
			}
		}
		else
		{
			return Error(Error::Type::Syntax, "Expected identificator after \"func\" keyword");
		}
	}
	/* <prog> -> PACK ID { <pack-item> <prog> */
	else if(this->pToken.type              == Scanner::TokenType::Keyword &&
		    this->pToken.attribute.keyword == Scanner::KeywordType::Pack)
	{
		this->pToken = this->pScanner.getToken();
		//after PACK must be ID
		if(this->pToken.type == Scanner::TokenType::Id)
		{
			//save package item
			std::printf("Create new package %s\n", this->pToken.attribute.litString.c_str());
			this->pCurrPackageName = this->pToken.attribute.litString;

			//check if we haven't already defined package with the same name
			if(this->pPackages.find(this->pCurrPackageName) == this->pPackages.end())
			{
				//insert package into package table
				this->pPackages.insert({this->pCurrPackageName, PackItem()});

				this->pToken = this->pScanner.getToken();
				//after ID must be LEFT CURLY BRACK
				if(this->pToken.type == Scanner::TokenType::LeftCurlyBracket)
				{
					ParserProcessState(this->packItem());
					ParserProcessState(this->prog());
				}
				else
				{
					return Error(Error::Type::Syntax, "Expected \"{\" after package identificator [%s]", this->pCurrPackageName.c_str());
				}
			}
			else
			{
				return Error(Error::Type::Syntax, "Cannot redefine package with the same name [%s]", this->pCurrPackageName.c_str());
			}
		}
		else
		{
			return Error(Error::Type::Syntax, "Expected identificator after \"pack\" keyword");
		}
	}
	/* <prog> -> BYTE   ID ; <prog> */
	/* <prog> -> BYTE   ID = <expr> ; <prog> */
	/* <prog> -> INT    ID ; <prog> */
	/* <prog> -> INT    ID = <expr> ; <prog> */
	/* <prog> -> FLOAT  ID ; <prog> */
	/* <prog> -> FLOAT  ID = <expr> ; <prog> */
	else if(this->pToken.type == Scanner::TokenType::Keyword)
	{
		if(this->pToken.attribute.keyword == Scanner::KeywordType::Byte  ||
		   this->pToken.attribute.keyword == Scanner::KeywordType::Int   ||
		   this->pToken.attribute.keyword == Scanner::KeywordType::Float)
		{
			switch(this->pToken.attribute.keyword)
			{
				case Scanner::KeywordType::Byte:   { this->pCurrVariableType = Parser::VarType::Byte;   break; }
				case Scanner::KeywordType::Int:    { this->pCurrVariableType = Parser::VarType::Int;    break; }
				case Scanner::KeywordType::Float:  { this->pCurrVariableType = Parser::VarType::Float;  break; }
				default: { break; }
			}

			this->pToken = this->pScanner.getToken();
			//after TYPE must be ID
			if(this->pToken.type == Scanner::TokenType::Id)
			{
				//save variable name
				this->pCurrVariableName = this->pToken.attribute.litString;

				this->pToken = this->pScanner.getToken();
				/* <prog> -> BYTE/INT/FLOAT/STRING ID ; <prog> */
				if(this->pToken.type == Scanner::TokenType::SemiColon)
				{
					//add variable into variable pool
					this->createVar(this->pCurrVariableName, this->pCurrVariableType, 0);

					switch(this->pCurrVariableType)
					{
						case Parser::VarType::Byte:   { std::printf("Define new variable \"%s\" of type \"byte\" in scope %llu\n",   this->pCurrVariableName.c_str(), this->pScope); break; }
						case Parser::VarType::Int:    { std::printf("Define new variable \"%s\" of type \"int\" in scope %llu\n",    this->pCurrVariableName.c_str(), this->pScope); break; }
						case Parser::VarType::Float:  { std::printf("Define new variable \"%s\" of type \"float\" in scope %llu\n",  this->pCurrVariableName.c_str(), this->pScope); break; }
						default: { break; }
					}

					ParserProcessState(this->prog());
				}
				/* <prog> -> BYTE/INT/FLOAT/STRING ID = <expr> ; <prog> */
				else if(this->pToken.type == Scanner::TokenType::Assign)
				{
					this->pToken = this->pScanner.getToken();
					ParserProcessState(this->expr());

					if(this->pToken.type == Scanner::TokenType::SemiColon)
					{
						//add variable into variable pool
						this->createVar(this->pCurrVariableName, this->pCurrVariableType, 0);

						switch(this->pCurrVariableType)
						{
							case Parser::VarType::Byte:   { std::printf("Define new variable \"%s\" of type \"byte\" and initialize with r0 in scope %llu\n",   this->pCurrVariableName.c_str(), this->pScope); break; }
							case Parser::VarType::Int:    { std::printf("Define new variable \"%s\" of type \"int\" and initialize with r0 in scope %llu\n",    this->pCurrVariableName.c_str(), this->pScope); break; }
							case Parser::VarType::Float:  { std::printf("Define new variable \"%s\" of type \"float\" and initialize with r0 in scope %llu\n",  this->pCurrVariableName.c_str(), this->pScope); break; }
							default: { break; }
						}

						ParserProcessState(this->prog());
					}
					else
					{
						return Error(Error::Type::Syntax, "Expected \";\" after expression");
					}
				}
				else
				{
					return Error(Error::Type::Syntax, "Expected \";\" or assignment after variable declaration");
				}
			}
			else
			{
				return Error(Error::Type::Syntax, "Expected identificator after type");
			}
		}
		else
		{
			return Error(Error::Type::Syntax, "Unexpected keyword in the global scope");
		}
	}
	/* <prog> -> ID     ID ; <prog> */
	else if(this->pToken.type == Scanner::TokenType::Id)
	{
		return Error(Error::Type::Syntax, "NYI");
	}
	/* <prog> -> EOF */
	else if(this->pToken.type == Scanner::TokenType::Eof)
	{
		return Error(Error::Type::Ok, "Reached the end of the source file");
	}
	else
	{
		return Error(Error::Type::Syntax, "Unexpected token in the global scope");
	}

	return Error(Error::Type::Ok);
}
/**
 * \brief implementation of <def-args> rule
 * \note see ll.grammar
 */
Error Parser::defArgs()
{
	//reset current function
	this->pCurrFunctionArgumentNum = 0;

	this->pToken = this->pScanner.getToken();

	/* <def-args> -> BYTE   ID <def-args-list> */
	/* <def-args> -> INT    ID <def-args-list> */
	/* <def-args> -> FLOAT  ID <def-args-list> */
	/* <def-args> -> STRING ID <def-args-list> */
	if(this->pToken.type == Scanner::TokenType::Keyword)
	{
		if(this->pToken.attribute.keyword == Scanner::KeywordType::Byte  ||
	       this->pToken.attribute.keyword == Scanner::KeywordType::Int   ||
	   	   this->pToken.attribute.keyword == Scanner::KeywordType::Float)
		{
			switch(this->pToken.attribute.keyword)
			{
				case Scanner::KeywordType::Byte:   { this->pCurrVariableType = Parser::VarType::Byte;   break; }
				case Scanner::KeywordType::Int:    { this->pCurrVariableType = Parser::VarType::Int;    break; }
				case Scanner::KeywordType::Float:  { this->pCurrVariableType = Parser::VarType::Float;  break; }
				default: { break; }
			}

			this->pToken = this->pScanner.getToken();
			//after argument TYPE must be argument ID
			if(this->pToken.type == Scanner::TokenType::Id)
			{
				//add argument into symbol table function
				this->pFunctions[this->pCurrFunctionName].args.emplace_back(this->pCurrVariableType);
				//add argument into local variable pool
				this->createVar(this->pToken.attribute.litString, this->pCurrVariableType, this->pScope + 1);

				switch(this->pCurrVariableType)
				{
					case Parser::VarType::Byte:   { std::printf("Define new argument \"%s\" of type \"byte\" in scope %llu\n",   this->pToken.attribute.litString.c_str(), this->pScope + 1); break; }
					case Parser::VarType::Int:    { std::printf("Define new argument \"%s\" of type \"int\" in scope %llu\n",    this->pToken.attribute.litString.c_str(), this->pScope + 1); break; }
					case Parser::VarType::Float:  { std::printf("Define new argument \"%s\" of type \"float\" in scope %llu\n",  this->pToken.attribute.litString.c_str(), this->pScope + 1); break; }
					default: { break; }
				}

				this->pCurrFunctionArgumentNum++;
				//process next argument definition or end
				ParserProcessState(this->defArgsList());
			}
			else
			{
				return Error(Error::Type::Syntax, "Expected identificator after type when defining function argument");
			}
		}
		else 
		{
			return Error(Error::Type::Syntax, "Expected type when defining function argument");
		}
	}
	/* <def-args> -> ID ID <def-args-list> */
	/*else if(this->pToken.type == Scanner::TokenType::Id)
	{
		//save structure name
		std::string structName = this->pToken.attribute.litString;
		
		this->pToken = this->pScanner.getToken();
		//after argument TYPE must be argument ID
		if(this->pToken.type == Scanner::TokenType::Id)
		{
			//add argument into symbol table function
			this->pFunctions[this->pCurrFunctionName].args.emplace_back(Parser::VarType::Struct);
			//add argument into local variable pool
			this->createVar(this->pToken.attribute.litString, Parser::VarType::Struct, this->pScope + 1);

			std::printf("Define argument of type \"%s\" and name \"%s\"\n", structName.c_str(), this->pToken.attribute.litString.c_str());

			this->pCurrFunctionArgumentNum++;
			//process next argument definition or end
			this->defArgsList();
		}
		else
		{
			throw std::runtime_error("Expected identificator after type\n");
		}
	}*/
	/* <def-args> -> ) */
	else if(this->pToken.type == Scanner::TokenType::RightBracket)
	{
		return Error(Error::Type::Ok);
	}
	else
	{
		return Error(Error::Type::Syntax, "Unexpected symbol near function argument definition e");
	}

	return Error(Error::Type::Ok);
}
/**
 * \brief implementation of <def-args-list> rule
 * \note see ll.grammar
 */
//TODO: check names of packages, functions and variables
Error Parser::defArgsList()
{
	this->pToken = this->pScanner.getToken();
	
	/* <def-args-list> -> , BYTE   ID <def-args-list> */
	/* <def-args-list> -> , INT    ID <def-args-list> */
	/* <def-args-list> -> , FLOAT  ID <def-args-list> */
	/* <def-args-list> -> , ID     ID <def-args-list> */
	if(this->pToken.type == Scanner::TokenType::Comma)
	{
		this->pToken = this->pScanner.getToken();

		/* <def-args-list> -> , BYTE   ID <def-args-list> */
		/* <def-args-list> -> , INT    ID <def-args-list> */
		/* <def-args-list> -> , FLOAT  ID <def-args-list> */
		if(this->pToken.type == Scanner::TokenType::Keyword)
		{
			if(this->pToken.attribute.keyword == Scanner::KeywordType::Byte  ||
		       this->pToken.attribute.keyword == Scanner::KeywordType::Int   ||
		   	   this->pToken.attribute.keyword == Scanner::KeywordType::Float)
			{
				switch(this->pToken.attribute.keyword)
				{
					case Scanner::KeywordType::Byte:   { this->pCurrVariableType = Parser::VarType::Byte;   break; }
					case Scanner::KeywordType::Int:    { this->pCurrVariableType = Parser::VarType::Int;    break; }
					case Scanner::KeywordType::Float:  { this->pCurrVariableType = Parser::VarType::Float;  break; }
					default: { break; }
				}

				this->pToken = this->pScanner.getToken();
				//after argument TYPE must be argument ID
				if(this->pToken.type == Scanner::TokenType::Id)
				{
					//add argument into symbol table function
					this->pFunctions[this->pCurrFunctionName].args.emplace_back(this->pCurrVariableType);
					//add argument into local variable pool
					this->createVar(this->pToken.attribute.litString, this->pCurrVariableType, this->pScope + 1);

					switch(this->pCurrVariableType)
					{
						case Parser::VarType::Byte:   { std::printf("Define new argument \"%s\" of type \"byte\" and initialize with r0 in scope %llu\n",   this->pToken.attribute.litString.c_str(), this->pScope + 1); break; }
						case Parser::VarType::Int:    { std::printf("Define new argument \"%s\" of type \"int\" and initialize with r0 in scope %llu\n",    this->pToken.attribute.litString.c_str(), this->pScope + 1); break; }
						case Parser::VarType::Float:  { std::printf("Define new argument \"%s\" of type \"float\" and initialize with r0 in scope %llu\n",  this->pToken.attribute.litString.c_str(), this->pScope + 1); break; }
						default: { break; }
					}

					this->pCurrFunctionArgumentNum++;
					//process next argument definition or end
					ParserProcessState(this->defArgsList());
				}
				else
				{
					return Error(Error::Type::Syntax, "Expected identificator after type");
				}
			}
			else 
			{
				return Error(Error::Type::Syntax, "Expected type when defining function argument");
			}
		}

		/* <def-args-list> -> , ID ID <def-args-list> */
		else if(this->pToken.type == Scanner::TokenType::Id)
		{
			return Error(Error::Type::Syntax, "Creating packages as arguments is not yet implemented");
			//save structure name
			/*std::string structName = this->pToken.attribute.litString;

			this->pToken = this->pScanner.getToken();
			//after argument TYPE must be argument ID
			if(this->pToken.type == Scanner::TokenType::Id)
			{
				//add argument into symbol table function
				this->pFunctions[this->pCurrFunctionName].args.emplace_back(Parser::VarType::Struct);
				//add argument into local variable pool
				this->createVar(this->pToken.attribute.litString, Parser::VarType::Struct, this->pScope + 1);
	
				std::printf("Define argument of type \"%s\" and name \"%s\"\n", structName.c_str(), this->pToken.attribute.litString.c_str());

				this->pCurrFunctionArgumentNum++;
				//process next argument definition or end
				this->defArgsList();
			}
			else
			{
				throw std::runtime_error("Expected identificator after type\n");
			}*/
		}
		else
		{
			return Error(Error::Type::Syntax, "Unexpected symbol near function argument definition c");
		}
	}
	else if(this->pToken.type == Scanner::TokenType::RightBracket)
	{
		return Error(Error::Type::Ok);
	}
	else
	{
		return Error(Error::Type::Syntax, "Unexpected symbol near function argument definition a");
	}

	return Error(Error::Type::Ok);
}
/**
 * \brief implementation of <pack-item> rule
 * \note see ll.grammar
 */
Error Parser::packItem()
{
	this->pToken = this->pScanner.getToken();
	/* <pack-item> -> BYTE ID <pack-item-list> */
	/* <pack-item> -> INT ID <pack-item-list> */
	/* <pack-item> -> FLOAT ID <pack-item-list> */
	if(this->pToken.type == Scanner::TokenType::Keyword)
	{
		if(this->pToken.attribute.keyword == Scanner::KeywordType::Byte ||
		   this->pToken.attribute.keyword == Scanner::KeywordType::Int  ||
		   this->pToken.attribute.keyword == Scanner::KeywordType::Float)
		{
			switch(this->pToken.attribute.keyword)
			{
				case Scanner::KeywordType::Byte:   { this->pCurrVariableType = Parser::VarType::Byte;   break; }
				case Scanner::KeywordType::Int:    { this->pCurrVariableType = Parser::VarType::Int;    break; }
				case Scanner::KeywordType::Float:  { this->pCurrVariableType = Parser::VarType::Float;  break; }
				default: { break; }
			}

			this->pToken = this->pScanner.getToken();
			//after package item TYPE must be package item ID
			if(this->pToken.type == Scanner::TokenType::Id)
			{
				//check if there are unique names for each package item
				auto it = this->pPackages[this->pCurrPackageName].items.find(this->pToken.attribute.litString);
				if(it == this->pPackages[this->pCurrPackageName].items.end())
				{
					std::printf("Add item into package \"%s\" <- \"%s\"\n", this->pCurrPackageName.c_str(), this->pToken.attribute.litString.c_str());

					//add item into the package
					this->pPackages[this->pCurrPackageName].items.insert({this->pToken.attribute.litString, this->pCurrVariableType});

					//scan for other items
					ParserProcessState(this->packItemList());
				}
				else
				{
					return Error(Error::Type::Syntax, "Cannot have same identificator for two package items [%s]", this->pToken.attribute.litString.c_str());
				}
			}
			else
			{
				return Error(Error::Type::Syntax, "Expected identificator after package item type");
			}
		}
		else
		{
			return Error(Error::Type::Syntax, "Expected item type when defining package item");
		}
	}
	else if(this->pToken.type == Scanner::TokenType::RightCurlyBracket)
	{
		return Error(Error::Type::Syntax, "Expected at least one item in package");
	}
	else
	{
		return Error(Error::Type::Syntax, "Expected item type when defining package item");
	}

	return Error(Error::Type::Ok);
}
/**
 * \brief implementation of <pack-item-list> rule
 * \note see ll.grammar
 */
Error Parser::packItemList()
{
	this->pToken = this->pScanner.getToken();
	//after package item ID must be SEMICOLON
	if(this->pToken.type == Scanner::TokenType::SemiColon)
	{
		this->pToken = this->pScanner.getToken();
		/* <pack-item-list> -> ; } */
		if(this->pToken.type == Scanner::TokenType::RightCurlyBracket)
		{
			return Error(Error::Type::Ok);
		}
		else
		{
			/* <pack-item-list> -> ; BYTE ID <pack-item-list> */
			/* <pack-item-list> -> ; INT ID <pack-item-list> */
			/* <pack-item-list> -> ; FLOAT ID <pack-item-list> */
			if(this->pToken.type == Scanner::TokenType::Keyword)
			{
				if(this->pToken.attribute.keyword == Scanner::KeywordType::Byte ||
				   this->pToken.attribute.keyword == Scanner::KeywordType::Int  ||
				   this->pToken.attribute.keyword == Scanner::KeywordType::Float)
				{

					switch(this->pToken.attribute.keyword)
					{
						case Scanner::KeywordType::Byte:   { this->pCurrVariableType = Parser::VarType::Byte;   break; }
						case Scanner::KeywordType::Int:    { this->pCurrVariableType = Parser::VarType::Int;    break; }
						case Scanner::KeywordType::Float:  { this->pCurrVariableType = Parser::VarType::Float;  break; }
						default: { break; }
					}

					this->pToken = this->pScanner.getToken();
					//after package item TYPE must be package item ID
					if(this->pToken.type == Scanner::TokenType::Id)
					{
						//check if there are unique names for each package item
						auto it = this->pPackages[this->pCurrPackageName].items.find(this->pToken.attribute.litString);
						if(it == this->pPackages[this->pCurrPackageName].items.end())
						{
							std::printf("Add item into package \"%s\" <- \"%s\"\n", this->pCurrPackageName.c_str(), this->pToken.attribute.litString.c_str());

							//add item into the package
							this->pPackages[this->pCurrPackageName].items.insert({this->pToken.attribute.litString, this->pCurrVariableType});

							//scan for other items
							ParserProcessState(this->packItemList());
						}
						else
						{
							return Error(Error::Type::Syntax, "Cannot have same identificator for two package items [%s]", this->pToken.attribute.litString.c_str());
						}
					}
					else
					{
						return Error(Error::Type::Syntax, "Expected identificator after package item type [%s]", this->pCurrPackageName.c_str());
					}
				}
				else
				{
					return Error(Error::Type::Syntax, "Expected item type when defining package item [%s]", this->pCurrPackageName.c_str());
				}
			}
			else
			{
				return Error(Error::Type::Syntax, "Expected item type when defining package item [%s]", this->pCurrPackageName.c_str());
			}
		}
	}
	else
	{
		return Error(Error::Type::Syntax, "Expected \";\" after package item identificator [%s]", this->pCurrPackageName.c_str());
	}

	return Error(Error::Type::Ok);
}
/**
 * \brief implementation of <args> rule
 * \note see ll.grammar
 */
Error Parser::args()
{
	this->pToken = this->pScanner.getToken();

	if(this->pToken.type != Scanner::TokenType::RightBracket)
	{
		ParserProcessState(this->expr(true));
		ParserProcessState(this->argsList());
	}

	return Error::Type::Ok;
}
/**
 * \brief implementation of <args-list> rule
 * \note see ll.grammar
 */
Error Parser::argsList()
{
	if(this->pToken.type == Scanner::TokenType::Comma)
	{
		this->pToken = this->pScanner.getToken();

		ParserProcessState(this->expr(true));
		ParserProcessState(this->argsList());
	}
	else if(this->pToken.type == Scanner::TokenType::RightBracket)
	{
		return Error(Error::Type::Ok);
	}
	else
	{
		return Error(Error::Type::Syntax, "Expected \",\" after function argument");
	}

	return Error(Error::Type::Ok);
}
/**
 * \brief implementation of <func-type> rule
 * \note see ll.grammar
 */
Error Parser::funcType()
{
	this->pToken = this->pScanner.getToken();

	if(this->pToken.type == Scanner::TokenType::Keyword)
	{
		if(this->pToken.attribute.keyword == Scanner::KeywordType::Byte)
		{
			this->pCurrFunctionReturnType = Parser::ReturnType::Byte;
		}
		else if(this->pToken.attribute.keyword == Scanner::KeywordType::Int)
		{
			this->pCurrFunctionReturnType = Parser::ReturnType::Int;
		}
		else if(this->pToken.attribute.keyword == Scanner::KeywordType::Float)
		{
			this->pCurrFunctionReturnType = Parser::ReturnType::Float;
		}
		else if(this->pToken.attribute.keyword == Scanner::KeywordType::Void)
		{
			this->pCurrFunctionReturnType = Parser::ReturnType::Void;
		}
		else 
		{
			return Error(Error::Type::Syntax, "Unexpected function return type");
		}
	}
	/*else if(this->pToken.type == Scanner::TokenType::Id)
	{
		this->pCurrFunctionReturnType = Parser::ReturnType::Pack;
	}*/
	else
	{
		return Error(Error::Type::Syntax, "Unexpected function return type");
	}

	//modify the return value of the function
	this->pFunctions[this->pCurrFunctionName].retType = this->pCurrFunctionReturnType;

	return Error(Error::Type::Ok);
}
/**
 * \brief implementation of <body> rule
 * \note see ll.grammar
 */
Error Parser::body()
{
	this->pToken = this->pScanner.getToken();

	/* <body> -> BYTE   ID ; <body> */
	/* <body> -> BYTE   ID = <expr> ; <body> */
	/* <body> -> INT    ID ; <body> */
	/* <body> -> INT    ID = <expr> ; <body> */
	/* <body> -> FLOAT  ID ; <body> */
	/* <body> -> FLOAT  ID = <expr> ; <body> */
	/* <body> -> RETURN <expr> ; <body> */
	/* <body> -> IF    ( <expr> ) { <body> */
	/* <body> -> WHILE ( <expr> ) { <body> */
	/* <body> -> FOR   ( <expr> ; <expr> ; <expr> ) : { <body */
	if(this->pToken.type == Scanner::TokenType::Keyword)
	{
		/* <body> -> BYTE   ID ; <body> */
		/* <body> -> BYTE   ID = <expr> ; <body> */
		/* <body> -> INT    ID ; <body> */
		/* <body> -> INT    ID = <expr> ; <body> */
		/* <body> -> FLOAT  ID ; <body> */
		/* <body> -> FLOAT  ID = <expr> ; <body> */
		if(this->pToken.attribute.keyword == Scanner::KeywordType::Byte  ||
		   this->pToken.attribute.keyword == Scanner::KeywordType::Int   ||
		   this->pToken.attribute.keyword == Scanner::KeywordType::Float)
		{
			switch(this->pToken.attribute.keyword)
			{
				case Scanner::KeywordType::Byte:   { pCurrVariableType = Parser::VarType::Byte;   break; }
				case Scanner::KeywordType::Int:    { pCurrVariableType = Parser::VarType::Int;    break; }
				case Scanner::KeywordType::Float:  { pCurrVariableType = Parser::VarType::Float;  break; }
				default: { break; }
			}

			this->pToken = this->pScanner.getToken();
			//after TYPE must be ID
			if(this->pToken.type == Scanner::TokenType::Id)
			{
				//save variable name
				this->pCurrVariableName = this->pToken.attribute.litString;

				this->pToken = this->pScanner.getToken();
				/* <body> -> BYTE/INT/FLOAT/STRING ID ; <body> */
				if(this->pToken.type == Scanner::TokenType::SemiColon)
				{
					//add variable into variable pool
					this->createVar(this->pCurrVariableName, this->pCurrVariableType, this->pScope);

					switch(this->pCurrVariableType)
					{
						case Parser::VarType::Byte:   { std::printf("Define new variable \"%s\" of type \"byte\" in scope %llu\n",   this->pCurrVariableName.c_str(), this->pScope); break; }
						case Parser::VarType::Int:    { std::printf("Define new variable \"%s\" of type \"int\" in scope %llu\n",    this->pCurrVariableName.c_str(), this->pScope); break; }
						case Parser::VarType::Float:  { std::printf("Define new variable \"%s\" of type \"float\" in scope %llu\n",  this->pCurrVariableName.c_str(), this->pScope); break; }
						default: { break; }
					}

					ParserProcessState(this->body());
				}
				/* <body> -> BYTE/INT/FLOAT/STRING ID = <expr> ; <body> */
				else if(this->pToken.type == Scanner::TokenType::Assign)
				{
					this->pToken = this->pScanner.getToken();
					ParserProcessState(this->expr());

					if(this->pToken.type == Scanner::TokenType::SemiColon)
					{
						//add variable into variable pool
						this->createVar(this->pCurrVariableName, this->pCurrVariableType, this->pScope);

						switch(this->pCurrVariableType)
						{
							case Parser::VarType::Byte:   { std::printf("Define new variable \"%s\" of type \"byte\" and initialize with r0 in scope %llu\n",   this->pCurrVariableName.c_str(), this->pScope); break; }
							case Parser::VarType::Int:    { std::printf("Define new variable \"%s\" of type \"int\" and initialize with r0 in scope %llu\n",    this->pCurrVariableName.c_str(), this->pScope); break; }
							case Parser::VarType::Float:  { std::printf("Define new variable \"%s\" of type \"float\" and initialize with r0 in scope %llu\n",  this->pCurrVariableName.c_str(), this->pScope); break; }
							default: { break; }
						}

						ParserProcessState(this->body());
					}
					else
					{
						return Error(Error::Type::Syntax, "Expected \";\" after expression");
					}
				}
				else
				{
					return Error(Error::Type::Syntax, "Expected \";\" or assignment after variable declaration");
				}
			}
			else
			{
				return Error(Error::Type::Syntax, "Expected identificator after type");
			}
		}
		/* <body> -> RETURN <expr> ; <body> */
		else if(this->pToken.attribute.keyword == Scanner::KeywordType::Return)
		{
			this->pToken = this->pScanner.getToken();

			if(this->pToken.type == Scanner::TokenType::SemiColon)
			{
				std::printf("Return from function \"%s\"\n", this->pCurrFunctionName.c_str());

				ParserProcessState(this->body());
			}
			else
			{
				ParserProcessState(this->expr());

				if(this->pToken.type == Scanner::TokenType::SemiColon)
				{
					std::printf("Return from function \"%s\" with r0\n", this->pCurrFunctionName.c_str());

					ParserProcessState(this->body());
				}
				else
				{
					return Error(Error::Type::Syntax, "Expected \";\" after return");
				}
			}
		}
		/* <body> -> IF ( <expr> ) : { <body>  */
		else if(this->pToken.attribute.keyword == Scanner::KeywordType::If)
		{
			this->pToken = this->pScanner.getToken();
			//after IF must be LEFT BRACKET
			if(this->pToken.type == Scanner::TokenType::LeftBracket)
			{
				this->pToken = this->pScanner.getToken();
				ParserProcessState(this->expr());

				if(this->pToken.type == Scanner::TokenType::RightBracket)
				{
					this->pToken = this->pScanner.getToken();
					//after RIGHT BRACKET must be LEFT CURLY BRACKET
					if(this->pToken.type == Scanner::TokenType::LeftCurlyBracket)
					{
						std::printf("Generate if head\n");

						pScope++;
						std::printf("Change scope to %llu\n", pScope);
						ParserProcessState(this->body());
						this->exitScope();
						pScope--;
						std::printf("Change scope to %llu\n", pScope);
						ParserProcessState(this->body());
					}
					else
					{
						return Error(Error::Type::Syntax, "Expected \"{\" after \")\"");
					}
				}
				else
				{
					return Error(Error::Type::Syntax, "Expected \")\" after expression");
				}
			}
			else
			{
				return Error(Error::Type::Syntax, "Expected \"(\" after if");
			}
		}
		/* <body> -> ELSE IF ( <expr> ) { <body> <body> */
		/* <body> -> ELSE { <body> <body> */
		else if(this->pToken.attribute.keyword == Scanner::KeywordType::Else)
		{
			this->pToken = this->pScanner.getToken();

			/* <body> -> ELSE IF ( <expr> ) { <body> <body> */
			if(this->pToken.type == Scanner::TokenType::Keyword)
			{
				if(this->pToken.attribute.keyword == Scanner::KeywordType::If)
				{
					this->pToken = this->pScanner.getToken();
					//after IF must be LEFT BRACKET
					if(this->pToken.type == Scanner::TokenType::LeftBracket)
					{
						this->pToken = this->pScanner.getToken();
						ParserProcessState(this->expr());

						if(this->pToken.type == Scanner::TokenType::RightBracket)
						{
							this->pToken = this->pScanner.getToken();
							//after RIGHT BRACKET must be LEFT CURLY BRACKET
							if(this->pToken.type == Scanner::TokenType::LeftCurlyBracket)
							{
								std::printf("Generate else if head\n");

								pScope++;
								std::printf("Change scope to %llu\n", pScope);
								ParserProcessState(this->body());
								this->exitScope();
								pScope--;
								std::printf("Change scope to %llu\n", pScope);
								ParserProcessState(this->body());
							}
							else
							{
								return Error(Error::Type::Syntax, "Expected \"{\" after \")\"");
							}
						}
						else
						{
							return Error(Error::Type::Syntax, "Expected \")\" after expression");
						}
					}
					else
					{
						return Error(Error::Type::Syntax, "Expected \"(\" after if");
					}
				}
				else
				{
					return Error(Error::Type::Syntax, "Unexpected symbol after else");
				}
			}
			else if(this->pToken.type == Scanner::TokenType::LeftCurlyBracket)
			{
				std::printf("Generate else head\n");

				pScope++;
				std::printf("Change scope to %llu\n", pScope);
				ParserProcessState(this->body());
				this->exitScope();
				pScope--;
				std::printf("Change scope to %llu\n", pScope);
				ParserProcessState(this->body());
			}
			else
			{
				return Error(Error::Type::Syntax, "Unexpected symbol after else");
			}
		}
		/* <body> -> WHILE ( <expr> ) : { <body> */
		else if(this->pToken.attribute.keyword == Scanner::KeywordType::While)
		{
			this->pToken = this->pScanner.getToken();
			//after WHILE must be LEFT BRACKET
			if(this->pToken.type == Scanner::TokenType::LeftBracket)
			{
				this->pToken = this->pScanner.getToken();
				ParserProcessState(this->expr());

				if(this->pToken.type == Scanner::TokenType::RightBracket)
				{
					this->pToken = this->pScanner.getToken();
					//after RIGHT BRACKET must be LEFT CURLY BRACKET
					if(this->pToken.type == Scanner::TokenType::LeftCurlyBracket)
					{
						std::printf("Generate while\n");
						
						pScope++;
						std::printf("Change scope to %llu\n", pScope);
						ParserProcessState(this->body());
						this->exitScope();
						pScope--;
						std::printf("Change scope to %llu\n", pScope);
						ParserProcessState(this->body());
					}
					else
					{
						return Error(Error::Type::Syntax, "Expected \"{\" after \")\"");
					}
				}
				else
				{
					return Error(Error::Type::Syntax, "Expected \")\" after expression");
				}
			}
			else
			{
				return Error(Error::Type::Syntax, "Expected \"(\" after while");
			}
		}
		/* <body> -> FOR ( <expr> ; <expr> ; <expr> ) : { <body */
		else if(this->pToken.attribute.keyword == Scanner::KeywordType::For)
		{
			this->pToken = this->pScanner.getToken();
			//after FOR must be LEFT BRACKET
			if(this->pToken.type == Scanner::TokenType::LeftBracket)
			{
				this->pToken = this->pScanner.getToken();
				ParserProcessState(this->expr());

				//after first expression must be SEMICOLON
				if(this->pToken.type == Scanner::TokenType::SemiColon)
				{
					this->pToken = this->pScanner.getToken();
					ParserProcessState(this->expr());

					//after second expression must be SEMICOLON
					if(this->pToken.type == Scanner::TokenType::SemiColon)
					{
						this->pToken = this->pScanner.getToken();
						ParserProcessState(this->expr());

						//after third expression must be RIGHT BRACET
						if(this->pToken.type == Scanner::TokenType::RightBracket)
						{
							this->pToken = this->pScanner.getToken();
							//after RIGHT BRACKET must be LEFT CURLY BRACKET
							if(this->pToken.type == Scanner::TokenType::LeftCurlyBracket)
							{
								std::printf("Generate for\n");

								pScope++;
								std::printf("Change scope to %llu\n", pScope);
								ParserProcessState(this->body());
								this->exitScope();
								pScope--;
								std::printf("Change scope to %llu\n", pScope);
								ParserProcessState(this->body());
							}
							else
							{
								return Error(Error::Type::Syntax, "Expected \"{\" after \")\"");
							}
						}
						else
						{
							return Error(Error::Type::Syntax, "Expected \")\" after third expression");
						}
					}
					else
					{
						return Error(Error::Type::Syntax, "Expected \";\" after second expression");
					}
				}
				else
				{
					return Error(Error::Type::Syntax, "Expected \";\" after first expression");
				}
			}
			else
			{
				return Error(Error::Type::Syntax, "Expected \"(\" after for");
			}
		}
		else 
		{
			return Error(Error::Type::Syntax, "Unexpected keyword in function body");
		}
	}
	/* <body> -> } */
	else if(this->pToken.type == Scanner::TokenType::RightCurlyBracket)
	{
		return Error(Error::Type::Ok);
	}
	/* <body> -> ID ID ; <body> */
	/* <body> -> ID = <expr> ; <body> */
	else if(this->pToken.type == Scanner::TokenType::Id)
	{
		//save the first identificator
		this->pCurrVariableName = this->pToken.attribute.litString;

		this->pToken = this->pScanner.getToken();

		/* <body> -> ID = <expr> ; <body> */
		if(this->pToken.type == Scanner::TokenType::Assign)
		{
			//check if the ID exists
			if(this->pVariables.find(this->pCurrVariableName) == this->pVariables.end())
			{
				return Error(Error::Type::Syntax, "Cannot assign expression to a undefined variable");
			}

			//evaluate expression
			this->pToken = this->pScanner.getToken();
			ParserProcessState(this->expr());

			//after expression must be SEMICOLON
			if(this->pToken.type == Scanner::TokenType::SemiColon)
			{
				std::printf("Assign variable \"%s\" a new value r0\n", pCurrVariableName.c_str());
				ParserProcessState(this->body());
			}
			else
			{
				return Error(Error::Type::Syntax, "Expected \";\" after expression");
			}
		}
		/* <body> -> ID ( <args> ; <body> */
		else if(this->pToken.type == Scanner::TokenType::LeftBracket)
		{
			ParserProcessState(this->args());

			this->pToken = this->pScanner.getToken();
			//after args must be SEMICOLON
			if(this->pToken.type == Scanner::TokenType::SemiColon)
			{

				std::printf("Call function \"%s\"\n", pCurrVariableName.c_str());
				ParserProcessState(this->body());
			}
			else
			{
				return Error(Error::Type::Syntax, "Expected \";\" after arguments");
			}
		}
		/* <body> -> ID ID ; <body> */
		else if(this->pToken.type == Scanner::TokenType::Id)
		{
			if(this->pPackages.find(this->pCurrVariableName) != this->pPackages.end())
			{
				//add variable into variable pool
				this->createVar(this->pCurrVariableName, Parser::VarType::Pack, this->pScope);

				std::printf("Declare variable \"%s\" of type \"%s\"\n", this->pToken.attribute.litString.c_str(), this->pCurrVariableName.c_str());

				this->pToken = this->pScanner.getToken();
				//after ID must be SEMICOLON
				if(this->pToken.type == Scanner::TokenType::SemiColon)
				{
					ParserProcessState(this->body());
				}
				else
				{
					return Error(Error::Type::Syntax, "Expected \";\" after identificator");
				}
			}
			else
			{
				return Error(Error::Type::Syntax, "Using undefined package");
			}
		}
		else
		{
			return Error(Error::Type::Syntax, "Unexpected symbol after identificator");
		}
	}
	else
	{
		return Error(Error::Type::Syntax, "Unexpected symbol while processing function body");
	}

	return Error(Error::Type::Ok);
}

/**
 * \brief main function -> generates output or throws an error
 */
Error Parser::parse(FILE* in, FILE* out)
{
	this->pIn  = in;
	this->pOut = out;

	this->pScanner = Scanner(pIn);

	this->pScope   = 0;

	return this->prog();
}