#include "pch.hpp"
#include "Parser.hpp"
#include "StringOps.hpp"

using namespace Plang;

std::set<std::string> Parser::postfixOperators
{
    "++", "--",
};
std::set<std::string> Parser::prefixOperators
{
    "++", "--", "!", "+", "-", "~", "="
};
std::map<std::string, BinaryOperator> Parser::infixOperators
{
    { "*", BinaryOperator("*", Association::LeftToRight, 3) },
    { "/", BinaryOperator("/", Association::LeftToRight, 3) },
    { "/", BinaryOperator("%", Association::LeftToRight, 3) },

    { "+", BinaryOperator("+", Association::LeftToRight, 4) },
    { "-", BinaryOperator("-", Association::LeftToRight, 4) },

    { "<<", BinaryOperator("<<", Association::LeftToRight, 5) },
    { ">>", BinaryOperator(">>", Association::LeftToRight, 5) },

    { "<", BinaryOperator("<", Association::LeftToRight, 6) },
    { "<=", BinaryOperator("<=", Association::LeftToRight, 6) },
    { ">=", BinaryOperator(">=", Association::LeftToRight, 6) },
    { ">", BinaryOperator(">", Association::LeftToRight, 6) },

    { "==", BinaryOperator("==", Association::LeftToRight, 7) },
    { "!=", BinaryOperator("!=", Association::LeftToRight, 7) },

    { "&", BinaryOperator("&", Association::LeftToRight, 8) },

    { "^", BinaryOperator("^", Association::LeftToRight, 9) },

    { "|", BinaryOperator("|", Association::LeftToRight, 10) },

    { "&&", BinaryOperator("&&", Association::LeftToRight, 11) },

    { "||", BinaryOperator("!=", Association::LeftToRight, 12) },

    { ":", BinaryOperator(":", Association::RightToLeft, 14) },
    { "=", BinaryOperator("=", Association::RightToLeft, 14) },
    //todo: other assignments
};

void Parser::Reparent(Instruction& node, const NonOwningRef<Instruction>& parent)
{
    //todo: make stack based
    node.parent = parent;
    auto children = std::get_if<TList>(&node.value);
    if (children != nullptr)
    {
        for (auto& i : *children)
            Reparent(i, &node);
    }
}

//add this node to the current parent
Instruction& Parser::AddChild(const Instruction& instruction)
{
    auto& list = parent->As<Instructions::List>();
    list.Add(instruction);
    list.Last().parent = parent;
    return list.Last();
}

Instruction& Parser::AddStatement(InstructionType parentType)
{
    parent = AddChild(Instruction(parentType, TList()));
    return AddChild(Instruction(InstructionType::Statement, TList()));
}

Instruction* Parser::LastInstruction()
{
    auto& list = parent->As<Instructions::List>();
    return (list.Count() > 0 ? &list.Last() : nullptr);
}

bool Parser::IsPunctuation(InstructionType instruction) const
{
    switch (instruction)
    {
    case InstructionType::Accessor:
    case InstructionType::List:
    case InstructionType::Block:
    case InstructionType::Tuple:
    case InstructionType::Program:
    case InstructionType::Statement:
        return true;
    default:
        return false;
    }
}

Parser::Parser()
{
    root = Plang::Instruction(InstructionType::Program, TList());
    parent = &root;
}

void Parser::Parse(const Lexer::TokenList& tokens, bool failOnFirstError)
{
    if (tokens.size() < 1)
        return;

    parent = &AddChild(Instruction(InstructionType::Statement, TList()));

    size_t errorCount = 0;

    for (auto it = tokens.cbegin(); it != tokens.cend(); ++it)
    {
        try
        {
            ParseNext(it, tokens);
        }
        catch (const EParser& e)
        {
            if (++errorCount > 0 && failOnFirstError)
                break;

            //todo: store exceptions
            std::cerr << "! " << e.Severity() << ": " << e.what() << " (" << e.token << ") @ " << e.location << "\n";
        }
    }

    if (parent->type == InstructionType::Accessor)
        parent = parent->parent;

    parent = parent->parent;
    if (parent != root)
    {
        auto loc(tokens.back().location);
        ++loc.column;
        throw EParserError("Unmatched )]}", LexerToken(LexerTokenType::Invalid, "", loc)); //todo
    }

    auto& list = root.As<Instructions::List>();
    if (list.Count() > 0)
        EvaluateStatement(list.Last());
}

void Parser::ParseNext(Lexer::TokenList::const_iterator& token, const Lexer::TokenList& allTokens) //todo: allTokens may not be necessary
{
    if (parent->type == InstructionType::Accessor &&
        token->type != LexerTokenType::Accessor &&
        (token - 1)->type != LexerTokenType::Accessor)
        parent = parent->parent;

    switch (token->type)
    {
    case LexerTokenType::Number:
    {
        auto number(ParseNumber(token->value));
        AddChild(number);
        break;
    }
    case LexerTokenType::String:
    {
        AddChild(Instruction(InstructionType::String, ParseString(token->value)));
        break;
    }
    case LexerTokenType::Identifier:
    {
        AddChild(Instruction(InstructionType::Identifier, token->value));
        break;
    }
    case LexerTokenType::Accessor:
    {
        auto& list = parent->As<Instructions::List>();
        if (parent->type != InstructionType::Accessor)
        {
            TList ax;
            if (list.Count() > 0)
            {
                ax.push_back(list.Last());
                list.Last() = Instruction(InstructionType::Accessor, ax);
            }
            else
            {
                ax.push_back(Instruction(InstructionType::Identifier, nullptr));
                AddChild(Instruction(InstructionType::Accessor, ax));
            }
            Reparent(list.Last(), parent);
            parent = &list.Last();
        }
        else if ((token - 1)->type == LexerTokenType::Accessor)
        {
            //reflexive accessor, a..b
            auto& list = parent->As<Instructions::List>();
            list.Add(Instruction(InstructionType::Identifier, nullptr));
        }
        break;
    }
    case LexerTokenType::ListOpen:
    {
        parent = &AddStatement(InstructionType::List);
        break;
    }
    case LexerTokenType::BlockOpen:
    {
        parent = &AddStatement(InstructionType::Block);
        break;
    }
    case LexerTokenType::TupleOpen:
    {
        parent = &AddStatement(InstructionType::Tuple);
        break;
    }
    case LexerTokenType::ListClose:
    {
        while (parent->parent != nullptr && !IsPunctuation(parent->type))
            parent = parent->parent;

        //todo: should always be statement?

        if (parent->type == InstructionType::Statement)
        {
            EvaluateStatement(*parent);
            parent = parent->parent;
        }

        if (parent->type != InstructionType::List)
            throw EParserError("Mismatched ]", *token);

        EvaluateStatement(*parent);
        parent = parent->parent;
        break;
    }
    case LexerTokenType::BlockClose:
    {
        while (parent->parent != nullptr && !IsPunctuation(parent->type))
            parent = parent->parent;

        //todo: should always be statement?
        if (parent->type == InstructionType::Statement)
        {
            EvaluateStatement(*parent);
            parent = parent->parent;
        }

        if (parent->type != InstructionType::Block)
            throw EParserError("Mismatched }", *token);

        EvaluateStatement(*parent);
        parent = parent->parent;
        break;
    }
    case LexerTokenType::TupleClose:
    {
        while (parent->parent != nullptr && !IsPunctuation(parent->type))
            parent = parent->parent;

        //todo: should always be statement?
        if (parent->type == InstructionType::Statement)
        {
            EvaluateStatement(*parent);
            parent = parent->parent;
        }

        if (parent->type != InstructionType::Tuple)
            throw EParserError("Mismatched )", *token);

        EvaluateStatement(*parent);
        parent = parent->parent;
        break;
    }
    case LexerTokenType::Separator:
    {
        while (parent->parent != nullptr && !IsPunctuation(parent->type))
            parent = parent->parent;

        //todo: assert list or tuple
        if (parent->type != InstructionType::Statement)
            throw EParserError("Unknown separator", *token);

        if (parent->As<Instructions::List>().Count() == 0)
            throw EParserError(parent->parent->TypeName() + " cannot have empty values", *token);

        EvaluateStatement(*parent);
        parent = parent->parent;

        switch (parent->type)
        {
        case InstructionType::List:
        case InstructionType::Tuple:
            parent = &AddChild(Instruction(InstructionType::Statement, TList()));
            break;
        default:
            throw EParserError("Invalid separator, must be in list or tuple", *token);
        }

        break;
    }
    case LexerTokenType::Terminator:
    {
        while (parent->parent != nullptr && !IsPunctuation(parent->type))
            parent = parent->parent;

        //special list/tuple parsing (2d arrays?)

        if (parent->type != InstructionType::Statement)
            throw EParserError("Unknown terminator", *token);

        //empty statement
        if (parent->As<Instructions::List>().Count() == 0)
            break;

        EvaluateStatement(*parent);

        parent = parent->parent;
        parent = &AddChild(Instruction(InstructionType::Statement, TList()));

        break;
    }
    }
}

void Parser::EvaluateStatement(Instruction& statement)
{
    if (statement.type != InstructionType::Statement)
        return;

    auto list = statement.As<Instructions::List>();
    auto stmtParent = statement.parent;

    if (list.Count() == 0)
    {
        stmtParent->As<Instructions::List>().Remove(statement);
        return;
    }

    std::stack<BinaryOperator> ops;
    std::stack<Instruction> output;

    auto infix = [&]()
    {
		if (output.size() == 0)
			output.push(Instruction(InstructionType::Identifier, ops.top().name));
        else if (output.size() == 1)
            throw EParserError("invalid operator", statement); //todo (should be operator location)

		else
		{
			auto second = output.top(); output.pop();
			auto first = output.top(); output.pop();

			output.push(Instructions::Call(
				Instruction(InstructionType::Identifier, ops.top().name),
				{ TryCollapseTuple(first), TryCollapseTuple(second) }
			));
		}
        ops.pop();
    };

    bool wasLastOp = false;
    for (size_t i = 0; i < list.Count(); ++i)
    {
        auto str = std::get_if<std::string>(&list[i].value);
        if (str != nullptr)
        {
            auto unaryFind(postfixOperators.find(*str));
            if (unaryFind != postfixOperators.end() && !output.empty() && output.top().type != InstructionType::Unknown)
            {
                output.top() = Instructions::Call(list[i], { Instruction(), output.top() });
                wasLastOp = true;
                continue;
            }

            unaryFind = prefixOperators.find(*str);
            if (unaryFind != prefixOperators.end() && (i == 0 || (wasLastOp && i < list.Count() - 1)))
            {
				output.push(Instructions::Call(list[i], { list[i + 1], Instruction() }));
				++i;
                continue;
            }

            auto binaryFind(infixOperators.find(*str));
            if (binaryFind != infixOperators.end())
            {
                auto& op = binaryFind->second;
                while (!ops.empty())
                {
                    auto& top = ops.top();
                    if ((op.association == Association::LeftToRight && op.precedence >= top.precedence) ||
                        (op.association == Association::RightToLeft && op.precedence > top.precedence))
                        infix();
                    else
                        break;
                }

                ops.push(op);

                wasLastOp = true;
                continue;
            }
        }

        //list accessor ( a[x] )
        if (list[i].type == InstructionType::List && !output.empty())
        {
            auto& top = output.top();
            list[i].As<Instructions::List>().Insert(top);
            top = list[i];
            top.type = InstructionType::Accessor;
        }

        //function call ( f() )
        else if (list[i].type == InstructionType::Tuple && !output.empty() && !wasLastOp)
        {
            auto& top = output.top();
            top = Instructions::Call(top, list[i]);
        }

        //expression ( () { } )
        else if (list[i].type == InstructionType::Block && !output.empty() && !wasLastOp)
        {
            auto& top = output.top();
            top = Instructions::Expression(top, list[i]);
        }

        else
        {
            output.push(TryCollapseTuple(list[i])); //todo: maybe collapse tuples in later pass (some places dont allow tuple collapse maybe)
            wasLastOp = false;
        }
    }

    while (!ops.empty())
        infix();

	if (output.size() != 1)
	{
		std::cout << "!! ";
		for (const auto& o : *((std::deque<Instruction>*)&output))
			std::cout << ">> " << o;
		throw EParserError("Too many operands", output.top());
	}
    statement = output.top();

    Reparent(statement, stmtParent);
}

Instruction Parser::ParseNumber(std::string input)
{
    StringOps::Replace(input, "'", "");

    int base = 10;
    auto baseOffset = input.find_last_of('_');
    if (baseOffset != std::string::npos)
    {
        auto baseStr = input.substr(baseOffset + 1);
        input.resize(baseOffset);
        base = std::stoi(baseStr);
    }
    else if (StringOps::StartsWith(input, "0x"))
        base = 16;

    //todo: handle invalid numbers: 0xff_8; 99_8;

    if (input.find('.') != input.npos)
        return { InstructionType::Float, std::stod(input) };
    else
        return { InstructionType::Int, std::stoll(input, nullptr, base) };
}

std::string Parser::ParseString(std::string Input)
{
    bool interpolate = Input[0] == '"';
    Input = Input.substr(1, Input.length() - 2);
    return Input;
}
