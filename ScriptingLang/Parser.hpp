#pragma once

#include "pch.hpp"
#include "Lexer.hpp"
#include "variant.hpp"
#include "SyntaxTree.hpp"
#include "types.hpp"

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

	class ParserException : public std::exception
	{
	public:
		ParserException(const std::string& Message, const std::string& Token, const Location& Location)
			: message(Message), token(Token), location(Location) { }

		std::string token;
		Location location;

		inline virtual const char* what() const noexcept override { return message.c_str(); }

	protected:
		std::string message;
	};

	class Parser
	{
	public:
		Parser() = default;
		Parser(const Lexer::TokenList& Tokens);
		Parser(const Parser& Parser)
			: syntaxTree(Parser.syntaxTree), operators(Parser.operators)
		{
			Reparent(&syntaxTree.root, nullptr);
		}
		~Parser() = default;

		Parser& operator = (const Parser& Other);

		SyntaxTree syntaxTree;
		std::map<std::string, Operator> operators; //predefined operators name->operator

		static Instruction ParseNumber(std::string Input);
		static StringT ParseString(std::string Input);
		static bool IsRegion(const Instruction& Instruction);

	protected:
		void CreatePredefinitions();
		void CreateOperator(const std::string& Name, Notation Notat, Association Assoc, unsigned Precedence);
		void Reparent(SyntaxTreeNode* Node, SyntaxTreeNode* Parent); //Reconnect all children to parents

		void ParseToken(Lexer::TokenList::const_iterator& Token, const Lexer::TokenList& List);

		void ParseStatement(SyntaxTreeNode* Statement); //parse a statement for operators. Converts Statement to the root op

		std::stack<std::string> blocks; //block matching
		SyntaxTreeNode* parent;
	};
};

inline std::ostream& operator << (std::ostream& Stream, const Plang::ParserException& Exception)
{
	Stream << Exception.what() << " [ " << Exception.token << " ] @ " << Exception.location;
	return Stream;
}
