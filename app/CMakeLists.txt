get_all_sources("${CMAKE_CURRENT_SOURCE_DIR}/src" APP_SOURCES)
add_executable(ego_pass_gen ${APP_SOURCES})

target_link_libraries(ego_pass_gen PRIVATE my_library)

# Тесты программы
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
