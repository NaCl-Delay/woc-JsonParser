//
// Created by luolonghua on 2026/2/18.
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
Json& Json::operator[](const std::string& key) {
    if (!is_object()) {
        // 进阶技巧：如果原本是 null，访问 key 时自动变成 object
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

std::string Json::dump(int indent) const {
    switch (m_type) {
        case Type::Null:
            return "null";
        case Type::Bool:
            return std::get<bool>(m_value) ? "true" : "false";
        case Type::Number: {
            double val = std::get<double>(m_value);
            // 处理整数显示为 1 而不是 1.000000 的情况
            if (val == static_cast<long long>(val)) {
                return std::to_string(static_cast<long long>(val));
            }
            return std::to_string(val);
        }
        case Type::String:
            // 简单处理：加上双引号
            return "\"" + std::get<std::string>(m_value) + "\"";
        case Type::Array: {
            std::string res = "[";
            const auto& arr = std::get<array_type>(m_value);
            for (size_t i = 0; i < arr.size(); ++i) {
                res += arr[i].dump(indent); // 递归调用
                if (i < arr.size() - 1) res += ",";
            }
            res += "]";
            return res;
        }
        case Type::Object: {
            std::string res = "{";
            const auto& obj = std::get<object_type>(m_value);
            size_t i = 0;
            for (const auto& [key, value] : obj) {
                res += "\"" + key + "\":" + value.dump(indent); // 递归调用
                if (++i < obj.size()) res += ",";
            }
            res += "}";
            return res;
        }
        default:
            return "";
    }
}