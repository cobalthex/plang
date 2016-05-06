#include "pch.hpp"
#include "StringOps.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Reference.hpp"
#include "Construct.hpp"

int main(int ac, const char* av[])
{
	Plang::AnyRef global = Plang::Construct();

	Plang::Function as({ { "Val", Plang::ArgumentType::Single }, { "Type", Plang::ArgumentType::Single } }, [](const Plang::Construct& Arguments)
	{
		auto& asFn = Arguments.Get("Val")->Get("as");
		if (asFn != Plang::Undefined)
		{
			Plang::Tuple args({ Arguments.Get("Type") });
			if (asFn->Type() == Plang::ConstructType::Function)
			{
				auto fn = asFn.As<Plang::Function>();
				return fn->Call(args);
			}
			if (asFn->Type() == Plang::ConstructType::Script)
			{
				auto script = asFn.As<Plang::Script>();
				return script->Evaluate(args);
			}
		}
		return Plang::Undefined;
	});
	global->Set("as", Plang::Reference<Plang::Function>(as));

	auto& x = global->Set("x", Plang::Reference<Plang::Int>(5));
	x->Set("test", Plang::Reference<Plang::Float>(10.f));

	Plang::Function log({ { "args", Plang::ArgumentType::Tuple } }, [](const Plang::Construct& Arguments)
	{
		return Arguments.Get("args");
	});
	global->Set("log", Plang::Reference<Plang::Function>(log));

	Plang::Lexer lex;
	Plang::Parser parser;
	if (ac < 2)
	{
		std::cout << "Interactive mode\n";

		std::string line;
		std::istringstream iss;
		while (std::cin.good())
		{
			std::cout << "<< ";
			std::getline(std::cin, line);

			if (line == "exit")
			{
				std::cout << "Bye!\n";
				return 0;
			}

			iss.str(line);
			iss.clear();

			try
			{
				lex = Plang::Lexer("#!", iss);
				parser = Plang::Parser(lex.tokens);
				Plang::Script(&parser.syntaxTree.root).Evaluate(global);
			}
			catch (const Plang::ParserException& Expt)
			{
				std::cout << "! Parser Error: " << Expt << "\n";
				continue;
			}
		}

		// std::cout << av[0] << " <script>";
		// return 1;
	}

	std::ifstream fin;
	fin.open(av[1], std::ios::in);
	if (!fin.is_open())
	{
		std::cerr << "Error opening " << av[1];
		return 2;
	}

	lex = Plang::Lexer(av[1], fin);
	fin.close();

	std::ofstream fout;
	fout.open("tests/out.lex", std::ios::out);
	fout << lex;
	fout.close();


	parser = Plang::Parser(lex.tokens);
	fout.open("tests/out.parse", std::ios::out);
	fout << parser.syntaxTree << "\n";
	fout.close();

	return 0;
}
