# Создаем тест, привязываем к цели test_lib, линкуем my_library
#register_all_tests_in_dir(check_library "${CMAKE_CURRENT_SOURCE_DIR}" test_lib my_library)
register_all_tests_in_dir("${CMAKE_CURRENT_SOURCE_DIR}" test_lib my_library)
