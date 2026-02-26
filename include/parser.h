//
// Created by luolonghua on 2026/2/18.
//

#ifndef JOSNPARSERTRY_PARSER_H
#define JOSNPARSERTRY_PARSER_H
#include <stdexcept>
#include <string_view>

#include "json.h"

enum class TokenType {
    BeginObject,  // {
    EndObject,    // }
    BeginArray,   // [
    EndArray,     // ]
    String,       // "name"
    Number,       // 123.45
    True,         // true
    False,        // false
    Null,         // null
    Colon,        // :
    Comma,        // ,
    EndInp        // 输入结束
};

struct Token {
    TokenType type;
    std::string_view value; // 直接指向原始字符串的一段，不拷贝，速度快
};

class Scanner {
    std::string_view src;
    size_t cursor = 0;

public:
    Scanner(std::string_view s) : src(s) {}
    Token next_token();

private:
    void skip_whitespace();
};

class Parser {
    Scanner scanner;
    Token lookahead; // 提前看一眼当前的单词

public:
    Parser(std::string_view s) : scanner(s) {
        lookahead = scanner.next_token(); // 初始化，先拿第一个零件
    }

    // 核心入口
    Json parse();

private:
    void consume();

    Json parse_array();

    Json parse_string();

    Json parse_number();

    // 解析任何可能的 JSON 值
    Json parse_value();
    // 解析对象 { "key" : value }
    Json parse_object();

};
#endif //JOSNPARSERTRY_PARSER_H