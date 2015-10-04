#include "pch.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include <iomanip>
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
	for (auto& i : lex.tokens)
		fout << std::setw(12) << i.type << ": " << i.value << std::endl;
	fout.close();


	Plang::Parser parser(lex);

	return 0;
}