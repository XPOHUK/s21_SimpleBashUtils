#ifndef SRC_GREP_S21_GREP_H_
#define SRC_GREP_S21_GREP_H_

#include <regex.h>
#include <stdio.h>

typedef struct {
    int e_flag;    // -e; expression; may be many
    int i_flag;    // -i; ignore case
    int v_flag;    // -v; v_flag match
    int c_flag;    // -c; only matched line count
    int l_flag;    // -l; only matched file names
    int n_flag;    // -n; print line numbers
    int h_flag;    // -h; no print file names
    int s_flag;    // -s; no error message about nonexistent or unreadable files
    int f_flag;    // -f; expressions from a file; may be many
    int o_flag;    // -o; only matched parts of line
    char **expr;   // patterns
    int expr_len;  // count of patterns
} t_parsed;

int check_opt(t_parsed *s, int c, char *optarg);
int check_file(char *file_name, int silent);
// int output(FILE *f);
int read_expr(t_parsed *s, char *filename);
int read_line(FILE *f, char **str);
int comp_regexs(t_parsed *s, regex_t *compiled);
int proc_file(FILE *f, regex_t *compiled, t_parsed *s, char *file_name);
int proc_line(regex_t *compiled, char *str, t_parsed *s, char *file_name, int line_number, int *match_count);
int proc_o_flag(char *str, t_parsed *s, char *file_name, int line_number, regex_t *compiled);
int output_line(char *str, t_parsed *s, char *file_name, int line_number);

void test_flags(t_parsed *s);

#endif  // SRC_GREP_S21_GREP_H_
