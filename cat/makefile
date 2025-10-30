CC         = gcc
CFLAGS     = -Wall -Wextra -Werror -pedantic-errors -O0 -g -std=c11
TARGET     = cat
# SUPPRESSOR = -Wno-unused-parameter -Wno-unused-variable


build: $(TARGET)

$(TARGET): cat.o
	@$(CC) cat.o -o $(TARGET)

cat.o: cat.c
	@$(CC) -c $(CFLAGS) $(SUPPRESSOR) cat.c -o cat.o

tests: $(TARGET)
	@cd tests && python3 tester.py

clean:
	rm -rf $(TARGET)
	rm -rf cat.o


.PHONY: build clean tests

