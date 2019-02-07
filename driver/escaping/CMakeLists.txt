add_library(atsd-odbc-escaping
    escape_sequences.cpp
    lexer.cpp
)

target_include_directories(atsd-odbc-escaping PUBLIC ..)
target_link_libraries(atsd-odbc-escaping PRIVATE atsd-odbc-log)
