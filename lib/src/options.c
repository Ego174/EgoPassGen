/*
options.c - Реализация инициализации и освобождения опций

Хаиров Егор Вадимович
МК-101
*/

#include "options.h"
#include <stdlib.h>
#include <string.h>

int options_init(Options *opts) {
    // Зануляем всю структуру
    memset(opts, 0, sizeof(Options));
    // Устанавливаем значения по умолчанию
    opts->fixed_len = -1;          // -1 означает "не задана"
    opts->count = 1;               // по умолчанию генерируем один пароль
    opts->delimiters = strdup(":="); // стандартные разделители
    if(!opts->delimiters) {
        return -1; // ошибка выделения памяти
    }
    return 0;
}

void options_free(Options *opts) {
    // Освобождаем динамическую память (free(NULL) безопасна)
    free(opts->alphabet_str);
    free(opts->probs);
    free(opts->delimiters);
    // Не обнуляем opts, так как структура будет уничтожена
}