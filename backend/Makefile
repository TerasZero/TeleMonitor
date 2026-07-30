CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -g

TARGET = telemonitor
SOURCES = ProcessViewer.cpp

all:
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)
