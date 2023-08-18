CXX   = g++-9
EXE = ./iplC
CXXDEBUG = -g -O0  -Wall 
CXXSTD = -std=c++11

.PHONY: all	
all: parser.o scanner.o  ast.o type.o symbtab.o main.o	
	$(CXX) $(CXXDEBUG) parser.o scanner.o  type.o symbtab.o ast.o main.o -o $(EXE)

main.o: main.cpp
	$(CXX) $(CXXDEBUG) -c main.cpp -o main.o

type.o: type.cpp
	$(CXX) $(CXXDEBUG) -c type.cpp -o type.o

symbtab.o: symbtab.cpp
	$(CXX) $(CXXDEBUG) -c symbtab.cpp -o symbtab.o

ast.o: ast.hh ast.cpp
	$(CXX) $(CXXDEBUG) -c ast.cpp -o ast.o

parser.o: parser.yy 
	bison -v --graph $<  
	$(CXX) $(CXXDEBUG) -c parser.tab.cc -o parser.o 

scanner.o: scanner.l scanner.hh parser.tab.hh parser.tab.cc	
	flex++ --outfile=scanner.yy.cc  $<
	$(CXX)  $(CXXDEBUG) -c scanner.yy.cc -o scanner.o

.PHONY: clean
clean:
	rm *.o parser.tab.cc parser.output scanner.yy.cc iplC parser.gv
