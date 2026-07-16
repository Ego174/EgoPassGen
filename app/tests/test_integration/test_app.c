/*
test_app.c - Интеграционные тесты основной программы

Хаиров Егор Вадимович
МК-101
*/

#include "unity.h"
#include "password_gen.h"

void setUp(void) {}
void tearDown(void) {}

// 1. Корректный вызов с -minl и -maxl
void test_app_valid_minl_maxl(void) {
    char *argv[] = {"prog", "-minl", "5", "-maxl", "10"};
    int ret = generate_passwords_from_args(4, argv);
    TEST_ASSERT_EQUAL_INT(0, ret);
}

// 2. Корректный вызов с -n
void test_app_valid_n(void) {
    char *argv[] = {"prog", "-n", "8"};
    int ret = generate_passwords_from_args(3, argv);
    TEST_ASSERT_EQUAL_INT(0, ret);
}

// 3. Корректный вызов с -a
void test_app_valid_a(void) {
    char *argv[] = {"prog", "-a", "abc"};
    int ret = generate_passwords_from_args(3, argv);
    TEST_ASSERT_EQUAL_INT(0, ret);
}

// 4. Корректный вызов с -C
void test_app_valid_C(void) {
    char *argv[] = {"prog", "-C", "aD"};
    int ret = generate_passwords_from_args(3, argv);
    TEST_ASSERT_EQUAL_INT(0, ret);
}

// 5. Корректный вызов с -P
void test_app_valid_P(void) {
    char *argv[] = {"prog", "-a", "abcd", "-P", "0.1,0.2,0.3"};
    int ret = generate_passwords_from_args(5, argv);
    TEST_ASSERT_EQUAL_INT(0, ret);
}

// 6. Несовместимость -n и -minl
void test_app_incompatible_n_minl(void) {
    char *argv[] = {"prog", "-n", "8", "-minl", "5"};
    int ret = generate_passwords_from_args(5, argv);
    TEST_ASSERT_EQUAL_INT(-1, ret);
}

// 7. Несовместимость -a и -C
void test_app_incompatible_a_C(void) {
    char *argv[] = {"prog", "-a", "abc", "-C", "aD"};
    int ret = generate_passwords_from_args(5, argv);
    TEST_ASSERT_EQUAL_INT(-1, ret);
}

// 8. Ошибка -minl без -maxl
void test_app_minl_without_maxl(void) {
    char *argv[] = {"prog", "-minl", "5"};
    int ret = generate_passwords_from_args(3, argv);
    TEST_ASSERT_EQUAL_INT(-1, ret);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_app_valid_minl_maxl);
    RUN_TEST(test_app_valid_n);
    RUN_TEST(test_app_valid_a);
    RUN_TEST(test_app_valid_C);
    RUN_TEST(test_app_valid_P);
    RUN_TEST(test_app_incompatible_n_minl);
    RUN_TEST(test_app_incompatible_a_C);
    RUN_TEST(test_app_minl_without_maxl);
    return UNITY_END();
}