CXX		= g++ -std=c++11
CXXFLAGS	= -g -Wall
OBJS		= lexer.o modparser.o string.o Type.o checker.o MySymbol.o MyScope.o
PROG		= scc

all:		$(PROG)

$(PROG):	$(OBJS)
		$(CXX) -o $(PROG) $(OBJS)

clean:;		$(RM) $(PROG) core *.o
