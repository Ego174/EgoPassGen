/*
test_lib.c - модуль проверки библиотеки. 

Хаиров Егор Вадимович
МК-101
*/

#include "unity.h"

void setUp(void) {
    // Вызывается перед каждым тестом (можно оставить пустым)
}

void tearDown(void) {
    // Вызывается после каждого теста (можно оставить пустым)
}

int main(void) {
    UNITY_BEGIN();
    return UNITY_END();
}

