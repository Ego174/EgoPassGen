/*
password_gen.c - Реализация генерации паролей и публичной функции

Хаиров Егор Вадимович
МК-101
*/

#include "password_gen.h"
#include "options.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

// Нормализует вероятности так, чтобы сумма была равна 1
static void normalize_probs(double *probs, int count) {
    double sum = 0.0;
    for(int i = 0; i < count; ++i) sum += probs[i];
    if(sum == 0.0) {
        for(int i = 0; i < count; ++i) probs[i] = 1.0 / count;
    } else {
        for(int i = 0; i < count; ++i) probs[i] /= sum;
    }
}

// Дополнительная функция: распределяет оставшуюся вероятность между элементами со значением -1.0
// и нормализует все вероятности
static void finalize_probabilities(double *probs, int count) {
    double sum_given = 0.0;
    int unassigned_count = 0;
    for(int i = 0; i < count; ++i) {
        if(probs[i] >= 0.0) {
            sum_given += probs[i];
        } else {
            unassigned_count++;
        }
    }
    if(sum_given > 1.0) {
        // Если сумма заданных > 1, просто нормализуем все (включая -1.0)
        // Нормализация заменит -1.0 на 0, но мы их обработаем позже
        // Проще: все элементы с -1.0 обнулим, а затем нормализуем все
        for(int i = 0; i < count; ++i) {
            if(probs[i] < 0.0) probs[i] = 0.0;
        }
        normalize_probs(probs, count);
        return;
    }
    // Распределяем оставшуюся вероятность равномерно между незаданными
    double remaining = 1.0 - sum_given;
    if(unassigned_count > 0 && remaining >= 0) {
        double each = remaining / unassigned_count;
        for(int i = 0; i < count; ++i) {
            if(probs[i] < 0.0) probs[i] = each;
        }
    } else {
        // Если незаданных нет или remaining < 0, нормализуем все
        for(int i = 0; i < count; ++i) {
            if(probs[i] < 0.0) probs[i] = 0.0;
        }
        normalize_probs(probs, count);
    }
}

// Генерирует и выводит пароли согласно опциям
static void generate_passwords(const Options *opts) {
    int alphabet_size = 0;
    char *alphabet = NULL;
    double *probs = NULL;
    bool free_alphabet = false;
    bool free_probs = false;

    // Построение алфавита и вероятностей в зависимости от типа
    if(opts->alphabet_type == ALPHABET_TYPE_STRING) {
        alphabet = opts->alphabet_str;
        alphabet_size = strlen(alphabet);
        probs = malloc(alphabet_size * sizeof(double));
        if(!probs) {
            fprintf(stderr, "Memory error in generation\n");
            return;
        }
        free_probs = true;

        // Если заданы вероятности, копируем их; иначе все равны
        if(opts->has_probs) {
            // opts->probs уже должен быть массивом длины alphabet_size
            for(int i = 0; i < alphabet_size; ++i) {
                probs[i] = opts->probs[i];
            }
            // Распределяем остаток и нормализуем
            finalize_probabilities(probs, alphabet_size);
        } else {
            for(int i = 0; i < alphabet_size; ++i) probs[i] = 1.0 / alphabet_size;
        }
    } else { // ALPHABET_TYPE_CATEGORIES
        // Строим полный алфавит из категорий
        char full_alphabet[256];
        int full_size = 0;
        for(const char *cat = opts->categories; *cat; ++cat) {
            switch(*cat) {
                case 'a':
                    for(char c = 'a'; c <= 'z'; ++c) full_alphabet[full_size++] = c;
                    break;
                case 'A':
                    for(char c = 'A'; c <= 'Z'; ++c) full_alphabet[full_size++] = c;
                    break;
                case 'D':
                    for(char c = '0'; c <= '9'; ++c) full_alphabet[full_size++] = c;
                    break;
                case 'S':
                    {
                        const char *specials = "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
                        for(const char *s = specials; *s; ++s) full_alphabet[full_size++] = *s;
                    }
                    break;
            }
        }
        full_alphabet[full_size] = '\0';
        alphabet = strdup(full_alphabet);
        if(!alphabet) {
            fprintf(stderr, "Memory error in generation\n");
            return;
        }
        free_alphabet = true;
        alphabet_size = full_size;

        // Получаем вероятности категорий
        int cat_count = strlen(opts->categories);
        double cat_probs[4];
        if(opts->has_probs) {
            // opts->probs уже должен быть массивом длины cat_count
            for(int i = 0; i < cat_count; ++i) {
                cat_probs[i] = opts->probs[i];
            }
            finalize_probabilities(cat_probs, cat_count);
        } else {
            for(int i = 0; i < cat_count; ++i) cat_probs[i] = 1.0 / cat_count;
        }

        // Назначаем каждому символу вероятность категории / размер категории
        probs = malloc(alphabet_size * sizeof(double));
        if(!probs) {
            fprintf(stderr, "Memory error in generation\n");
            if(free_alphabet) free(alphabet);
            return;
        }
        free_probs = true;
        int idx = 0;
        int cat_index = 0;
        for(const char *cat = opts->categories; *cat; ++cat) {
            int cat_size = 0;
            switch(*cat) {
                case 'a': cat_size = 26; break;
                case 'A': cat_size = 26; break;
                case 'D': cat_size = 10; break;
                case 'S': cat_size = 32; break;
            }
            double prob_per_char = cat_probs[cat_index] / cat_size;
            for(int i = 0; i < cat_size; ++i) {
                probs[idx++] = prob_per_char;
            }
            cat_index++;
        }
        normalize_probs(probs, alphabet_size);
    }

    // Определяем длину пароля
    int length;
    if(opts->has_fixed_len) {
        length = opts->fixed_len;
    } else {
        int min = opts->has_minl ? opts->min_len : 1;
        int max = opts->has_maxl ? opts->max_len : 8;
        if(!opts->has_minl && !opts->has_maxl) {
            length = 8;
        } else {
            length = min + rand() % (max - min + 1);
        }
    }

    // Генерируем каждый пароль
    for(int p = 0; p < opts->count; ++p) {
        char *password = malloc(length + 1);
        if(!password) {
            fprintf(stderr, "Memory error\n");
            break;
        }
        for(int i = 0; i < length; ++i) {
            double r = (double)rand() / RAND_MAX;
            double cum = 0.0;
            int chosen = 0;
            for(int j = 0; j < alphabet_size; ++j) {
                cum += probs[j];
                if(r <= cum) {
                    chosen = j;
                    break;
                }
            }
            password[i] = alphabet[chosen];
        }
        password[length] = '\0';
        printf("%s\n", password);
        free(password);
    }

    // Освобождаем временные ресурсы
    if(free_probs) free(probs);
    if(free_alphabet) free(alphabet);
}

// Публичная функция
int generate_passwords_from_args(int argc, char **argv) {
    Options opts;
    if(options_init(&opts) != 0) {
        fprintf(stderr, "Error: Initialization failed (memory)\n");
        return -1;
    }

    if(parse_options(argc, argv, &opts) != 0) {
        options_free(&opts);
        return -1;
    }

    srand((unsigned int)time(NULL));
    generate_passwords(&opts);

    options_free(&opts);
    return 0;
}