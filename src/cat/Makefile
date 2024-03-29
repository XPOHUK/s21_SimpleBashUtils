.PHONY: all cpplint cppcheck clean rebuild

CFLAGS = -Wall -Wextra -Werror
LDFLAGS = -O0 -std=c11 -pedantic -Wno-stringop-truncation -ftrapv -Wundef -Wformat=2 -fanalyzer

all: cpplint s21_cat

s21_cat: s21_cat.o s21_cat.h
	gcc s21_cat.o -o $@

s21_cat.o: s21_cat.c s21_cat.h
	gcc $(CFLAGS) -c s21_cat.c -o $@

test: CFLAGS += -fsanitize=address
test: LDFLAGS += -fsanitize=address
test: s21_cat.o s21_cat.h
	gcc $(LDFLAGS) s21_cat.o -o s21_cat
	./t_cat.sh
	python ./s21_cat_tests.py
	make clean

test-valg: s21_cat.o s21_cat.h
	gcc $(LDFLAGS) s21_cat.o -o $@
	valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./test-valg -net data-samples/sample.txt data-samples/aboba.txt
	make clean

cpplint:
	cp ../../materials/linters/CPPLINT.cfg ./
	python3 ../../materials/linters/cpplint.py --extensions=c *.h *.c
	rm ./CPPLINT.cfg

cppcheck:
	cppcheck --enable=all --suppress=missingIncludeSystem *.c *.h
	
clean:
	rm -f *.o test-valg s21_cat
