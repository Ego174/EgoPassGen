get_all_sources("${CMAKE_CURRENT_SOURCE_DIR}/src" APP_SOURCES)
add_executable(my_program ${APP_SOURCES})

target_link_libraries(my_program PRIVATE my_library)

# Тесты программы
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
