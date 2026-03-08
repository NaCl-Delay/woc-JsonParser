cmake_minimum_required(VERSION 4.0)
project(JsonParserTry)

set(CMAKE_CXX_STANDARD 20)

add_executable(JosnParserTry src/main.cpp
        src/json.cpp
        src/parser.cpp
)

target_include_directories(JosnParserTry PRIVATE include)

# 单独的测试 target
add_executable(JsonTest
        test/text.cpp
        src/json.cpp
        src/parser.cpp
)
target_include_directories(JsonTest PRIVATE include)