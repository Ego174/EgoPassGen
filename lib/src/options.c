/*
options.c - Реализация инициализации и освобождения структуры опций

Хаиров Егор Вадимович
МК-101
*/

#include "options.h"
#include <stdlib.h>
#include <string.h>

int options_init(Options *opts) {
    memset(opts, 0, sizeof(Options));
    opts->fixed_len = -1;
    opts->count = 1;
    opts->delimiters = strdup(":=");
    if(!opts->delimiters) {
        return -1;
    }
    return 0;
}

void options_free(Options *opts) {
    free(opts->alphabet_str);
    free(opts->probs);
    free(opts->delimiters);
}