#include "pch.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "StringOps.hpp"

int main(int ac, const char* av[])
{
	if (ac < 2)
	{
		std::cout << av[0] << " <script>";
		return 1;
	}

	std::ifstream fin;
	fin.open(av[1], std::ios::in);
	if (!fin.is_open())
	{
		std::cerr << "Error opening " << av[1];
		return 2;
	}

	Plang::Lexer lex(fin);
	fin.close();

	std::ofstream fout;
	fout.open("tests/out.lex", std::ios::out);
	fout << lex;
	fout.close();


	Plang::Parser parser(lex);
	fout.open("tests/out.parse", std::ios::out);
	fout << parser.syntaxTree << std::endl;
	fout.close();

	return 0;
}