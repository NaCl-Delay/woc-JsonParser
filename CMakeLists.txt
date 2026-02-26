cmake_minimum_required(VERSION 4.0)
project(JosnParserTry)

set(CMAKE_CXX_STANDARD 20)

add_executable(JosnParserTry src/main.cpp
        src/json.cpp
        src/parser.cpp)

target_include_directories(JosnParserTry PRIVATE include)
