CC         = gcc
CFLAGS     = -Wall -Wextra -Werror -pedantic-errors -O0 -g -std=c11
TARGET     = cat
# SUPPRESSOR = -Wno-unused-parameter -Wno-unused-variable


all: build

build: $(TARGET)

$(TARGET): cat.o
	@$(CC) cat.o -o $(TARGET)

cat.o: cat.c
	@$(CC) -c $(CFLAGS) $(SUPPRESSOR) cat.c -o cat.o

run: $(TARGET)
	./$(TARGET)

tests: $(TARGET)
	@cd tests && python3 tester.py && cd ..

clean:
	rm -rf $(TARGET)
	rm -rf cat.o


.PHONY: build run clean tests

