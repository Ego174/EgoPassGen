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
    if(!opts->delimiters) return -1;
    return 0;
}

// Освобождение динамической памяти
static void options_free(Options *opts) {
    free(opts->alphabet_str);
    free(opts->probs);
    free(opts->delimiters);
}