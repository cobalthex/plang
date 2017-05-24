#include "pch.hpp"
#include "StringOps.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Instruction.hpp"
//#include "Reference.hpp"
//#include "Construct.hpp"

int main(int ac, const char* av[])
{
    using namespace std::literals::string_literals;
    using I = Plang::Instruction;

    I root(Plang::InstructionType::Program, Plang::TList {
        I(Plang::InstructionType::Float, 5.0),
        I(Plang::InstructionType::Identifier, "taco"s),
        I(Plang::InstructionType::List, Plang::TList {
            I(Plang::InstructionType::String, "Moo"s)
        })
    });

    /*std::cout << root.As<Plang::Instructions::List>()[0];
    std::cin.get();*/

	/*Plang::Parser parser;
	Plang::AnyRef scope = Plang::Construct();*/
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
            else if (line == "clear")
            {
#ifdef _WIN32
                system("cls");
#elif
                system("clear");
#endif
                continue;
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
				Plang::Lexer lex("#!", iss);
                Plang::Parser parser;
                parser.Parse(lex.tokens);

                std::cout << parser.root << std::endl;
			}
			catch (const std::exception& x)
			{
				std::cout << "! Error: " << x.what() << "\n";
				continue;
			}
		}

		// std::cout << av[0] << " <script>";
		// return 1;
	}

	/*try
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
		//std::cout << parser.syntaxTree << "\n";
		//fout.close();
		Plang::Script(parser.syntaxTree.root).Evaluate(global);
	}
	catch (const Plang::EParser& Expt)
	{
		std::cout << "! Parser Error: " << Expt << "\n";
		return 1;
	}*/

	return 0;
}
