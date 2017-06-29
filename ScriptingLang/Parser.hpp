#pragma once

#include "pch.hpp"
#include "Lexer.hpp"
#include "Instruction.hpp"

namespace Plang
{
    enum class Association
    {
        None,
        LeftToRight,
        RightToLeft
    };
    struct BinaryOperator
    {
        std::string name;
        Association association;
        unsigned precedence; //lower numbers = higher precedence

        BinaryOperator(const std::string& name, Association association, unsigned precedence)
            : name(name), association(association), precedence(precedence) { }
    };

    class EParser : public std::exception
    {
    public:
        EParser(const std::string& message, const LexerToken& token)
            : message(message), token(token.value), location(token.location)
        {
        }
        EParser(const std::string& message, const Instruction& instruction)
            : message(message), token(instruction.TypeName()), location(instruction.location)
        {
        }

        virtual const std::string Severity() const = 0;

        const std::string token;
        const Location location;
        const std::string message;

        inline virtual const char* what() const noexcept override
        {
            return message.c_str();
        }
    };
    class EParserWarning : public EParser
    {
    public:
        EParserWarning(const std::string& message, const LexerToken& token)
            : EParser(message, token) { }
        EParserWarning(const std::string& message, const Instruction& instruction)
            : EParser(message, instruction) { }

        const std::string Severity() const override { return "Warning"; }
    };
    class EParserError : public EParser
    {
    public:
        EParserError(const std::string& message, const LexerToken& token)
            : EParser(message, token) { }
        EParserError(const std::string& message, const Instruction& instruction)
            : EParser(message, instruction) { }

        const std::string Severity() const override { return "Error"; }
    };

    class Parser
    {
    public:
        Parser();
        Parser(const Parser& Parser)
            : root(Parser.root)
        {
            Reparent(root, nullptr);
        }
        ~Parser() = default;

        void Parse(const Lexer::TokenList& tokens, bool failOnFirstError = false);

        Instruction root;

        static Instruction ParseNumber(std::string Input);
        static std::string ParseString(std::string Input);

    protected:
        void ParseNext(Lexer::TokenList::const_iterator& token, const Lexer::TokenList& list);
        void EvaluateStatement(Instruction& instruction);

        void Reparent(Instruction& instruction, const NonOwningRef<Instruction>& parent);
        Instruction& AddChild(const Instruction& instruction);
        Instruction& AddStatement(InstructionType parentType);
        Instruction* LastInstruction();

        bool IsPunctuation(InstructionType instruction) const;

        const Instruction& TryCollapseTuple(const Instruction& instruction) const
        {
            if (instruction.type == InstructionType::Tuple)
            {
                auto l = std::get_if<TList>(&instruction.value);
                if (l != nullptr && l->size() == 1)
                    return l->front();
            }
            return instruction;
        }

        NonOwningRef<Instruction> parent;

        static std::set<std::string> prefixOperators;
        static std::set<std::string> postfixOperators;
        static std::map<std::string, BinaryOperator> infixOperators;
    };
};

//todo: operators should use map<name, map<notation, op>>
