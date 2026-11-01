CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Werror -g -pthread

.PHONY: all run test clean
all: telemonitor

telemonitor: ProcessViewer.cpp ProcessViewer.h main.cpp
	$(CXX) $(CXXFLAGS) -o $@ ProcessViewer.cpp main.cpp

run: telemonitor
	./telemonitor

test: process-tests
	./process-tests

process-tests: ProcessViewer.cpp ProcessViewer.h tests.cpp
	$(CXX) $(CXXFLAGS) -o $@ ProcessViewer.cpp tests.cpp

clean:
	rm -f telemonitor process-tests
