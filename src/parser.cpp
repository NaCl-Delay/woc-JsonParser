//
// Created by luolonghua on 2026/2/18.
//

#include "parser.h"

// token_line/token_col: 字符串内容（不含开头的 "）在源码中的起始位置
static std::string unescape_string(std::string_view raw, int token_line, int token_col) {
    std::string result;
    result.reserve(raw.size());

    int cur_line = token_line;
    int cur_col  = token_col;

    for (size_t i = 0; i < raw.size(); ++i) {
        if (raw[i] == '\\' && i + 1 < raw.size()) {
            int escape_line = cur_line;
            int escape_col  = cur_col;
            cur_col++;
            i++;
            cur_col++;

            switch (raw[i]) {
                case '"':  result += '"';  break;
                case '\\': result += '\\'; break;
                case '/':  result += '/';  break;
                case 'b':  result += '\b'; break;
                case 'f':  result += '\f'; break;
                case 'n':  result += '\n'; break;
                case 'r':  result += '\r'; break;
                case 't':  result += '\t'; break;
                case 'u': {
                    auto parse_hex4 = [&](size_t pos) -> int {
                        if (pos + 4 > raw.size()) return -1;
                        int val = 0;
                        for (int j = 0; j < 4; ++j) {
                            char c = raw[pos + j];
                            val <<= 4;
                            if (c >= '0' && c <= '9') val |= (c - '0');
                            else if (c >= 'a' && c <= 'f') val |= (c - 'a' + 10);
                            else if (c >= 'A' && c <= 'F') val |= (c - 'A' + 10);
                            else return -1;
                        }
                        return val;
                    };

                    int cp = parse_hex4(i + 1);
                    if (cp == -1) {
                        throw std::runtime_error(
                            "Line " + std::to_string(escape_line) +
                            ", Col " + std::to_string(escape_col) +
                            ": Invalid \\uXXXX sequence");
                    }
                    i += 4; cur_col += 4;
                    if (cp >= 0xD800 && cp <= 0xDBFF) {
                        if (i + 2 < raw.size() && raw[i + 1] == '\\' && raw[i + 2] == 'u') {
                            int cp2 = parse_hex4(i + 3);
                            if (cp2 >= 0xDC00 && cp2 <= 0xDFFF) {
                                cp = (((cp - 0xD800) << 10) | (cp2 - 0xDC00)) + 0x10000;
                                i += 6; cur_col += 6;
                            }
                        }
                    }
                    if (cp <= 0x7F) {
                        result += static_cast<char>(cp);
                    } else if (cp <= 0x7FF) {
                        result += static_cast<char>(0xC0 | (cp >> 6));
                        result += static_cast<char>(0x80 | (cp & 0x3F));
                    } else if (cp <= 0xFFFF) {
                        result += static_cast<char>(0xE0 | (cp >> 12));
                        result += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                        result += static_cast<char>(0x80 | (cp & 0x3F));
                    } else {
                        result += static_cast<char>(0xF0 | (cp >> 18));
                        result += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
                        result += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                        result += static_cast<char>(0x80 | (cp & 0x3F));
                    }
                    break;
                }
                default:
                    throw std::runtime_error(
                        "Line " + std::to_string(escape_line) +
                        ", Col " + std::to_string(escape_col) +
                        ": Invalid escape sequence: \\" + raw[i]);
            }
        } else {
            if (raw[i] == '\n') { cur_line++; cur_col = 1; }
            else                { cur_col++; }
            result += raw[i];
        }
    }
    return result;
}

char Scanner::advance() {
    char current = src[cursor];
    if (current == '\n' || current == '\r') { line++; column = 1; }
    else                                    { column++; }
    cursor++;
    return current;
}

Token Scanner::next_token() {
    skip_whitespace();
    if (cursor >= src.size()) return {TokenType::EndInp, "", line, column};

    char c = src[cursor];
    switch (c) {
        case '{': { int l=line,col=column; advance(); return {TokenType::BeginObject, "{", l, col}; }
        case '}': { int l=line,col=column; advance(); return {TokenType::EndObject,   "}", l, col}; }
        case '[': { int l=line,col=column; advance(); return {TokenType::BeginArray,  "[", l, col}; }
        case ']': { int l=line,col=column; advance(); return {TokenType::EndArray,    "]", l, col}; }
        case ':': { int l=line,col=column; advance(); return {TokenType::Colon,       ":", l, col}; }
        case ',': { int l=line,col=column; advance(); return {TokenType::Comma,       ",", l, col}; }
    }

    if (c == '"') {
        int l = line, col = column;
        advance(); // 消耗开头 "
        int content_col = column; // 内容起始列（开头 " 之后）

        size_t start = cursor;
        while (cursor < src.size()) {
            if (src[cursor] == '\\') {
                advance();
                if (cursor < src.size()) advance();
            } else if (src[cursor] == '"') {
                break;
            } else {
                advance();
            }
        }
        if (cursor >= src.size()) {
            throw std::runtime_error(
                "Line " + std::to_string(l) +
                ", Col " + std::to_string(col) +
                ": Unterminated string");
        }
        std::string_view val = src.substr(start, cursor - start);
        advance(); // 消耗结束 "
        // line 用 l（字符串可能跨行，token 行列记录开头），col 用 content_col
        return {TokenType::String, val, l, content_col};
    }

    if (std::isdigit(c) || c == '-' || c == '+') {
        int l=line, col=column;
        size_t start = cursor;
        advance();
        while (cursor < src.size() && (std::isdigit(src[cursor]) || src[cursor] == '.'))
            advance();
        return {TokenType::Number, src.substr(start, cursor - start), l, col};
    }

    if (src.substr(cursor, 4) == "true")  { int l=line,col=column; for(int i=0;i<4;++i) advance(); return {TokenType::True,  "true",  l, col}; }
    if (src.substr(cursor, 5) == "false") { int l=line,col=column; for(int i=0;i<5;++i) advance(); return {TokenType::False, "false", l, col}; }
    if (src.substr(cursor, 4) == "null")  { int l=line,col=column; for(int i=0;i<4;++i) advance(); return {TokenType::Null,  "null",  l, col}; }

    int l=line, col=column;
    advance();
    throw std::runtime_error("Line " + std::to_string(l) + ", Col " + std::to_string(col) + ": Unexpected character");
}

void Scanner::skip_whitespace() {
    while (cursor < src.size() && (src[cursor]==' '||src[cursor]=='\n'||src[cursor]=='\r'||src[cursor]=='\t'))
        advance();
}

Json Parser::parse()        { return parse_value(); }
void Parser::consume()      { lookahead = scanner.next_token(); }

Json Parser::parse_array() {
    Json::array_type arr;
    consume();
    while (lookahead.type != TokenType::EndArray) {
        arr.push_back(parse_value());
        if (lookahead.type == TokenType::Comma) consume();
    }
    consume();
    return Json(arr);
}

Json Parser::parse_string() {
    std::string val = unescape_string(lookahead.value, lookahead.line, lookahead.column);
    consume();
    return Json(val);
}

Json Parser::parse_number() {
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
        default: throw std::runtime_error(
            "Line " + std::to_string(lookahead.line) +
            ", Col " + std::to_string(lookahead.column) +
            ": Unexpected token");
    }
}

Json Parser::parse_object() {
    Json::object_type obj;
    consume();

    while (lookahead.type != TokenType::EndObject) {
        if (lookahead.type != TokenType::String) {
            throw std::runtime_error(
                "Line " + std::to_string(lookahead.line) +
                ", Col " + std::to_string(lookahead.column) +
                ": Key must be a string");
        }
        std::string key = unescape_string(lookahead.value, lookahead.line, lookahead.column);
        consume();

        if (lookahead.type != TokenType::Colon) {
            throw std::runtime_error(
                "Line " + std::to_string(lookahead.line) +
                ", Col " + std::to_string(lookahead.column) +
                ": Expected ':' after key");
        }
        consume();

        obj[key] = parse_value();

        if (lookahead.type == TokenType::Comma) consume();
    }
    consume();
    return Json(obj);
}