/*
parser.h - Объявление функции разбора аргументов

Хаиров Егор Вадимович
МК-101
*/

#pragma once

#include "options.h"

// Разбор аргументов командной строки, заполнение структуры Options
// Возвращает 0 при успехе, -1 при ошибке (сообщение выводится в stderr)
int parse_options(int argc, char **argv, Options *opts);