#pragma once

#include "pch.hpp"
#include "Lexer.hpp"
#include "Union.hpp"

namespace Plang
{
	struct Preprocessor
	{
		std::string rule;
		std::vector<std::string> values;
	};

	enum class ValueType
	{
		Undefined,
		Null,
		Boolean,
		Integer,
		Float,
		Decimal,
		String,
		Tuple,
		NamedTuple,
		List,
		Array,
		Subject,
		Predicate
	};

	enum class TupleType
	{
		Indexed,
		Associative
	};
	struct ParserTuple
	{
		TupleType type;
		Union<std::vector<std::string>, std::map<std::string, std::string>> values;
	};

	struct Instruction
	{
		std::string identifier;
		ParserTuple arguments;
	};

	enum class Association
	{
		None,
		LeftToRight,
		RightToLeft
	};
	struct ParseIdentifier
	{
		std::string name;
		Association association;
		unsigned precedence; //lower numbers = higher precedence
	};

	class Parser
	{
	public:
		Parser(const Lexer& Lex);
		~Parser() = default;

		std::map<std::string, ParseIdentifier> predefinedIdentifiers; //predefined parse-identifiers

		std::vector<Instruction> instructions;

	protected:
		void CreatePredefinedIdentifiers();
	};
};