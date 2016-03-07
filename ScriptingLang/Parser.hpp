#pragma once

#include "pch.hpp"
#include "Lexer.hpp"
#include "variant.hpp"
#include "SyntaxTree.hpp"

namespace Plang
{
	//Notation
	//Only prefix can be called with an unbounded, uncommented tuple
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
	struct Operator
	{
		std::string name;
		Notation notation;
		Association association;
		unsigned precedence; //lower numbers = higher precedence
	};

	struct Number
	{
		Number(InstructionType Type, Int I) : type(Type), value(I) { }
		Number(InstructionType Type, Float F) : type(Type), value(F) { }

		InstructionType type;
		union NumberValue
		{
			NumberValue(Int I) : i(I) { }
			NumberValue(Float F) : f(F) { }

			Int i;
			Float f;
		} value;
	};

	class Parser
	{
	public:
		Parser(const Lexer& Lex);
		~Parser() = default;

		std::map<std::string, Operator> operators; //predefined operators

		SyntaxTree syntaxTree;

		static Number ParseNumber(std::string Input);
		static String ParseString(std::string Input);
		static bool IsRegion(const Instruction& Instruction);

	protected:
		void CreateOperators();
		void CreateOperator(const std::string& Name, Notation Notat, Association Assoc, unsigned Precedence);
		void Reparent(SyntaxTreeNode* Node, SyntaxTreeNode* Parent); //Reconnect all children to parents

		void ParseToken(Lexer::TokenList::const_iterator& Token, const Lexer::TokenList& List);
		void ParseNextToken(Lexer::TokenList::const_iterator& Token, const Lexer::TokenList& List);

		void ParseOps(SyntaxTreeNode* Statement);

		std::stack<std::string> blocks; //block matching
		SyntaxTreeNode* parent;
	};
};
