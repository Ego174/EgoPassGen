/*
options.h - Внутреннее определение структуры опций и функций инициализации/освобождения

Хаиров Егор Вадимович
МК-101
*/

#pragma once

#include <stdbool.h>

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

int options_init(Options *opts);
void options_free(Options *opts);