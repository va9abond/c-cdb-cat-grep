CC         = gcc
CFLAGS     = -Wall -Wextra -Werror -pedantic-errors -O0 -g -std=c11
TARGET     = grep
# SUPPRESSOR = -Wno-unused-parameter -Wno-unused-variable
INC        = ../common


build: $(TARGET)

$(TARGET): __grep.o __xstdio.o
	@$(CC) __grep.o __xstdio.o -o $(TARGET)

__grep.o: grep.c
	@$(CC) -I $(INC) -c $(CFLAGS) $(SUPPRESSOR) grep.c -o __grep.o

__xstdio.o: $(INC)/__xstdio.c
	@$(CC) -I $(INC) -c $(CFLAGS) $(SUPPRESSOR) $(INC)/__xstdio.c -o __xstdio.o

run: $(TARGET)
	./$(TARGET)

tests: $(TARGET)
	@cd tests && python3 tester.py

clean:
	rm -rf $(TARGET)
	rm -rf __grep.o
	rm -rf __xstdio.o


.PHONY: build run clean

