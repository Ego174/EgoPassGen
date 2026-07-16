/*
parser.c - Реализация разбора аргументов командной строки

Хаиров Егор Вадимович
МК-101
*/

#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

// Вспомогательные функции (статик)

static bool is_delimiter(const Options *opts, char c) {
    return strchr(opts->delimiters, c) != NULL;
}

static int add_delimiter(Options *opts, char c) {
    size_t len = strlen(opts->delimiters);
    char *new_del = realloc(opts->delimiters, len + 2);
    if(!new_del) {
        return -1;
    }
    opts->delimiters = new_del;
    opts->delimiters[len] = c;
    opts->delimiters[len + 1] = '\0';
    return 0;
}

static int set_delimiters(Options *opts, char c) {
    char *new_del = malloc(2);
    if(!new_del) {
        return -1;
    }
    free(opts->delimiters);
    opts->delimiters = new_del;
    opts->delimiters[0] = c;
    opts->delimiters[1] = '\0';
    return 0;
}

static bool is_positive_integer(const char *s) {
    if(!s || *s == '\0') return false;
    while(*s) {
        if(*s < '0' || *s > '9') return false;
        s++;
    }
    return true;
}

static const char *extract_value(Options *opts, int argc, char **argv, int i, int *next_i) {
    const char *arg = argv[i];
    if(arg[0] != '-') {
        *next_i = i;
        return NULL;
    }

    // Ищем разделитель
    char *delim_pos = NULL;
    for(const char *d = opts->delimiters; *d; ++d) {
        char *pos = strchr(arg, *d);
        if(pos && (!delim_pos || pos < delim_pos)) {
            delim_pos = pos;
        }
    }
    if(delim_pos) {
        *next_i = i;
        return delim_pos + 1;
    }

    // Разделителя нет, пробуем выделить имя опции (буквенная часть)
    const char *opt_part = arg + 1;
    int len = 0;
    while(opt_part[len] && isalpha(opt_part[len])) len++;
    // Если после буквенной части есть символы, то это значение
    if(opt_part[len] != '\0') {
        *next_i = i;
        return opt_part + len;
    }

    // Иначе смотрим следующий аргумент
    if(i + 1 < argc) {
        const char *next = argv[i + 1];
        if(next[0] != '-') {
            *next_i = i + 1;
            return next;
        }
    }
    *next_i = i;
    return NULL;
}

// Основная функция парсинга
int parse_options(int argc, char **argv, Options *opts) {
    int i = 1;
    while(i < argc) {
        const char *arg = argv[i];
        if(arg[0] != '-') {
            i++;
            continue;
        }
        const char *opt_name = arg + 1;

        // -d
        if(strcmp(opt_name, "d") == 0) {
            if(i + 1 >= argc) {
                fprintf(stderr, "Error: Option -d requires a character argument\n");
                return -1;
            }
            const char *sym = argv[i + 1];
            if(sym[0] == '-') {
                fprintf(stderr, "Error: Option -d requires a character, not an option\n");
                return -1;
            }
            if(strlen(sym) != 1) {
                fprintf(stderr, "Error: Option -d expects exactly one character\n");
                return -1;
            }
            if(add_delimiter(opts, sym[0]) != 0) {
                fprintf(stderr, "Error: Memory allocation failed for delimiter\n");
                return -1;
            }
            i += 2;
            continue;
        }

        // -D
        if(strcmp(opt_name, "D") == 0) {
            if(i + 1 >= argc) {
                fprintf(stderr, "Error: Option -D requires a character argument\n");
                return -1;
            }
            const char *sym = argv[i + 1];
            if(sym[0] == '-') {
                fprintf(stderr, "Error: Option -D requires a character, not an option\n");
                return -1;
            }
            if(strlen(sym) != 1) {
                fprintf(stderr, "Error: Option -D expects exactly one character\n");
                return -1;
            }
            if(set_delimiters(opts, sym[0]) != 0) {
                fprintf(stderr, "Error: Memory allocation failed for delimiter\n");
                return -1;
            }
            i += 2;
            continue;
        }

        int next_i;
        const char *value = extract_value(opts, argc, argv, i, &next_i);

        // -minl
        if(strcmp(opt_name, "minl") == 0) {
            if(opts->has_minl) {
                fprintf(stderr, "Error: Option -minl repeated\n");
                return -1;
            }
            if(!value) {
                fprintf(stderr, "Error: Option -minl requires a numeric argument\n");
                return -1;
            }
            if(!is_positive_integer(value)) {
                fprintf(stderr, "Error: Option -minl expects a positive integer\n");
                return -1;
            }
            opts->min_len = atoi(value);
            if(opts->min_len <= 0) {
                fprintf(stderr, "Error: Minimum length must be positive\n");
                return -1;
            }
            opts->has_minl = true;
            i = next_i + 1;
            continue;
        }

        // -maxl
        if(strcmp(opt_name, "maxl") == 0) {
            if(opts->has_maxl) {
                fprintf(stderr, "Error: Option -maxl repeated\n");
                return -1;
            }
            if(!value) {
                fprintf(stderr, "Error: Option -maxl requires a numeric argument\n");
                return -1;
            }
            if(!is_positive_integer(value)) {
                fprintf(stderr, "Error: Option -maxl expects a positive integer\n");
                return -1;
            }
            opts->max_len = atoi(value);
            if(opts->max_len <= 0) {
                fprintf(stderr, "Error: Maximum length must be positive\n");
                return -1;
            }
            opts->has_maxl = true;
            i = next_i + 1;
            continue;
        }

        // -n
        if(strcmp(opt_name, "n") == 0) {
            if(opts->has_fixed_len) {
                fprintf(stderr, "Error: Option -n repeated\n");
                return -1;
            }
            if(!value) {
                fprintf(stderr, "Error: Option -n requires a numeric argument\n");
                return -1;
            }
            if(!is_positive_integer(value)) {
                fprintf(stderr, "Error: Option -n expects a positive integer\n");
                return -1;
            }
            opts->fixed_len = atoi(value);
            if(opts->fixed_len <= 0) {
                fprintf(stderr, "Error: Length must be positive\n");
                return -1;
            }
            opts->has_fixed_len = true;
            i = next_i + 1;
            continue;
        }

        // -c
        if(strcmp(opt_name, "c") == 0) {
            if(opts->has_count) {
                fprintf(stderr, "Error: Option -c repeated\n");
                return -1;
            }
            if(!value) {
                fprintf(stderr, "Error: Option -c requires a numeric argument\n");
                return -1;
            }
            if(!is_positive_integer(value)) {
                fprintf(stderr, "Error: Option -c expects a positive integer\n");
                return -1;
            }
            opts->count = atoi(value);
            if(opts->count <= 0) {
                fprintf(stderr, "Error: Count must be positive\n");
                return -1;
            }
            opts->has_count = true;
            i = next_i + 1;
            continue;
        }

        // -a
        if(strcmp(opt_name, "a") == 0) {
            if(opts->has_alphabet) {
                fprintf(stderr, "Error: Option -a repeated\n");
                return -1;
            }
            opts->has_alphabet = true;
            if(value && strlen(value) > 0) {
                opts->alphabet_type = ALPHABET_TYPE_STRING;
                opts->alphabet_str = strdup(value);
                if(!opts->alphabet_str) {
                    fprintf(stderr, "Error: Memory allocation for alphabet\n");
                    return -1;
                }
                i = next_i + 1;
            } else {
                opts->alphabet_type = ALPHABET_TYPE_STRING;
                opts->alphabet_str = NULL;
                i = next_i + 1;
            }
            continue;
        }

        // -C
        if(strcmp(opt_name, "C") == 0) {
            if(opts->has_categories) {
                fprintf(stderr, "Error: Option -C repeated\n");
                return -1;
            }
            if(!value || strlen(value) == 0) {
                fprintf(stderr, "Error: Option -C requires an argument (aADS)\n");
                return -1;
            }
            const char *p = value;
            while(*p) {
                if(*p != 'a' && *p != 'A' && *p != 'D' && *p != 'S') {
                    fprintf(stderr, "Error: Option -C expects only characters a, A, D, S\n");
                    return -1;
                }
                p++;
            }
            if(strlen(value) > 4) {
                fprintf(stderr, "Error: Option -C can have at most 4 characters\n");
                return -1;
            }
            opts->alphabet_type = ALPHABET_TYPE_CATEGORIES;
            strcpy(opts->categories, value);
            opts->has_categories = true;
            i = next_i + 1;
            continue;
        }

        // -P
        if(strcmp(opt_name, "P") == 0) {
            if(opts->has_probs) {
                fprintf(stderr, "Error: Option -P repeated\n");
                return -1;
            }
            if(!value || strlen(value) == 0) {
                fprintf(stderr, "Error: Option -P requires a list of probabilities\n");
                return -1;
            }
            const char *s = value;
            int count = 0;
            const char *t = s;
            int in_num = 0;
            while(*t) {
                if(*t == ',') {
                    if(in_num) { count++; in_num = 0; }
                } else if((*t >= '0' && *t <= '9') || *t == '.' || *t == '-') {
                    in_num = 1;
                } else {
                    fprintf(stderr, "Error: Invalid character in probability list\n");
                    return -1;
                }
                t++;
            }
            if(in_num) count++;
            if(count == 0) {
                fprintf(stderr, "Error: No probabilities found\n");
                return -1;
            }

            opts->probs = malloc(count * sizeof(double));
            if(!opts->probs) {
                fprintf(stderr, "Error: Memory allocation for probabilities\n");
                return -1;
            }
            opts->probs_count = count;

            int idx = 0;
            const char *start = s;
            while(*s) {
                if(*s == ',') {
                    if(s > start) {
                        char *end;
                        double val = strtod(start, &end);
                        if(end == start || val < 0) {
                            fprintf(stderr, "Error: Invalid probability value\n");
                            free(opts->probs);
                            opts->probs = NULL;
                            return -1;
                        }
                        opts->probs[idx++] = val;
                    }
                    start = s + 1;
                }
                s++;
            }
            if(s > start) {
                char *end;
                double val = strtod(start, &end);
                if(end == start || val < 0) {
                    fprintf(stderr, "Error: Invalid probability value\n");
                    free(opts->probs);
                    opts->probs = NULL;
                    return -1;
                }
                opts->probs[idx++] = val;
            }
            opts->has_probs = true;
            i = next_i + 1;
            continue;
        }

        // Неизвестная опция – игнорируем
        i++;
    }

    // Проверки совместимости
    if(opts->has_minl && !opts->has_maxl) {
        fprintf(stderr, "Error: -minl requires -maxl\n");
        return -1;
    }
    if(opts->has_maxl && !opts->has_minl) {
        fprintf(stderr, "Error: -maxl requires -minl\n");
        return -1;
    }
    if(opts->has_fixed_len && (opts->has_minl || opts->has_maxl)) {
        fprintf(stderr, "Error: -n cannot be used with -minl or -maxl\n");
        return -1;
    }
    if(opts->has_alphabet && opts->has_categories) {
        fprintf(stderr, "Error: -a and -C are incompatible\n");
        return -1;
    }

    // Установка алфавита по умолчанию, если не задан
    if(!opts->has_alphabet && !opts->has_categories) {
        char default_alphabet[95];
        int idx = 0;
        for(int c = 33; c <= 126; ++c) {
            default_alphabet[idx++] = (char)c;
        }
        default_alphabet[idx] = '\0';
        opts->alphabet_type = ALPHABET_TYPE_STRING;
        opts->alphabet_str = strdup(default_alphabet);
        if(!opts->alphabet_str) {
            fprintf(stderr, "Error: Memory allocation for default alphabet\n");
            return -1;
        }
        opts->has_alphabet = true;
    }

    // Если -a без аргумента – ставим умолчание
    if(opts->has_alphabet && opts->alphabet_str == NULL && opts->alphabet_type == ALPHABET_TYPE_STRING) {
        char default_alphabet[95];
        int idx = 0;
        for(int c = 33; c <= 126; ++c) {
            default_alphabet[idx++] = (char)c;
        }
        default_alphabet[idx] = '\0';
        opts->alphabet_str = strdup(default_alphabet);
        if(!opts->alphabet_str) {
            fprintf(stderr, "Error: Memory allocation for default alphabet\n");
            return -1;
        }
    }

    if(opts->alphabet_type == ALPHABET_TYPE_STRING) {
        if(!opts->alphabet_str || strlen(opts->alphabet_str) == 0) {
            fprintf(stderr, "Error: Alphabet must not be empty\n");
            return -1;
        }
    } else {
        if(strlen(opts->categories) == 0) {
            fprintf(stderr, "Error: Categories must not be empty\n");
            return -1;
        }
    }

    if(opts->has_minl && opts->has_maxl && opts->min_len > opts->max_len) {
        fprintf(stderr, "Error: min length cannot exceed max length\n");
        return -1;
    }

    if(opts->has_probs) {
        int expected = 0;
        if(opts->alphabet_type == ALPHABET_TYPE_STRING) {
            expected = strlen(opts->alphabet_str);
        } else {
            expected = strlen(opts->categories);
        }
        if(opts->probs_count > expected) {
            fprintf(stderr, "Error: Too many probabilities, expected at most %d\n", expected);
            return -1;
        }
    }

    return 0;
}