SILANG - Silent language compiler

This compiler takes as input C-like language and generates abstract intermediate code
while describing what is going on with the compiler

This is just a demonstration on how to write a simple language compiler

Compiler

	1) - Takes text file as input

	1.5)(Optional) - Processes input with preprocessor

	2) - Divides the input into language atoms (Tokens)
	3) - Based on grammar, checks if sequence of tokens is valid
	4) - Based on valid sequence of tokens, generates intermediate code
	5) - Optimizes intermediate code and generates output program

Error.hpp/Error.cpp module
	
	Manages error printing and handling.

types.hpp module

	Defines utility typedefs, functions and constants.

Preprocessor.hpp/Preprocessor.cpp module

	Implements simple preprocessor for including files and defining constants.
	Has simple scanner and parser mashed into one module.

Scanner.hpp/Scanner.cpp module

	Takes input file, divides it into tokens and sends them to parser based on his needs.

Parser.hpp/Parser.cpp module

	Heart of the whole program.
	Is requesting tokens from scanner, checking their sequence and generating intermediate code.

Parser_Expr.cpp extension

	Implements processing of expressions by converting infix to postfix expression. 
	Generates intermediate code.

Codegen.hpp/Codegen.cpp module

	Provides utility functions to parser to generate intermediate code.

main.cpp module

	Start of the program. 

