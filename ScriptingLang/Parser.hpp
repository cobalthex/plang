#pragma once

#include "pch.hpp"
#include "Lexer.hpp"
#include "Union.hpp"
#include "SyntaxTree.hpp"

namespace Plang
{
	enum class Notation
	{
		Prefix,
		Infix,
		Postfix
	};
	enum class Association
	{
		None,
		LeftToRight,
		RightToLeft
	};
	struct ParseExpression
	{
		std::string name;
		Notation notation;
		Association association;
		unsigned precedence; //lower numbers = higher precedence
	};

	struct Number
	{
		InstructionType type;
		Union<Int, Float> value;
	};

	class Parser
	{
	public:
		Parser(const Lexer& Lex);
		~Parser() = default;

		std::map<std::string, ParseExpression> predefinedExpressions;

		SyntaxTree syntaxTree;

		static Number ParseNumber(std::string Input);
		static String ParseString(std::string Input);

	protected:
		void CreatePredefinedExpressions();
		void Reparent(SyntaxTreeNode* Node, SyntaxTreeNode* Parent); //Reconnect all children to parent
	};
};