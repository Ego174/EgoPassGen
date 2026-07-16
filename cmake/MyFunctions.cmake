include(FetchContent)

# Автоматически скачиваем Unity (фреймворк для тестирования)
if(NOT unity_POPULATED)
    FetchContent_Declare(
        unity
		URL https://github.com/ThrowTheSwitch/Unity/archive/refs/tags/v2.6.1.zip
		DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(unity)
endif()



function(get_all_sources dir result_var)
    file(GLOB SOURCE_FILES CONFIGURE_DEPENDS
        "${dir}/*.c"
        "${dir}/*.h"
    )
    set(${result_var} ${SOURCE_FILES} PARENT_SCOPE)
endfunction()



function(register_all_tests_in_dir base_tests_dir custom_target_name)
    file(GLOB SUB_DIRS RELATIVE "${base_tests_dir}" "${base_tests_dir}/*")
    
    foreach(sub_dir ${SUB_DIRS})
        set(full_sub_dir_path "${base_tests_dir}/${sub_dir}")
        
        if(IS_DIRECTORY "${full_sub_dir_path}")
            get_all_sources("${full_sub_dir_path}" TEST_SOURCES)
            
            if(TEST_SOURCES)
                set(test_exe_name "test_${sub_dir}_${custom_target_name}")
                
                add_executable(${test_exe_name} ${TEST_SOURCES})
                target_link_libraries(${test_exe_name} PRIVATE unity ${ARGN})
                
                # Регистрируем тест в CTest
                add_test(NAME ${test_exe_name} COMMAND ${test_exe_name})

                if(NOT TARGET ${custom_target_name})
                    add_custom_target(${custom_target_name}
                        COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIG> -R "_${custom_target_name}$" --output-on-failure
                        COMMENT "Running tests for ${custom_target_name}..."
                    )
                else()
                    add_custom_command(TARGET ${custom_target_name} POST_BUILD
                        COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIG> -R "^${test_exe_name}$" --output-on-failure
                    )
                endif()
                add_dependencies(${custom_target_name} ${test_exe_name})
                
                if(TARGET test_all)
                    add_dependencies(test_all ${custom_target_name})
                endif()
            endif()
        endif()
    endforeach()
endfunction()


