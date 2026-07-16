get_all_sources("${CMAKE_CURRENT_SOURCE_DIR}/src" LIB_SOURCES)
add_library(my_library ${LIB_SOURCES})

target_include_directories(my_library PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

if(BUILD_SHARED_LIBS)
    set_target_properties(my_library PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# Тесты библиотеки
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

