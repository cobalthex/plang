#include "pch.hpp"
#include "StringOps.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"

int main(int ac, const char* av[])
{
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
				std::cout << "! Parser Error: " << Expt << std::endl;
				continue;
			}

			std::cout << ">> " << parser.syntaxTree << std::endl;
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
	fout << parser.syntaxTree << std::endl;
	fout.close();

	return 0;
}
