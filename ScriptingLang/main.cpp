#include "pch.hpp"
#include "StringOps.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Value.hpp"

int main(int ac, const char* av[])
{
	Plang::Lexer lex;
	Plang::Parser parser;

	Plang::Value v;
	v.type = Plang::ValueType::Int;
	v.data = 5;

	Plang::Value u;
	u.type = Plang::ValueType::Dictionary;
	u.data = Plang::Dictionary();

	Plang::Reference r (&v);
	auto uvd = u.Get<Plang::Dictionary>();
	uvd["test"] = r;
	auto uv = uvd["test"];
	std::cout << uv->Get<Plang::Int>() << std::endl;

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

			lex = Plang::Lexer("#!", iss);
			parser = Plang::Parser(lex.tokens);

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
