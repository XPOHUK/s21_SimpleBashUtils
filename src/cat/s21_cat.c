#include "s21_cat.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int verbose;
int nonblank;  // GNU: --number-nonblank
int ends;      // GNU only: -E the same, but without implying -v
int numbers;   // GNU: --number
int squeeze;   // GNU: --squeeze-blank
int tabs;      // GNU: -T the same, but without implying -v

#ifdef __linux__
int line_count;
int prev_ch = -1;
#endif

int main(int argc, char **argv) {
    int res = 0;

    if (argc > 1) {
        while (1) {
            int c;
            int option_ind = 0;
            static struct option long_options[] = {{"number-nonblank", 0, NULL, 'b'},
                                                   {"number", 0, NULL, 'n'},
                                                   {"squeeze-blank", 0, NULL, 's'},
                                                   {0, 0, 0, 0}};
            c = getopt_long(argc, argv, "vbenst", long_options, &option_ind);
            if (c == -1 || (res = check_opt(c))) {
                break;
            }
        }
    } else {
        res = 1;
    }
    if (!res) {
        if (optind == argc) {
            fprintf(stderr, "no files in arguments");
            res = 1;
        } else {
            while (optind < argc) {
                char *file_name = argv[optind++];
                if (!(res = check_file(file_name))) {
                    errno = 0;
                    FILE *f = fopen(file_name, "r");
                    if (f) {
                        output(f);
                        fclose(f);
                    } else {
                        perror("Error");
                        res = 1;
                    }
                }
            }
        }
    }
    return res;
}

int check_opt(int c) {
    int res = 0;
    switch (c) {
        case 'b':
            numbers = 0;
            nonblank = 1;
            break;
        case 'n':
            if (!nonblank) numbers = 1;
            break;
        case 's':
            squeeze = 1;
            break;
        case 'e':
            ends = 1;
            verbose = 1;
            break;
        case 't':
            tabs = 1;
            verbose = 1;
            break;
        // case 'E':
        //     ends = 1;
        //     break;
        // case 'T':
        //     tabs = 1;
        //     break;
        case 'v':
            verbose = 1;
            break;
        case '?':
            fprintf(stderr, "bad option\n");
            res = 1;
            break;
        default:
            break;
    }
    return res;
}

int check_file(char *file_name) {
    int res = 1;
    struct stat path_stat;
    errno = 0;
    int stat_res = stat(file_name, &path_stat);
    if (!stat_res) {
        if (S_ISDIR(path_stat.st_mode)) {
            fprintf(stderr, "%s is a directory\n", file_name);
        } else if (S_ISREG(path_stat.st_mode)) {
            res = 0;
        }
    } else {
        perror("Error");
    }
    return res;
}

int output(FILE *f) {
    int res = 0;
#ifndef __linux__
    int line_count = 0;
    int prev_ch = -1;
#endif
    int prev_line_empty = 0;
    while (!feof(f)) {
        int ch = fgetc(f);
        if (ch == -1) {
            break;
        }
        if (ch == '\n' && (prev_ch == '\n' || prev_ch == -1)) {
            if (squeeze && prev_line_empty)
                continue;
            else
                prev_line_empty = 1;
        }
        if (prev_ch == '\n' && ch != '\n') prev_line_empty = 0;
        if ((prev_ch == '\n' || prev_ch == -1) && (numbers || (nonblank && ch != '\n'))) {
            line_count++;
            printf("%6d\t", line_count);
        }
        if (verbose) {
            if (ch < -96) {
                printf("M-^%c", ch + 192);
            } else if (ch < 0) {
                printf("M-%c", ch + 128);
            } else if (ch == 9) {
                if (tabs)
                    printf("^I");
                else
                    printf("%c", ch);
            } else if (ch == 10) {
                if (ends)
                    printf("$\n");
                else
                    printf("%c", ch);
            } else if (ch < 32) {
                printf("^%c", ch + 64);
            } else if (ch < 127) {
                printf("%c", ch);
            } else if (ch == 127) {
                printf("^?");
            } else if (ch < 160) {
                printf("M-^%c", ch - 64);
            } else if (ch < 255) {
                printf("M-%c", ch - 128);
            } else {
                printf("M-^?");
            }
        } else {
            printf("%c", ch);
        }
        prev_ch = ch;
    }
    return res;
}
