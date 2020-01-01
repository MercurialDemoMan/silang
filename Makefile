# compiler
CC  = g++

# dependecies
LIB = -lm

# compiler flags
FLG = -Wall -Wextra -g -O2 -std=c++17
DEF = -DARCH=SILENT

# product specifications
OUT = ./silang

# no make argument
all: $(OUT)

OUT_SOURCES := $(wildcard ./src/*.cpp)
OUT_OBJECTS := $(patsubst ./src/%.cpp, ./out/%.o, $(OUT_SOURCES))
OUT_DEPENDS := $(patsubst ./src/%.cpp, ./out/%.d, $(OUT_SOURCES))

# link compiled objects
$(OUT): $(OUT_OBJECTS)
	$(CC) $^ $(LIB) -o $(OUT)

-include $(OUT_DEPENDS)

./out/%.o: ./src/%.cpp
	$(CC) $(FLG) $(DEF) $(INC) -MMD -c $< -o $@


# clean exe folder
clean:
	rm -f $(OUT) $(OUT_OBJECTS) $(OUT_DEPENDS)

# compile and run
run: $(OUT)
	./$(OUT)

