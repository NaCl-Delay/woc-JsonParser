//
// Created by NaCl-Delay on 2026/2/18.
//

#include "json.h"
#include "parser.h"
#include <sstream>
#include <iomanip>
#include <stdexcept>

// 取布尔值
bool Json::as_bool() const {
    if (!is_bool()) throw std::runtime_error("Not a boolean");
    return std::get<bool>(m_value);
}

// 取浮点数
double Json::as_double() const {
    if (!is_number()) throw std::runtime_error("Not a number");
    return std::get<double>(m_value);
}

// 取整数（简单转换）
int Json::as_int() const {
    return static_cast<int>(as_double());
}

// 取字符串
const std::string& Json::as_string() const {
    if (!is_string()) throw std::runtime_error("Not a string");
    return std::get<std::string>(m_value);
}

// 数组索引
Json& Json::operator[](size_t index) {
    if (!is_array()) throw std::runtime_error("Not an array");
    return std::get<array_type>(m_value).at(index);
}

//取数组
const Json::array_type& Json::as_array() const {
    if (!is_array()) throw std::runtime_error("Not an array");
    return std::get<array_type>(m_value);
}

// 对象键值访问
// 访问 null 时自动变成 object
Json& Json::operator[](const std::string& key) {
    if (!is_object()) {
        if (is_null()) {
            m_type = Type::Object;
            m_value = object_type();
        } else {
            throw std::runtime_error("Not an object");
        }
    }
    return std::get<object_type>(m_value)[key];
}

//取对象
const Json::object_type& Json::as_object() const{
    if (!is_object()) {
        throw std::runtime_error("Not an object");
    }
    return std::get<object_type>(m_value);
}

Json Json::parse(std::string_view source) {
    Parser p(source);
    return p.parse();
}

// 只在 json.cpp 内部使用，用来生成空格
static std::string get_indent_str(int depth, int indent_size) {
    if (indent_size <= 0) return "";
    return std::string(depth * indent_size, ' ');
}


// 将字符串写入 oss，自动处理转义（String 值和 Object key 共用）
static void write_escaped_string(std::ostringstream& oss, const std::string& s) {
    oss << "\"";
    for (char c : s) {
        switch (c) {
            case '\"': oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\b': oss << "\\b";  break;
            case '\f': oss << "\\f";  break;
            case '\n': oss << "\\n";  break;
            case '\r': oss << "\\r";  break;
            case '\t': oss << "\\t";  break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[8];
                    snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned char>(c));
                    oss << buf;
                } else {
                    oss << c;
                }
                break;
        }
    }
    oss << "\"";
}

void Json::dump_internal(std::ostringstream& oss, int depth, int indent_size) const {
    bool pretty = indent_size >= 0;

    switch (m_type) {
        case Type::Null:   oss << "null";
            break;

        case Type::Bool:   oss << (std::get<bool>(m_value) ? "true" : "false");
            break;

        case Type::Number: {
            double val = std::get<double>(m_value);
            // 解决 1.000000 问题：如果是整数，去掉小数点
            if (val == static_cast<long long>(val))
                oss << static_cast<long long>(val);
            else
                oss << val;
            break;
        }

        case Type::String: {
            write_escaped_string(oss, std::get<std::string>(m_value));
            break;
        }

        case Type::Array: {
            const auto& arr = std::get<array_type>(m_value);
            if (arr.empty()) { oss << "[]"; break; }

            oss << "[";
            if (pretty) oss << "\n";
            for (size_t i = 0; i < arr.size(); ++i) {
                if (pretty) oss << get_indent_str(depth + 1, indent_size);
                arr[i].dump_internal(oss, depth + 1, indent_size); // 递归：深度+1
                if (i < arr.size() - 1) oss << ",";
                if (pretty) oss << "\n";
            }
            if (pretty) oss << get_indent_str(depth, indent_size); // 封口缩进回退
            oss << "]";
            break;
        }
        case Type::Object: {
            const auto& obj = std::get<object_type>(m_value);
            if (obj.empty()) { oss << "{}"; break; }

            oss << "{";
            if (pretty) oss << "\n";
            size_t count = 0;
            for (const auto& [key, value] : obj) {
                if (pretty) oss << get_indent_str(depth + 1, indent_size);
                write_escaped_string(oss, key);
                oss << ": ";
                value.dump_internal(oss, depth + 1, indent_size);
                if (++count < obj.size()) oss << ",";
                if (pretty) oss << "\n";
            }
            if (pretty) oss << get_indent_str(depth, indent_size);
            oss << "}";
            break;
        }
    }
}

// 用户调用的公开接口
std::string Json::dump(int indent) const {
    std::ostringstream oss;
    dump_internal(oss, 0, indent); // 从深度 0 开始递归
    return oss.str();
}