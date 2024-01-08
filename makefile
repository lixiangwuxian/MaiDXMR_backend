#build
main.o: main.cpp
	clang++ -std=c++11 -stdlib=libc++ -c main.cpp
build: main.o
	clang++ -std=c++11 -stdlib=libc++ -o build main.o