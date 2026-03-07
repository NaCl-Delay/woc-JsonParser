//
// Created by NaCl-Delay on 2026/2/18.
//

#ifndef JOSNPARSERTRY_JOSN_H
#define JOSNPARSERTRY_JOSN_H

#include <string>
#include <vector>
#include <map>
#include <variant>
#include <string_view>

class Json {
public:
    // 1. 定义 JSON 的六种基本类型
    enum class Type {
        Null, Bool, Number, String, Array, Object
    };

    // 2. 为了代码整洁，给复杂的容器起别名
    using array_type = std::vector<Json>;
    using object_type = std::map<std::string, Json>;

    // 3. 构造函数：让 Json 对象在创建时就能自动识别类型
    Json() : m_value(std::nullptr_t{}), m_type(Type::Null) {}
    Json(std::nullptr_t) : m_value(std::nullptr_t{}), m_type(Type::Null) {}
    Json(bool value) : m_value(value), m_type(Type::Bool) {}
    Json(double value) : m_value(value), m_type(Type::Number) {}
    Json(int value) : m_value((double)value), m_type(Type::Number) {} // 整数也存为 double
    Json(const std::string& value) : m_value(value), m_type(Type::String) {}
    Json(const char* value) : m_value(std::string(value)), m_type(Type::String) {}
    Json(const array_type& value) : m_value(value), m_type(Type::Array) {}
    Json(const object_type& value) : m_value(value), m_type(Type::Object) {}

    // 4. 类型判断接口
    Type type() const { return m_type; }
    bool is_null() const { return m_type == Type::Null; }
    bool is_bool() const { return m_type == Type::Bool; }
    bool is_number() const { return m_type == Type::Number; }
    bool is_string() const { return m_type == Type::String; }
    bool is_array() const { return m_type == Type::Array; }
    bool is_object() const { return m_type == Type::Object; }

    // 5. 数据取值接口 (as_int, as_string 等)
    bool as_bool() const;
    double as_double() const;
    int as_int() const;
    const std::string& as_string() const;
    const array_type& as_array() const;
    const object_type& as_object() const;

    // 6. 方便的数据访问运算符重载
    // 访问数组：json[0]
    Json& operator[](size_t index);
    // 访问对象：json["key"]
    Json& operator[](const std::string& key);

    // 7. 静态解析函数：Json::parse("...")
    static Json parse(std::string_view source);

    // 8. 序列化函数：将 Json 对象转回字符串
    std::string dump(int indent = -1) const;// 这里的 indent 是用户要求的“每层几个空格”

private:
    // 核心存储：std::variant 会根据存入的值自动选择空间
    std::variant<std::nullptr_t, bool, double, std::string, array_type, object_type> m_value;
    Type m_type;

    // depth: 当前在第几层（0, 1, 2...）
    // indent_size: 每一层要缩进的固定空格数（比如 4）
    void dump_internal(std::ostringstream& oss, int depth, int indent_size) const;
};

#endif //JOSNPARSERTRY_JOSN_H