/*
test_parser.c - Unit-тесты для парсера аргументов

Хаиров Егор Вадимович
МК-101
*/

#include "unity.h"
#include "options.h"
#include "parser.h"
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

// 1. Корректный парсинг -minl и -maxl
void test_parse_minl_maxl(void) {
    Options opts;
    options_init(&opts);
    char *argv[] = {"prog", "-minl", "10", "-maxl", "20"};
    int ret = parse_options(5, argv, &opts);          // argc = 5
    TEST_ASSERT_EQUAL_INT(0, ret);
    TEST_ASSERT_TRUE(opts.has_minl);
    TEST_ASSERT_TRUE(opts.has_maxl);
    TEST_ASSERT_EQUAL_INT(10, opts.min_len);
    TEST_ASSERT_EQUAL_INT(20, opts.max_len);
    options_free(&opts);
}

// 2. Корректный парсинг -n
void test_parse_n(void) {
    Options opts;
    options_init(&opts);
    char *argv[] = {"prog", "-n", "15"};
    int ret = parse_options(3, argv, &opts);
    TEST_ASSERT_EQUAL_INT(0, ret);
    TEST_ASSERT_TRUE(opts.has_fixed_len);
    TEST_ASSERT_EQUAL_INT(15, opts.fixed_len);
    options_free(&opts);
}

// 3. Ошибка: -n с -minl/-maxl
void test_parse_n_incompatible_with_minmax(void) {
    Options opts;
    options_init(&opts);
    char *argv[] = {"prog", "-n", "10", "-minl", "5", "-maxl", "15"};
    int ret = parse_options(7, argv, &opts);
    TEST_ASSERT_EQUAL_INT(-1, ret);
    options_free(&opts);
}

// 4. Корректный -a с аргументом
void test_parse_alphabet_a_with_arg(void) {
    Options opts;
    options_init(&opts);
    char *argv[] = {"prog", "-a", "abc123"};
    int ret = parse_options(3, argv, &opts);
    TEST_ASSERT_EQUAL_INT(0, ret);
    TEST_ASSERT_TRUE(opts.has_alphabet);
    TEST_ASSERT_EQUAL_INT(ALPHABET_TYPE_STRING, opts.alphabet_type);
    TEST_ASSERT_EQUAL_STRING("abc123", opts.alphabet_str);
    options_free(&opts);
}

// 5. Корректный -a без аргумента (должен установить алфавит по умолчанию)
void test_parse_alphabet_a_without_arg(void) {
    Options opts;
    options_init(&opts);
    char *argv[] = {"prog", "-a"};
    int ret = parse_options(2, argv, &opts);
    TEST_ASSERT_EQUAL_INT(0, ret);
    TEST_ASSERT_TRUE(opts.has_alphabet);
    TEST_ASSERT_EQUAL_INT(ALPHABET_TYPE_STRING, opts.alphabet_type);
    TEST_ASSERT_NOT_NULL(opts.alphabet_str);
    TEST_ASSERT_GREATER_THAN(0, strlen(opts.alphabet_str));
    options_free(&opts);
}

// 6. Корректный -C
void test_parse_categories_C(void) {
    Options opts;
    options_init(&opts);
    char *argv[] = {"prog", "-C", "aD"};
    int ret = parse_options(3, argv, &opts);
    TEST_ASSERT_EQUAL_INT(0, ret);
    TEST_ASSERT_TRUE(opts.has_categories);
    TEST_ASSERT_EQUAL_INT(ALPHABET_TYPE_CATEGORIES, opts.alphabet_type);
    TEST_ASSERT_EQUAL_STRING("aD", opts.categories);
    options_free(&opts);
}

// 7. Ошибка: -a и -C вместе
void test_parse_a_and_C_incompatible(void) {
    Options opts;
    options_init(&opts);
    char *argv[] = {"prog", "-a", "abc", "-C", "aD"};
    int ret = parse_options(5, argv, &opts);
    TEST_ASSERT_EQUAL_INT(-1, ret);
    options_free(&opts);
}

// 8. Корректный -P с вероятностями
void test_parse_probabilities(void) {
    Options opts;
    options_init(&opts);
    char *argv[] = {"prog", "-a", "abcd", "-P", "0.1,0.2,0.3"};
    int ret = parse_options(5, argv, &opts);
    TEST_ASSERT_EQUAL_INT(0, ret);
    TEST_ASSERT_TRUE(opts.has_probs);
    TEST_ASSERT_EQUAL_INT(3, opts.probs_count);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 0.1, opts.probs[0]);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 0.2, opts.probs[1]);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 0.3, opts.probs[2]);
    options_free(&opts);
}

// 9. Ошибка: слишком много вероятностей
void test_parse_too_many_probs(void) {
    Options opts;
    options_init(&opts);
    char *argv[] = {"prog", "-a", "ab", "-P", "0.1,0.2,0.3"};
    int ret = parse_options(5, argv, &opts);
    TEST_ASSERT_EQUAL_INT(-1, ret);
    options_free(&opts);
}

// 10. Корректный -d добавление разделителя и использование его (с парой -minl/-maxl)
void test_parse_delimiter_add_and_use(void) {
    Options opts;
    options_init(&opts);
    // используем разделитель '!' для -minl и -maxl
    char *argv[] = {"prog", "-d", "!", "-minl!10", "-maxl!20"};
    int ret = parse_options(5, argv, &opts);
    TEST_ASSERT_EQUAL_INT(0, ret);
    TEST_ASSERT_TRUE(strchr(opts.delimiters, '!') != NULL);
    TEST_ASSERT_TRUE(opts.has_minl);
    TEST_ASSERT_TRUE(opts.has_maxl);
    TEST_ASSERT_EQUAL_INT(10, opts.min_len);
    TEST_ASSERT_EQUAL_INT(20, opts.max_len);
    options_free(&opts);
}

// 11. Корректный -D замена разделителей и использование (с парой -minl/-maxl)
void test_parse_delimiter_replace_and_use(void) {
    Options opts;
    options_init(&opts);
    char *argv[] = {"prog", "-D", "#", "-minl#10", "-maxl#20"};
    int ret = parse_options(5, argv, &opts);
    TEST_ASSERT_EQUAL_INT(0, ret);
    TEST_ASSERT_EQUAL_STRING("#", opts.delimiters);
    TEST_ASSERT_TRUE(opts.has_minl);
    TEST_ASSERT_TRUE(opts.has_maxl);
    TEST_ASSERT_EQUAL_INT(10, opts.min_len);
    TEST_ASSERT_EQUAL_INT(20, opts.max_len);
    options_free(&opts);
}

// 12. Игнорирование неизвестных опций
void test_ignore_unknown_options(void) {
    Options opts;
    options_init(&opts);
    char *argv[] = {"prog", "-s", "-minl", "5", "-i", "-maxl", "15", "-x"};
    int ret = parse_options(8, argv, &opts);
    TEST_ASSERT_EQUAL_INT(0, ret);
    TEST_ASSERT_TRUE(opts.has_minl);
    TEST_ASSERT_TRUE(opts.has_maxl);
    TEST_ASSERT_EQUAL_INT(5, opts.min_len);
    TEST_ASSERT_EQUAL_INT(15, opts.max_len);
    options_free(&opts);
}

// 13. Проверка ошибки при отсутствии аргумента для -minl
void test_minl_without_arg(void) {
    Options opts;
    options_init(&opts);
    char *argv[] = {"prog", "-minl"};
    int ret = parse_options(2, argv, &opts);
    TEST_ASSERT_EQUAL_INT(-1, ret);
    options_free(&opts);
}

// 14. Проверка ошибки при нечисловом аргументе
void test_minl_with_non_numeric(void) {
    Options opts;
    options_init(&opts);
    char *argv[] = {"prog", "-minl", "abc"};
    int ret = parse_options(3, argv, &opts);
    TEST_ASSERT_EQUAL_INT(-1, ret);
    options_free(&opts);
}

// 15. Проверка, что -maxl требует -minl
void test_maxl_without_minl(void) {
    Options opts;
    options_init(&opts);
    char *argv[] = {"prog", "-maxl", "20"};
    int ret = parse_options(3, argv, &opts);
    TEST_ASSERT_EQUAL_INT(-1, ret);
    options_free(&opts);
}

// 16. -C с повторами (должно быть ошибкой)
void test_C_with_duplicates(void) {
    Options opts;
    options_init(&opts);
    char *argv[] = {"prog", "-C", "aDa"};
    int ret = parse_options(3, argv, &opts);
    TEST_ASSERT_EQUAL_INT(-1, ret);
    options_free(&opts);
}

// 17. -C с недопустимым символом
void test_C_invalid_char(void) {
    Options opts;
    options_init(&opts);
    char *argv[] = {"prog", "-C", "aX"};
    int ret = parse_options(3, argv, &opts);
    TEST_ASSERT_EQUAL_INT(-1, ret);
    options_free(&opts);
}

// 18. -d с несколькими символами
void test_d_with_multiple_chars(void) {
    Options opts;
    options_init(&opts);
    char *argv[] = {"prog", "-d", "ab"};
    int ret = parse_options(3, argv, &opts);
    TEST_ASSERT_EQUAL_INT(-1, ret);
    options_free(&opts);
}

// 19. -D с несколькими символами
void test_D_with_multiple_chars(void) {
    Options opts;
    options_init(&opts);
    char *argv[] = {"prog", "-D", "ab"};
    int ret = parse_options(3, argv, &opts);
    TEST_ASSERT_EQUAL_INT(-1, ret);
    options_free(&opts);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_parse_minl_maxl);
    RUN_TEST(test_parse_n);
    RUN_TEST(test_parse_n_incompatible_with_minmax);
    RUN_TEST(test_parse_alphabet_a_with_arg);
    RUN_TEST(test_parse_alphabet_a_without_arg);
    RUN_TEST(test_parse_categories_C);
    RUN_TEST(test_parse_a_and_C_incompatible);
    RUN_TEST(test_parse_probabilities);
    RUN_TEST(test_parse_too_many_probs);
    RUN_TEST(test_parse_delimiter_add_and_use);
    RUN_TEST(test_parse_delimiter_replace_and_use);
    RUN_TEST(test_ignore_unknown_options);
    RUN_TEST(test_minl_without_arg);
    RUN_TEST(test_minl_with_non_numeric);
    RUN_TEST(test_maxl_without_minl);
    RUN_TEST(test_C_with_duplicates);
    RUN_TEST(test_C_invalid_char);
    RUN_TEST(test_d_with_multiple_chars);
    RUN_TEST(test_D_with_multiple_chars);
    return UNITY_END();
}