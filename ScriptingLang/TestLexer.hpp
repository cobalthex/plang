#pragma once

#include "pch.hpp"
#include "Lexer.hpp"

#include <CppUnitTest.h>
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Plang
{
	namespace Tests
	{
		TEST_CLASS(LEXER)
		{
		public:
			TEST_METHOD(Empty)
			{
				std::istringstream iss ("");
				Plang::Lexer lex ("", iss);
				Assert::AreEqual(lex.tokens.size(), 0ull, L"Empty input = Empty output");
			}
		};
	};
};