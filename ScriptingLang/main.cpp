#include "pch.hpp"
#include "StringOps.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Reference.hpp"
#include "Construct.hpp"

int main(int ac, const char* av[])
{
	Plang::AnyRef global = Plang::Construct();
	
	Plang::Function as ({ { "From", Plang::ArgumentType::Single }, { "To", Plang::ArgumentType::Single } }, [](const Plang::Construct& Arguments)
	{
		auto& asFn = Arguments.Get("From")->Get("as");
		if (asFn != Plang::Undefined)
		{
			Plang::Tuple args({ Arguments.Get("From"), Arguments.Get("To") });
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
	x->Set("as", Plang::Reference<Plang::Function>({ { { "From", Plang::ArgumentType::Single }, { "To", Plang::ArgumentType::Single } }, [](const Plang::Construct& Arguments)
	{
		auto from = Arguments.Get("From");
		auto to = Arguments.Get("To");

		Plang::AnyRef rval (Plang::Undefined);
		if (to != Plang::Undefined && to->Type() == Plang::ConstructType::String)
			rval = Plang::Reference<Plang::String>("ecks");

		return rval;
	} }));

	Plang::Function ast ({ { "args", Plang::ArgumentType::Tuple } }, [](const Plang::Construct& Arguments)
	{
		auto& args = Arguments.Get("args").As<Plang::Tuple>();
		for (auto i = 0; i < args->Length(); i++)
		{
			auto& val = (*args)[i];
			std::cout << val << "\n";
		}
		return Plang::Undefined;
	});
	global->Set("ast", Plang::Reference<Plang::Function>(ast));

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
			
			if (line.length() > 2 && line[0] == '#' && line[1] == '!')
			{
				system(line.data() + 2);
				continue;
			}

			iss.str(line);
			iss.clear();

			try
			{
				lex = Plang::Lexer("#!", iss);
				parser = Plang::Parser(lex.tokens);
				//std::cout << parser.syntaxTree.root << std::endl;
				Plang::Script(parser.syntaxTree.root).Evaluate(global);
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
	
	try
	{
		if (StringOps::StartsWith(av[1], "-e") || StringOps::StartsWith(av[1], "--eval"))
		{
			const char* sp = av[2];
			if (strlen(av[1]) > 2)
				sp = av[1] + 2;
			
			std::istringstream iss(sp);

			lex = Plang::Lexer("#!", iss);
		}

		else
		{
			std::ifstream fin;
			fin.open(av[1], std::ios::in);
			if (!fin.is_open())
			{
				std::cerr << "Error opening " << av[1];
				return 2;
			}

			lex = Plang::Lexer(av[1], fin);
			fin.close();
		}

		//std::ofstream fout;
		//fout.open("tests/out.lex", std::ios::out);
		//fout << lex;
		//fout.close();


		parser = Plang::Parser(lex.tokens);
		//fout.open("tests/out.parse", std::ios::out);
		std::cout << parser.syntaxTree << "\n";
		//fout.close();
	}
	catch (const Plang::ParserException& Expt)
	{
		std::cout << "! Parser Error: " << Expt << "\n";
		return 1;
	}

	return 0;
}
