#pragma once

struct Error
{
	enum class Type
	{
		Ok,
		Lexical,
		Syntax,
	} type;

	Error(Error::Type t) { this->type = t; }
	Error(Error::Type t, const char* msg, ...);
};