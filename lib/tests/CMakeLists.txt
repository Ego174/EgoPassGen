# Создаём тест, привязываем к цели test_parser, линкуем my_library
register_all_tests_in_dir("${CMAKE_CURRENT_SOURCE_DIR}" test_parser my_library)

target_include_directories(test_test_parser_test_parser PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)