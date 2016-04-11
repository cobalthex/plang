source_dir=ScriptingLang/
cpp_opts=--std=c++1y -m64

pch = clang++ $(cpp_opts) $(source_dir)pch.hpp -emit-pch -o $(1)/pch.pch

debug:
	@mkdir -p bin/debug
	$(call pch,"bin/debug")
	clang++ -g $(cpp_opts) $(source_dir)*.cpp -include-pch bin/debug/pch.pch -o bin/debug/plang
release:
	@mkdir -p bin/release
	$(call pch,"bin/release")
	clang++ $(cpp_opts) -o bin/release/plang $(source_dir)*.cpp

clean:
	rm -rf bin/
