/*
parser.c - Реализация разбора аргументов командной строки

Хаиров Егор Вадимович
МК-101
*/

#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

static bool is_delimiter(const Options *opts, char c) {
    return strchr(opts->delimiters, c) != NULL;
}

static int add_delimiter(Options *opts, char c) {
    size_t len = strlen(opts->delimiters);
    char *new_del = realloc(opts->delimiters, len + 2);
    if(!new_del) return -1;
    opts->delimiters = new_del;
    opts->delimiters[len] = c;
    opts->delimiters[len + 1] = '\0';
    return 0;
}

static int set_delimiters(Options *opts, char c) {
    char *new_del = malloc(2);
    if(!new_del) return -1;
    free(opts->delimiters);
    opts->delimiters = new_del;
    opts->delimiters[0] = c;
    opts->delimiters[1] = '\0';
    return 0;
}

static bool is_positive_integer(const char *s) {
    if(!s || *s == '\0') return false;
    while(*s) {
        if(!isdigit(*s)) return false;
        s++;
    }
    return true;
}

// Извлекает имя опции и возможное значение из аргумента.
// Возвращает указатель на начало имени (внутри arg) или NULL.
// *out_value: если есть значение в этом же аргументе – указатель на него, иначе NULL.
// *name_len: длина имени опции.
static const char *extract_option_name_and_value(const Options *opts, const char *arg,
                                                 const char **out_value, int *name_len) {
    if(arg[0] != '-') return NULL;
    const char *p = arg + 1;
    int len = 0;
    while(p[len] && isalpha(p[len])) len++;
    if(len == 0) return NULL;
    *name_len = len;
    if(p[len] != '\0') {
        const char *val_start = p + len;
        // Если первый символ после имени – разделитель, пропускаем его
        if(is_delimiter(opts, val_start[0])) {
            val_start++;
        }
        *out_value = val_start;
    } else {
        *out_value = NULL;
    }
    return p;
}

int parse_options(int argc, char **argv, Options *opts) {
    int i = 1;
    while(i < argc) {
        const char *arg = argv[i];
        if(arg[0] != '-') {
            i++;
            continue;
        }

        int name_len;
        const char *value;
        const char *opt_start = extract_option_name_and_value(opts, arg, &value, &name_len);
        if(!opt_start) {
            i++;
            continue;
        }

        char opt_name[64];
        if(name_len >= (int)sizeof(opt_name)) name_len = (int)sizeof(opt_name) - 1;
        strncpy(opt_name, opt_start, name_len);
        opt_name[name_len] = '\0';

        // Если значение не было в текущем аргументе, попробуем взять следующий аргумент (если он не опция)
        int next_delta = 0;
        if(!value) {
            if(i + 1 < argc && argv[i + 1][0] != '-') {
                value = argv[i + 1];
                next_delta = 1;
            }
        }

        // --- Обработка -d (добавление разделителя) ---
        if(strcmp(opt_name, "d") == 0) {
            const char *delim_char = NULL;
            int delta = 0;
            if(value && strlen(value) == 1) {
                delim_char = value;
                delta = 1; // съели текущий аргумент
            } else if(!value && i + 1 < argc && argv[i + 1][0] != '-') {
                if(strlen(argv[i + 1]) == 1) {
                    delim_char = argv[i + 1];
                    delta = 2; // съели текущий и следующий
                }
            }
            if(!delim_char) {
                fprintf(stderr, "Error: Option -d requires a character argument\n");
                return -1;
            }
            if(add_delimiter(opts, delim_char[0]) != 0) {
                fprintf(stderr, "Error: Memory allocation for delimiter\n");
                return -1;
            }
            i += delta;
            continue;
        }

        // --- Обработка -D (замена разделителей) ---
        if(strcmp(opt_name, "D") == 0) {
            const char *delim_char = NULL;
            int delta = 0;
            if(value && strlen(value) == 1) {
                delim_char = value;
                delta = 1;
            } else if(!value && i + 1 < argc && argv[i + 1][0] != '-') {
                if(strlen(argv[i + 1]) == 1) {
                    delim_char = argv[i + 1];
                    delta = 2;
                }
            }
            if(!delim_char) {
                fprintf(stderr, "Error: Option -D requires a character argument\n");
                return -1;
            }
            if(set_delimiters(opts, delim_char[0]) != 0) {
                fprintf(stderr, "Error: Memory allocation for delimiter\n");
                return -1;
            }
            i += delta;
            continue;
        }

        // --- Определяем, известна ли опция и требует ли аргумент ---
        bool known = false;
        bool requires_arg = false;

        if(strcmp(opt_name, "minl") == 0 ||
           strcmp(opt_name, "maxl") == 0 ||
           strcmp(opt_name, "n") == 0 ||
           strcmp(opt_name, "c") == 0 ||
           strcmp(opt_name, "C") == 0 ||
           strcmp(opt_name, "P") == 0) {
            known = true;
            requires_arg = true;
        } else if(strcmp(opt_name, "a") == 0) {
            known = true;
            requires_arg = false; // может быть без аргумента
        }

        // Неизвестная опция – игнорируем
        if(!known) {
            i++;
            continue;
        }

        // Если опция требует аргумент, но его нет – ошибка
        if(requires_arg && !value) {
            fprintf(stderr, "Error: Option -%s requires an argument\n", opt_name);
            return -1;
        }

        // --- Обработка известных опций (кроме -d и -D) ---
        if(strcmp(opt_name, "minl") == 0) {
            if(opts->has_minl) {
                fprintf(stderr, "Error: Option -minl repeated\n");
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
            i += 1 + next_delta;
            continue;
        }

        if(strcmp(opt_name, "maxl") == 0) {
            if(opts->has_maxl) {
                fprintf(stderr, "Error: Option -maxl repeated\n");
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
            i += 1 + next_delta;
            continue;
        }

        if(strcmp(opt_name, "n") == 0) {
            if(opts->has_fixed_len) {
                fprintf(stderr, "Error: Option -n repeated\n");
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
            i += 1 + next_delta;
            continue;
        }

        if(strcmp(opt_name, "c") == 0) {
            if(opts->has_count) {
                fprintf(stderr, "Error: Option -c repeated\n");
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
            i += 1 + next_delta;
            continue;
        }

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
                i += 1 + next_delta;
            } else {
                opts->alphabet_type = ALPHABET_TYPE_STRING;
                opts->alphabet_str = NULL;
                i += 1 + next_delta;
            }
            continue;
        }

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
            i += 1 + next_delta;
            continue;
        }

        if(strcmp(opt_name, "P") == 0) {
            if(opts->has_probs) {
                fprintf(stderr, "Error: Option -P repeated\n");
                return -1;
            }
            if(!value || strlen(value) == 0) {
                fprintf(stderr, "Error: Option -P requires a list of probabilities\n");
                return -1;
            }
            // Подсчёт чисел
            const char *s = value;
            int count = 0;
            int in_num = 0;
            while(*s) {
                if(*s == ',') {
                    if(in_num) { count++; in_num = 0; }
                } else if(*s >= '0' && *s <= '9' || *s == '.' || *s == '-') {
                    in_num = 1;
                } else {
                    fprintf(stderr, "Error: Invalid character in probability list\n");
                    return -1;
                }
                s++;
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
            const char *start = value;
            s = value;
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
            i += 1 + next_delta;
            continue;
        }

        // Если сюда дошли – что-то пошло не так, но игнорируем
        i++;
    }

    // --- Пост-разбор ---
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

    // Если -a была без аргумента – ставим алфавит по умолчанию
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
        int expected = (opts->alphabet_type == ALPHABET_TYPE_STRING) ? strlen(opts->alphabet_str) : strlen(opts->categories);
        if(opts->probs_count > expected) {
            fprintf(stderr, "Error: Too many probabilities, expected at most %d\n", expected);
            return -1;
        }
    }

    return 0;
}