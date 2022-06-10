.PHONY: all cpplint cppcheck clean rebuild

CFLAGS = -Wall -Wextra -Werror
LDFLAGS = -O0 -std=c11 -pedantic -Wno-stringop-truncation -ftrapv -Wundef -Wformat=2 -fanalyzer

all: s21_grep

s21_grep: s21_grep.o s21_grep.h
	gcc s21_grep.o -o $@

s21_grep.o: s21_grep.c s21_grep.h
	gcc $(CFLAGS) -c s21_grep.c -o $@

test: CFLAGS += -fsanitize=address
test: LDFLAGS += -fsanitize=address
test: s21_grep.o s21_grep.h
	gcc $(LDFLAGS) s21_grep.o -o s21_grep
	python ./s21_grep_tests.py
	make clean

test-valg:
	gcc $(CFLAGS) -g -c s21_grep.c
	gcc $(LDFLAGS) s21_grep.o -o $@
	valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./test-valg -o -f test_files/grep_file1 test_files/grep_file4 test_files/grep_file1
	make clean

cpplint:
	cp ../../materials/linters/CPPLINT.cfg ./
	python3 ../../materials/linters/cpplint.py --extensions=c *.h *.c
	rm ./CPPLINT.cfg

cppcheck:
	cppcheck --enable=all --suppress=missingIncludeSystem *.c *.h

clean:
	rm -f *.o test-valg s21_grep
