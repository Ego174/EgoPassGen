cmake_minimum_required(VERSION 3.15)

project(MyProject C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)


if(MSVC)
    add_compile_options(/wd5045)
    
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
	link_libraries(msvcrt.lib)
	link_libraries(legacy_stdio_definitions.lib)
	
endif()


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(MyFunctions)

option(BUILD_SHARED_LIBS "Build libraries as shared (dynamic)" OFF)
option(BUILD_TESTING "Build the testing suite" ON)

if(BUILD_TESTING)
    enable_testing()
endif()


add_subdirectory(lib)

add_subdirectory(app)


if(BUILD_TESTING)
    add_custom_target(test_all
        COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIG> --output-on-failure
        COMMENT "Running ALL tests..."
    )
endif()