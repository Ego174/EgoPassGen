/*
password_gen.c - Реализация генерации паролей с разбором аргументов

Хаиров Егор Вадимович
МК-101
*/

#include "password_gen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>

// Определение типов и структур
typedef enum {
    ALPHABET_TYPE_STRING,
    ALPHABET_TYPE_CATEGORIES
} AlphabetType;

typedef struct {
    int min_len;
    int max_len;
    int fixed_len;
    int count;
    AlphabetType alphabet_type;
    char *alphabet_str;
    char categories[5];
    double *probs;
    int probs_count;
    char *delimiters;
    bool has_minl;
    bool has_maxl;
    bool has_fixed_len;
    bool has_count;
    bool has_alphabet;
    bool has_categories;
    bool has_probs;
} Options;

// Инициализация опций значениями по умолчанию
static int options_init(Options *opts) {
    memset(opts, 0, sizeof(Options));
    opts->fixed_len = -1;
    opts->count = 1;
    opts->delimiters = strdup(":=");
    if(!opts->delimiters) {
        return -1;
    }
    return 0;
}

// Освобождение динамической памяти
static void options_free(Options *opts) {
    free(opts->alphabet_str);
    free(opts->probs);
    free(opts->delimiters);
}

// Проверка, является ли символ допустимым разделителем
static bool is_delimiter(const Options *opts, char c) {
    return strchr(opts->delimiters, c) != NULL;
}

// Добавление разделителя (для -d)
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

// Замена разделителей (для -D)
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

// Проверка, является ли строка целым неотрицательным числом
static bool is_positive_integer(const char *s) {
    if(!s || *s == '\0') return false;
    while(*s) {
        if(*s < '0' || *s > '9') return false;
        s++;
    }
    return true;
}

// Извлекает значение для опции. Возвращает указатель на значение и сдвигает индекс.
// Если ошибка – возвращает NULL и устанавливает *next_i = i.
static const char *extract_value(Options *opts, int argc, char **argv, int i, int *next_i) {
    const char *arg = argv[i];
    if(arg[0] != '-') {
        *next_i = i;
        return NULL;
    }
    const char *opt = arg + 1; // имя опции без '-'

    // Ищем разделитель в arg
    char *delim_pos = NULL;
    for(const char *d = opts->delimiters; *d; ++d) {
        char *pos = strchr(arg, *d);
        if(pos && (!delim_pos || pos < delim_pos)) {
            delim_pos = pos;
        }
    }
    if(delim_pos) {
        // Есть разделитель, значение после него
        *next_i = i;
        return delim_pos + 1;
    } else {
        // Нет разделителя, проверяем следующий аргумент
        if(i + 1 < argc) {
            const char *next = argv[i + 1];
            if(next[0] != '-') {
                // Следующий аргумент не начинается с '-' – считаем его значением
                *next_i = i + 1;
                return next;
            }
        }
        // Значения нет
        *next_i = i;
        return NULL;
    }
}

static int parse_options(int argc, char **argv, Options *opts) {
    int i = 1;
    while(i < argc) {
        const char *arg = argv[i];
        if(arg[0] != '-') {
            i++;
            continue;
        }
        const char *opt_name = arg + 1;

        // -d и -D (изменение разделителей)
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

        // Извлекаем значение для остальных опций
        int next_i;
        const char *value = extract_value(opts, argc, argv, i, &next_i);

        // Обработка -minl
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
                // есть аргумент – используем как алфавит
                opts->alphabet_type = ALPHABET_TYPE_STRING;
                opts->alphabet_str = strdup(value);
                if(!opts->alphabet_str) {
                    fprintf(stderr, "Error: Memory allocation for alphabet\n");
                    return -1;
                }
                i = next_i + 1;
            } else {
                // без аргумента – позже установим алфавит по умолчанию
                opts->alphabet_type = ALPHABET_TYPE_STRING;
                opts->alphabet_str = NULL;
                i = next_i + 1; // next_i == i
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
            // Проверка, что все символы из множества {a,A,D,S}
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

        // -P (вероятности)
        if(strcmp(opt_name, "P") == 0) {
            if(opts->has_probs) {
                fprintf(stderr, "Error: Option -P repeated\n");
                return -1;
            }
            if(!value || strlen(value) == 0) {
                fprintf(stderr, "Error: Option -P requires a list of probabilities\n");
                return -1;
            }
            // Разбираем список чисел, разделённых запятыми
            const char *s = value;
            int count = 0;
            // подсчёт чисел
            const char *t = s;
            int in_num = 0;
            while(*t) {
                if(*t == ',') {
                    if(in_num) { count++; in_num = 0; }
                } else if(*t >= '0' && *t <= '9' || *t == '.' || *t == '-') {
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

            // парсим числа
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

    // --- Пост-разбор: проверки совместимости и установка умолчаний ---

    // Если задана -minl, должна быть -maxl и наоборот
    if(opts->has_minl && !opts->has_maxl) {
        fprintf(stderr, "Error: -minl requires -maxl\n");
        return -1;
    }
    if(opts->has_maxl && !opts->has_minl) {
        fprintf(stderr, "Error: -maxl requires -minl\n");
        return -1;
    }

    // -n несовместима с -minl/-maxl
    if(opts->has_fixed_len && (opts->has_minl || opts->has_maxl)) {
        fprintf(stderr, "Error: -n cannot be used with -minl or -maxl\n");
        return -1;
    }

    // -a и -C несовместимы
    if(opts->has_alphabet && opts->has_categories) {
        fprintf(stderr, "Error: -a and -C are incompatible\n");
        return -1;
    }

    // Если не задан ни -a, ни -C, используем алфавит по умолчанию (все печатные ASCII)
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
        opts->has_alphabet = true; // для внутреннего использования
    }

    // Если была -a без аргумента, устанавливаем алфавит по умолчанию
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

    // Проверка: длина алфавита > 0
    if(opts->alphabet_type == ALPHABET_TYPE_STRING) {
        if(!opts->alphabet_str || strlen(opts->alphabet_str) == 0) {
            fprintf(stderr, "Error: Alphabet must not be empty\n");
            return -1;
        }
    } else { // CATEGORIES
        if(strlen(opts->categories) == 0) {
            fprintf(stderr, "Error: Categories must not be empty\n");
            return -1;
        }
    }

    // Проверка корректности длины: если задана -n, она должна быть >= 1 (уже проверено)
    // Если заданы min/max, проверяем min <= max
    if(opts->has_minl && opts->has_maxl && opts->min_len > opts->max_len) {
        fprintf(stderr, "Error: min length cannot exceed max length\n");
        return -1;
    }

    // Проверка вероятностей
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
        // Если вероятностей меньше, оставшиеся будут заполнены равномерно позже
    }

    return 0;
}