/*
options.h - Определение структуры Options и функций управления

Хаиров Егор Вадимович
МК-101
*/

#pragma once

#include <stdbool.h>
#include <stddef.h>

// Тип задания алфавита: строка символов или набор категорий (aADS)
typedef enum {
    ALPHABET_TYPE_STRING,    // явная строка через -a
    ALPHABET_TYPE_CATEGORIES // набор категорий через -C
} AlphabetType;

// Структура, хранящая все параметры, извлечённые из аргументов командной строки
typedef struct {
    // Длины паролей
    int min_len;          // минимальная длина (из -minl)
    int max_len;          // максимальная длина (из -maxl)
    int fixed_len;        // фиксированная длина (из -n), -1 если не задана
    int count;            // количество паролей (из -c), по умолчанию 1

    // Алфавит
    AlphabetType alphabet_type;   // тип задания алфавита
    char *alphabet_str;           // строка символов (если тип STRING), динамическая память
    char categories[5];           // категории (если тип CATEGORIES), например "aD"

    // Вероятности
    double *probs;          // массив вероятностей, размер probs_count
    int probs_count;        // количество элементов в массиве probs

    // Разделители
    char *delimiters;       // строка допустимых разделителей, по умолчанию ":="

    // Флаги наличия опций (используются для проверки повторов и совместимости)
    bool has_minl;
    bool has_maxl;
    bool has_fixed_len;
    bool has_count;
    bool has_alphabet;      // опция -a присутствует (с аргументом или без)
    bool has_categories;    // опция -C присутствует
    bool has_probs;         // опция -P присутствует
} Options;

// Инициализирует структуру Options значениями по умолчанию
// Возвращает 0 при успехе, -1 при ошибке выделения памяти для разделителей
int options_init(Options *opts);

// Освобождает динамическую память, занятую полями структуры
void options_free(Options *opts);