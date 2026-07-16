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