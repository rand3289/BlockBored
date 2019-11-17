all:
	g++ -g -std=c++17 block.cpp sdltxt.cpp sdlaudio.cpp -o bb -lSDL2
# wtf? gcc requires -lSDL2 at the end now!!!
