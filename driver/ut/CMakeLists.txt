find_package (Threads)

add_executable(atsd-odbc-ut
    main.cpp
    escape_sequences_ut.cpp
    lexer_ut.cpp
)

target_link_libraries(atsd-odbc-ut
    PRIVATE
    atsd-odbc-escaping
    gtest-lib
    Threads::Threads
)

add_test(NAME "atsd-odbc-ut" COMMAND "atsd-odbc-ut")

if (NANOODBC_LIBRARY)
    add_executable(atsd-odbc-nano nano.cpp)
    target_link_libraries(atsd-odbc-nano PRIVATE ${NANOODBC_LIBRARY})
    add_dependencies(atsd-odbc-nano atsd-odbc)
    if (TEST_DSN)
        add_test(NAME "atsd-odbc-nano" COMMAND atsd-odbc-nano DSN=${TEST_DSN})
        if (NOT (CMAKE_BUILD_TYPE_UC STREQUAL "TSAN" OR CMAKE_BUILD_TYPE_UC STREQUAL "ASAN" OR CMAKE_BUILD_TYPE_UC STREQUAL "MSAN"))
            #TODO: add_test(NAME "atsd-odbc-nano-valgrind" COMMAND valgrind -v --leak-check=full --show-leak-kinds=all ${CMAKE_CURRENT_BINARY_DIR}/atsd-odbc-nano DSN=${TEST_DSN})
        endif ()
    endif ()
    if (TEST_DSN_W)
        add_test(NAME "atsd-odbc-nano-w" COMMAND atsd-odbc-nano DSN=${TEST_DSN_W})
        if (NOT (CMAKE_BUILD_TYPE_UC STREQUAL "TSAN" OR CMAKE_BUILD_TYPE_UC STREQUAL "ASAN" OR CMAKE_BUILD_TYPE_UC STREQUAL "MSAN"))
            #TODO: add_test(NAME "atsd-odbc-nano-valgrind-w" COMMAND valgrind -v --leak-check=full --show-leak-kinds=all ${CMAKE_CURRENT_BINARY_DIR}/atsd-odbc-nano DSN=${TEST_DSN_W})
        endif ()
    endif ()
    # gdb -ex "catch throw" -ex run --args ./atsd-odbc-nano DSN=atsd_localhost_system
endif ()

add_executable(load-ut load.cpp)
target_link_libraries(load-ut PRIVATE ${Poco_Foundation_LIBRARY} ${CMAKE_DL_LIBS} Threads::Threads)
if (WIN32)
   target_link_libraries(load-ut PRIVATE Iphlpapi)
endif()
if(ODBC_INCLUDE_DIRECTORIES)
    target_include_directories(load-ut PRIVATE ${ODBC_INCLUDE_DIRECTORIES})
endif()
add_test(NAME "load-ut" COMMAND load-ut $<TARGET_FILE:atsd-odbc>)
add_test(NAME "load-ut-w" COMMAND load-ut $<TARGET_FILE:atsd-odbcw>)
