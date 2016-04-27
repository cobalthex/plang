#include "pch.hpp"
#include "StringOps.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Scope.hpp"
#include "Reference.hpp"
#include "Construct.hpp"

int main(int ac, const char* av[])
{
	Plang::Reference<Plang::Int> i (5);
	Plang::Reference<Plang::String> s ("test");
	Plang::Reference<Plang::Float> f (4.0);
	Plang::Reference<Plang::Float> b (true);

	std::cout << *i << "\n";
	Plang::AnyRef c(i);
	std::cout << *c << "\n";

	Plang::Signature z (::Array<Plang::Argument> { { "a", Plang::ArgumentType::Single }, { "b", Plang::ArgumentType::Tuple } });
	auto x = Plang::Tuple({ i, s, f });

	Plang::Function t (z, [](const Plang::Scope& Args) { std::cout << Args << std::endl; return Plang::Undefined; });
	t.Call(x, nullptr);

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
			}
			catch (const Plang::ParserException& Expt)
			{
				std::cout << "! Parser Error: " << Expt << "\n";
				continue;
			}

			std::cout << ">> " << parser.syntaxTree << "\n";
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
