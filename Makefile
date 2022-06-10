CXX = g++
CCFLAGS = -g -Wall -pthread -std=c++17
PROGRAM = lineCounter
SOURCES = main.cpp argumentParse.cpp lineCounter.cpp
CLEAR = rm -f

$(PROGRAM):	$(OBJECTS)
	$(CXX) $(CCFLAGS) $(SOURCES) -o $(PROGRAM)
	@echo "$@ is compiled"

clean:
	$(CLEAR) $(PROGRAM)