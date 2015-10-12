source_dir=ScriptingLang/

debug:	
	mkdir -p Debug
	clang++ -o Debug/plang -g --std=c++14 $(source_dir)*.cpp

release:
	mkdir -p Release
	clang++ -o Release/plang --std=c++14 $(source_dir)*.cpp

clean:
	rm -rf Debug/ Release/
