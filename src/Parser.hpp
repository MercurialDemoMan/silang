#pragma once

#include "types.hpp"
#include "Scanner.hpp"
#include "Error.hpp"

#include <cstdio>
#include <string>
#include <vector>
#include <unordered_map>

/**
 * \brief parser
 */
class Parser
{
public:
	Parser() {}

	/**
	 * \brief main function -> generates output or throws an error
	 */
	Error parse(FILE* in, FILE* out);

	//useful enums
	enum class ReturnType { Byte, Int, Float, Pack, Void };
	enum class VarType    { Byte, Int, Float, Pack };

private:

	/**
	 * \brief parser states
	 */
	Error prog();
	Error defArgs();
	Error defArgsList();
	Error packItem();
	Error packItemList();
	Error args();
	Error argsList();
	Error funcType();
	Error body();
	Error expr(bool resOnStack = false);

	/**
	 * \brief utility for exiting scope
	 */
	void exitScope();

	//scanner for fetching tokens
	Scanner pScanner;

	//input/output
	FILE* pIn;
	FILE* pOut;

	//current token
	Scanner::Token pToken;

	//temp information about defining function
	std::string pCurrFunctionName;
	ReturnType  pCurrFunctionReturnType;
	u64         pCurrFunctionArgumentNum;

	//temp information about defining variable
	std::string pCurrVariableName;
	VarType     pCurrVariableType;

	//temp information about defining package
	std::string pCurrPackageName;

	//keep track of scope depth
	u64 pScope;

	/**
	 * \brief function table
	 */
	struct FunctionItem
	{
		struct Arg 
		{ 
			VarType     type;
			Arg() {} 
			Arg(VarType t) { type = t; }
		};
		//argument list
		std::vector<Arg> args;
		//return type
		ReturnType       retType;

		FunctionItem() {}
	};
	std::unordered_map<std::string, Parser::FunctionItem> pFunctions;


	/**
	 * \brief variable table
	 */
	struct VariableItem
	{
		//variable type
		VarType varType;
		//living scope
		u64     scope;

		VariableItem() {}
	};
	std::unordered_map<std::string, Parser::VariableItem> pVariables;
	/**
	 * \brief manage creating of variable
	 */
	Error createVar(std::string& name, Parser::VarType type, u64 scope);

	/**
	 * \brief structure table
	 */
	struct PackItem
	{
		std::unordered_map<std::string, VarType> items;
		PackItem() {}
	};
	std::unordered_map<std::string, Parser::PackItem> pPackages;
};