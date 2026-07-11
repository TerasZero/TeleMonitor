CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -g

TARGET = telemonitor
SOURCES = main.cpp ProcessViewer.cpp

all:
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)
