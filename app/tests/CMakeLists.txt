# Создаем тест, привязываем к цели test_app, линкуем my_library
register_all_tests_in_dir("${CMAKE_CURRENT_SOURCE_DIR}" test_app my_library)
