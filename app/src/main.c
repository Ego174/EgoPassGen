/*
main.c - Точка входа программы генерации паролей

Хаиров Егор Вадимович
МК-101
*/

#include "password_gen.h"

int main(int argc, char **argv) {
    return generate_passwords_from_args(argc, argv);
}