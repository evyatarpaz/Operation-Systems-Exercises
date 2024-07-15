# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -std=c++20 -pthread

# Executable name
TARGET = ex3.out

# Source files
SRCS = BoundedBuffer.cpp CoEditor.cpp Dispatcher.cpp Producer.cpp ScreenManager.cpp UnboundedBuffer.cpp ex3Main.cpp

# Header files
HDRS = BoundedBuffer.h CoEditor.h Dispatcher.h Producer.h ScreenManager.h UnboundedBuffer.h

# Object files
OBJS = $(SRCS:.cpp=.o)

# Default rule
all: $(TARGET) clean_objs

# Rule to link the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Rule to compile source files into object files
%.o: %.cpp $(HDRS)
	$(CXX) $(CXXFLAGS) -c $< -o $@ -g

# Rule to clean up object files after build
clean_objs:
	rm -f $(OBJS)

# Clean rule to remove object files and executable
clean:
	rm -f $(OBJS) $(TARGET)
