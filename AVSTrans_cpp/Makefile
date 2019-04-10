CC = g++
CC_FLAGS = -w -DAVSTRANS_CLI

EXEC=avstrans-cli
SOURCES = cli-main.cpp AVSTrans_cpp.cpp
OBJECTS = $(SOURCES:.cpp=.o)

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $^ -o $@

%.o: %.cpp
	$(CC) -c $(CC_FLAGS) $< -o $@

# To remove generated files
clean:
	rm -f $(EXEC) $(OBJECTS)
