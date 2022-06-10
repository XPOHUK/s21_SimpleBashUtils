#define _GNU_SOURCE

#include "s21_grep.h"

#include <errno.h>
#include <getopt.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_ERR_LENGTH 80

// #ifdef __linux__
// int line_count;
// int prev_ch = -1;
// #endif

int main(int argc, char **argv) {
    t_parsed s = {0};
    int res = 0;
    char *pattern = NULL;
    if (argc > 1) {
        while (1) {
            int c;
            c = getopt(argc, argv, "e:ivclnhsf:o");
            if (c == -1 || (res = check_opt(&s, c, optarg))) {
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
            // Если нет флага -f или -e, то первый аргумент это паттерн
            if (!(s.f_flag) && !(s.e_flag)) {
                pattern = argv[optind++];  // Надо ли освобождать паттерн? Наверное нет.
                s.expr_len++;
                char **tmp = reallocarray(s.expr, s.expr_len, sizeof(char *));
                if (tmp) {
                    s.expr = tmp;
                    char *patt_copy = calloc(sizeof(char), strlen(pattern) + 1);
                    strcpy(patt_copy, pattern);
                    s.expr[s.expr_len - 1] = patt_copy;
                }
            }
            regex_t *compiled = malloc(sizeof(regex_t) * s.expr_len);  // Требуется очистка
            comp_regexs(&s, compiled);

            if (argc - optind == 1) s.h_flag = 1;
            while (optind < argc) {
                char *file_name = argv[optind++];
                if (!(res = check_file(file_name, s.s_flag))) {
#ifdef __linux__
                    if (s.v_flag && s.o_flag) {
                        res = 0;
                    } else {
#endif
                        errno = 0;
                        FILE *f = fopen(file_name, "r");
                        if (f) {
                            res = proc_file(f, compiled, &s, file_name);
                            fclose(f);
                        } else {
                            perror("Error");
                            res = 1;
                        }
#ifdef __linux__
                    }
#endif
                }
            }
            if (s.expr) {
                for (int i = 0; i < s.expr_len; i++) {
                    free(s.expr[i]);
                    regfree(&compiled[i]);
                }
                free(s.expr);
                free(compiled);
            }
        }

        // free(&s);
    }
    return res;
}

int check_opt(t_parsed *s, int c, char *optarg) {
    int res = 0;
    char **tmp = NULL;
    switch (c) {
        case 'e':
            s->e_flag = 1;
            s->expr_len++;
            tmp = reallocarray(s->expr, s->expr_len, sizeof(char *));
            if (tmp) {
                s->expr = tmp;
                char *optarg_copy = calloc(sizeof(char), strlen(optarg) + 1);
                strcpy(optarg_copy, optarg);
                s->expr[s->expr_len - 1] = optarg_copy;
            } else {
                // Ошибка выделения памяти
            }
            break;
        case 'i':
            s->i_flag = 1;
            break;
        case 'v':
            s->v_flag = 1;
#ifndef __linux__
            s->o_flag = 0;
#endif
            break;
        case 'c':
            if (!s->l_flag) {
                s->c_flag = 1;
                // Надо обнулить другие флаги ответственные за вывод
                s->n_flag = s->o_flag = 0;
            }
            break;
        case 'l':
            s->l_flag = 1;
            // Надо обнулить другие флаги ответственные за вывод
            s->c_flag = s->n_flag = s->h_flag = s->o_flag = 0;
            break;
        case 'n':
            if (!s->c_flag && !s->l_flag) s->n_flag = 1;
            break;
        case 'h':
            if (!s->l_flag) s->h_flag = 1;
            break;
        case 's':
            s->s_flag = 1;
            break;
        case 'f':
            s->f_flag = 1;
            res = read_expr(s, optarg);
            // printf("expr in case: %s\n", (s->expr)[0]);
            break;
        case 'o':
#ifdef __linux__
            if (!s->c_flag && !s->l_flag) s->o_flag = 1;
#else
            if (!s->c_flag && !s->l_flag && !s->v_flag) s->o_flag = 1;
#endif
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

int check_file(char *file_name, int silent) {
    int res = 1;
    struct stat path_stat;
    errno = 0;
    int stat_res = stat(file_name, &path_stat);
    if (!stat_res) {
        if (S_ISDIR(path_stat.st_mode) && !silent) {
            fprintf(stderr, "%s is a directory\n", file_name);
        } else if (S_ISREG(path_stat.st_mode)) {
            res = 0;
        }
    } else if (!silent) {
        perror("Error");
    }
    return res;
}

int read_expr(t_parsed *s, char *file_name) {
    int res = check_file(file_name, s->s_flag);
    if (!res) {
        FILE *f = fopen(file_name, "r");
        while (!feof(f)) {
            char *str = NULL;
            int read_res = read_line(f, &str);
            if (read_res == 0 || read_res == 2) {
                (s->expr_len)++;
                char **tmp = reallocarray(s->expr, s->expr_len, sizeof(char *));
                if (tmp) {
                    s->expr = tmp;
                    (s->expr)[(s->expr_len) - 1] = str;
                    // printf("expr: %s\n", (s->expr)[(s->expr_len) - 1]);
                } else {
                    // Ошибка выделения памяти
                }
            } else {
                // Error read line
                res = 1;
            }
        }
        fclose(f);
    }
    return res;
}

int read_line(FILE *f, char **str) {
    int res = 0;
    int max_len = 100;
    *str = (char *)malloc(sizeof(char) * max_len);

    if (*str == NULL) {
        fprintf(stderr, "Error allocating memory for line buffer.");
        res = 1;
    } else {
        int ch = fgetc(f);
        int count = 0;

        while ((ch != '\n') && (ch != EOF)) {
            if (count == max_len - 2) {
                max_len += 100;
                char *tmp = realloc(*str, max_len);
                if (tmp == NULL) {
                    fprintf(stderr, "Error reallocating space for line buffer.");
                    res = 1;
                    break;
                } else {
                    *str = tmp;
                }
            }
            (*str)[count] = ch;
            count++;

            ch = fgetc(f);
        }
        //        if (ch == 10) {
        (*str)[count] = '\0';
        /*} else*/ if (ch == EOF) { res = 2; }
    }
    return res;
}

int comp_regexs(t_parsed *s, regex_t *compiled) {
    int res = 0;
    char err_msg[MAX_ERR_LENGTH];
    int count = s->expr_len;
    for (int i = 0; i < count; i++) {
        res = regcomp(&(compiled[i]), (s->expr)[i], (s->i_flag) ? REG_ICASE : 0);
        if (res) {
            regerror(res, &(compiled[i]), err_msg, MAX_ERR_LENGTH);
            printf("Error analyzing regular expression '%s': %s.\n", (s->expr)[i], err_msg);
            res = 1;
            break;
        }
    }
    return res;
}

int proc_file(FILE *f, regex_t *compiled, t_parsed *s, char *file_name) {
    int res = 0;
    int lines_count = 0;
    int match_count = 0;
    while (!feof(f)) {
        char *str = NULL;
        int read_res = read_line(f, &str);
        // printf("String: %s\n", str);
        if (read_res == 0 || read_res == 2) {
            lines_count++;
            res = proc_line(compiled, str, s, file_name, lines_count, &match_count);
            // printf("Res in proc_file: %d\n", res);
            free(str);
            if (res != 0) break;
        }
    }

    if (s->c_flag) {
        if (!(s->h_flag)) printf("%s:", file_name);
        printf("%d\n", match_count);
    } else if (s->l_flag && res == 3) {
        printf("%s\n", file_name);
    }
    return res;
}

int proc_line(regex_t *compiled, char *str, t_parsed *s, char *file_name, int line_number, int *match_count) {
    char err_msg[MAX_ERR_LENGTH];
    int res = 0;
    for (int i = 0; i < s->expr_len; i++) {
        res = regexec(&(compiled[i]), str, 0, NULL, 0);
        // printf("res: %d\nString: %s\nFile: %s\n", res, str, file_name);
        if ((res == 0 && !(s->v_flag)) || (res == REG_NOMATCH && s->v_flag && s->expr_len - i == 1)) {
            if (s->c_flag) {
                (*match_count)++;
                res = 0;  // Строка посчитана, остальные паттерны можно не проверять
            } else if (s->l_flag) {
                res = 3;  // Можно переходить к следующему файлу
                // printf("%s\n", file_name);
            } else if (s->o_flag) {
                proc_o_flag(str, s, file_name, line_number, compiled);
                res = 0;
            } else {
                // отправляем строку на вывод
                output_line(str, s, file_name, line_number);
                res = 0;
            }
            break;
        } else if (res == 0 && s->v_flag) {
            break;
        } else if (res != REG_NOMATCH) {
            regerror(res, &(compiled[i]), err_msg, MAX_ERR_LENGTH);
            printf("Error blia: %s.\n", err_msg);
            res = 1;
        } else {
            res = 0;
        }
    }
    return res;
}

int output_line(char *str, t_parsed *s, char *file_name, int line_number) {
    int res = 0;
    if (!(s->h_flag)) {
        printf("%s:", file_name);
    }
    if (s->n_flag) {
        printf("%d:", line_number);
    }
    printf("%s\n", str);
    return res;
}

int proc_o_flag(char *str, t_parsed *s, char *file_name, int line_number, regex_t *compiled) {
    int res = 0;
    int len = strlen(str);
    // printf("Str: %s\n", str);
    int f_started = 0;
    size_t nmatch = 1;
    regmatch_t pmatch[1];
    regoff_t off, end;
    for (int i = 0; i < s->expr_len; i++) {
        if (s->expr[i][0] != '\0') {
            res = regexec(&(compiled[i]), str, nmatch, pmatch, 0);
            if (res == 0) {
                // printf("Pattern: %s\n", s->expr[i]);
                if (!f_started) {
                    off = pmatch[0].rm_so;
                    end = pmatch[0].rm_eo;
                    f_started = 1;
                } else {
                    if (off > pmatch[0].rm_so) {
                        off = pmatch[0].rm_so;
                        end = pmatch[0].rm_eo;
                    } else if (off == pmatch[0].rm_so) {
                        if (end < pmatch[0].rm_eo) {
                            end = pmatch[0].rm_eo;
                        }
                    }
                }
            }
        }
    }
    if (f_started) {
        char *part = calloc(sizeof(char), (end - off + 1));
        strncpy(part, str + off, end - off);
        output_line(part, s, file_name, line_number);
        free(part);
        if (len - end > 0) proc_o_flag(str + end, s, file_name, line_number, compiled);
    }
    return res;
}
