/*
parser.h - Внутренний заголовок для разбора аргументов

Хаиров Егор Вадимович
МК-101
*/

#pragma once

#include "options.h"

int parse_options(int argc, char **argv, Options *opts);