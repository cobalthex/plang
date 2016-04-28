#include "pch.hpp"
#include "StringOps.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Reference.hpp"
#include "Construct.hpp"

int main(int ac, const char* av[])
{
	Plang::AnyRef global = Plang::Construct();

	Plang::Signature sgn ({ { "a", Plang::ArgumentType::Single }, { "b", Plang::ArgumentType::Single } });
	Plang::Function plus(sgn, [](const Plang::Construct& Arguments) { std::cout << ">>> " << Arguments << std::endl; return Plang::Undefined; });

	global->Set("+", Plang::Reference<Plang::Function>(plus));

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
