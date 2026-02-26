//
// Created by luolonghua on 2026/2/18.
//

#include "parser.h"

Token Scanner::next_token() {
    skip_whitespace(); // 自动跳过空格、换行
    if (cursor >= src.size()) return {TokenType::EndInp, ""};

    char c = src[cursor];
    switch (c) {
        case '{': cursor++; return {TokenType::BeginObject, "{"};
        case '}': cursor++; return {TokenType::EndObject, "}"};
        case '[': cursor++; return {TokenType::BeginArray, "["};
        case ']': cursor++; return {TokenType::EndArray, "]"};
        case ':': cursor++; return {TokenType::Colon, ":"};
        case ',': cursor++; return {TokenType::Comma, ","};
    }

    //字符串
    if (c == '"') {
        size_t start = ++cursor; //为什么是++cursor,即为什么要跳过 " ?.应该看parser_string
        while (cursor < src.size() && src[cursor] != '"') {
            cursor++; //cursor到了 " 就退出循环
        }//eg. "hello" 0 1 2 3 4 5 6       start:1,   6 - 1 = 5.符合长度
        std::string_view val = src.substr(start, cursor - start);
        cursor++; //跳过结束的 "
        return {TokenType::String, val};
    }

    //数字
    if (std::isdigit(c) || c == '-' || c == '+') {
        size_t start = cursor++; // 如果只有单独的-或+符号，会有bug
        while (cursor < src.size() && (std::isdigit(src[cursor]) || src[cursor] == '.')) {
            cursor++;
        }
        std::string_view val = src.substr(start, cursor - start);
        return {TokenType::Number, val};
    }

    //bool
    if (src.substr(cursor, 4) == "true") {
        cursor += 4;
        return {TokenType::True, "true"};
    }

    if (src.substr(cursor, 5) == "false") {
        cursor += 5;
        return {TokenType::False, "false"};
    }

    //Null
    if (src.substr(cursor, 4) == "null") {
        cursor += 4;
        return {TokenType::Null, "null"};
    }

    //其他 错误
    throw std::runtime_error("Unexpected character");

}


void Scanner::skip_whitespace() {
    while (cursor < src.size() && (src[cursor] == ' ' || src[cursor] == '\n' || src[cursor] == '\r' || src[cursor] == '\t')) {
        cursor++;
    }
}

// 核心入口
Json Parser::parse() {
    return parse_value();
}

void Parser::consume() {
    lookahead = scanner.next_token();
}

Json Parser::parse_array() {
    Json::array_type arr;
    consume(); // 吃掉 '['
    while (lookahead.type != TokenType::EndArray) {
        arr.push_back(parse_value());
        if (lookahead.type == TokenType::Comma) {
            consume(); // 跳过 ','
        }
    }
    consume(); // 吃掉 ']'
    return Json(arr);
}

Json Parser::parse_string() {
    std::string val(lookahead.value);
    consume();// 消费掉这个 String Token
    return Json(val);
}

Json Parser::parse_number() {
    // 将 string_view 转为 string 再用 stod(string to double)  转为 double .
    double val = std::stod(std::string(lookahead.value));
    consume();
    return Json(val);
}

Json Parser::parse_value() {
    switch (lookahead.type) {
        case TokenType::BeginObject: return parse_object();
        case TokenType::BeginArray:  return parse_array();
        case TokenType::String:      return parse_string();
        case TokenType::Number:      return parse_number();
        case TokenType::True:        consume(); return Json(true);
        case TokenType::False:       consume(); return Json(false);
        case TokenType::Null:        consume(); return Json(nullptr);
        default: throw std::runtime_error("Unexpected token");
    }
}

// 解析对象 { "key" : value }
Json Parser::parse_object() {
    Json::object_type obj;
    consume(); // 吃掉 '{'

    while (lookahead.type != TokenType::EndObject) {
        // 1. 解析 Key
        if (lookahead.type != TokenType::String) {
            throw std::runtime_error("Key must be a string");
        }
        std::string key = std::string(lookahead.value);
        consume(); // 吃掉 Key

        // 2. 吃掉 ':'
        if (lookahead.type != TokenType::Colon) {
            throw std::runtime_error("Expected ':' after key");
        }
        consume();

        // 3. 递归解析 Value (这里就是最神奇的地方！)
        obj[key] = parse_value();

        // 4. 处理逗号
        if (lookahead.type == TokenType::Comma) {
            consume();
        }
    }
    consume(); // 吃掉 '}'
    return Json(obj);
}