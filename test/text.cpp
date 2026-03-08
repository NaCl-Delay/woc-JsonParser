//
// Created by NaCl-Delay on 2026/3/8.
//

// tests/test_json.cpp
// JSON 解析器测试用例
// 编译方式（在项目根目录）：
//   g++ -std=c++17 -I include src/json.cpp src/parser.cpp tests/test_json.cpp -o test_json
// 运行：
//   ./test_json

#include <cassert>
#include <iostream>
#include <stdexcept>
#include "json.h"


// 辅助宏：捕获异常，确认异常确实被抛出

#define ASSERT_THROWS(expr)                          \
    do {                                             \
        bool _threw = false;                         \
        try { (expr); }                              \
        catch (...) { _threw = true; }               \
        assert(_threw && "Expected exception, but none was thrown"); \
    } while(0)

// 用于打印测试分组标题
static void section(const char* name) {
    std::cout << "\n[TEST] " << name << std::endl;
}
static void pass(const char* name) {
    std::cout << "  PASS  " << name << std::endl;
}

// 1. 基本类型解析测试

void test_basic_types() {
    section("Basic Types");

    // null
    {
        Json j = Json::parse("null");
        assert(j.is_null());
        pass("null");
    }

    // true / false
    {
        Json t = Json::parse("true");
        assert(t.is_bool());
        assert(t.as_bool() == true);

        Json f = Json::parse("false");
        assert(f.is_bool());
        assert(f.as_bool() == false);
        pass("boolean true/false");
    }

    // 整数
    {
        Json j = Json::parse("42");
        assert(j.is_number());
        assert(j.as_int() == 42);
        pass("integer");
    }

    // 负数
    {
        Json j = Json::parse("-7");
        assert(j.as_int() == -7);
        pass("negative integer");
    }

    // 浮点数
    {
        Json j = Json::parse("3.14");
        assert(j.is_number());
        assert(j.as_double() > 3.13 && j.as_double() < 3.15);
        pass("float");
    }

    // 科学计数法
    {
        Json j = Json::parse("1.5e2");
        assert(j.as_double() == 150.0);
        Json j2 = Json::parse("2E-3");
        assert(j2.as_double() > 0.001 && j2.as_double() < 0.003);
        pass("scientific notation");
    }

    // 简单字符串
    {
        Json j = Json::parse("\"hello\"");
        assert(j.is_string());
        assert(j.as_string() == "hello");
        pass("simple string");
    }

    // 空字符串
    {
        Json j = Json::parse("\"\"");
        assert(j.is_string());
        assert(j.as_string().empty());
        pass("empty string");
    }
}


// 2. 转义字符测试

void test_escape_characters() {
    section("Escape Characters");

    // 基本转义：\n \t \r \" \\

    {
        Json j = Json::parse(R"("line1\nline2")");
        assert(j.as_string().find('\n') != std::string::npos);
        pass("\\n newline");
    }
    {
        Json j = Json::parse(R"("col1\tcol2")");
        assert(j.as_string().find('\t') != std::string::npos);
        pass("\\t tab");
    }
    {
        Json j = Json::parse(R"("say \"hi\"")");
        assert(j.as_string() == "say \"hi\"");
        pass("\\\" quote");
    }
    {
        Json j = Json::parse(R"("C:\\Windows\\System32")");
        assert(j.as_string() == "C:\\Windows\\System32");
        pass("\\\\ backslash");
    }

    // \uXXXX：ASCII 范围
    {
        Json j = Json::parse(R"("\u0041\u0042\u0043")"); // ABC
        assert(j.as_string() == "ABC");
        pass("\\uXXXX ASCII");
    }

    // \uXXXX：中文 (U+4E2D = 中, U+6587 = 文)
    {
        Json j = Json::parse(R"("\u4e2d\u6587")");
        assert(j.as_string() == "中文");
        pass("\\uXXXX Chinese");
    }

    // \uXXXX：代理对 Emoji (U+1F600 = 😀)
    {
        Json j = Json::parse(R"("\uD83D\uDE00")");
        assert(j.as_string() == "😀");
        pass("\\uXXXX surrogate pair (emoji)");
    }

    // dump() 后转义字符应该被还原回转义序列
    {
        Json j = Json::parse(R"("a\nb")");
        std::string dumped = j.dump();
        assert(dumped.find("\\n") != std::string::npos);
        pass("dump() re-escapes \\n");
    }

    // Round-trip：parse → dump → parse，内容保持一致
    {
        std::string original = R"({"path":"C:\\foo\\bar","msg":"line1\nline2"})";
        Json j1 = Json::parse(original);
        std::string dumped = j1.dump();
        Json j2 = Json::parse(dumped);
        assert(j2["path"].as_string() == "C:\\foo\\bar");
        assert(j2["msg"].as_string().find('\n') != std::string::npos);
        pass("round-trip parse → dump → parse");
    }
}


// 3. 嵌套结构测试

void test_nested_structures() {
    section("Nested Structures");

    // 简单 object
    {
        Json j = Json::parse(R"({"name":"Alice","age":30})");
        assert(j.is_object());
        assert(j["name"].as_string() == "Alice");
        assert(j["age"].as_int() == 30);
        pass("simple object");
    }

    // 简单 array
    {
        Json j = Json::parse(R"([1, 2, 3])");
        assert(j.is_array());
        assert(j[0].as_int() == 1);
        assert(j[2].as_int() == 3);
        pass("simple array");
    }

    // 嵌套 object
    {
        Json j = Json::parse(R"({
            "address": {
                "city": "Beijing",
                "zip": "100000"
            }
        })");
        assert(j["address"]["city"].as_string() == "Beijing");
        pass("nested object");
    }

    // object 内含 array
    {
        Json j = Json::parse(R"({"scores": [95, 87, 92]})");
        assert(j["scores"][0].as_int() == 95);
        assert(j["scores"][1].as_int() == 87);
        assert(j["scores"][2].as_int() == 92);
        pass("object with array");
    }

    // array 内含 object
    {
        Json j = Json::parse(R"([{"x": 1}, {"x": 2}])");
        assert(j[0]["x"].as_int() == 1);
        assert(j[1]["x"].as_int() == 2);
        pass("array of objects");
    }

    // 深层嵌套（5 层）
    {
        Json j = Json::parse(R"({"a":{"b":{"c":{"d":{"e":42}}}}})");
        assert(j["a"]["b"]["c"]["d"]["e"].as_int() == 42);
        pass("deep nesting (5 levels)");
    }

    // 混合类型数组
    {
        Json j = Json::parse(R"([null, true, 1, "str", [], {}])");
        assert(j[0].is_null());
        assert(j[1].as_bool() == true);
        assert(j[2].as_int() == 1);
        assert(j[3].as_string() == "str");
        assert(j[4].is_array());
        assert(j[5].is_object());
        pass("mixed-type array");
    }
}


// 4. 边界情况测试

void test_edge_cases() {
    section("Edge Cases");

    // 空 object
    {
        Json j = Json::parse("{}");
        assert(j.is_object());
        assert(j.as_object().empty());
        pass("empty object {}");
    }

    // 空 array
    {
        Json j = Json::parse("[]");
        assert(j.is_array());
        assert(j.as_array().empty());
        pass("empty array []");
    }

    // 前后有空白字符
    {
        Json j = Json::parse("  \n  42  \n  ");
        assert(j.as_int() == 42);
        pass("leading/trailing whitespace");
    }

    // 数字 0
    {
        Json j = Json::parse("0");
        assert(j.as_int() == 0);
        pass("zero");
    }

    // 负浮点数
    {
        Json j = Json::parse("-0.5");
        assert(j.as_double() < 0);
        pass("negative float");
    }

    // 单个 null/true/false 在 object value 位置
    {
        Json j = Json::parse(R"({"a":null,"b":true,"c":false})");
        assert(j["a"].is_null());
        assert(j["b"].as_bool() == true);
        assert(j["c"].as_bool() == false);
        pass("null/bool as object values");
    }

    // dump() 空对象和空数组
    {
        Json j = Json::parse("{}");
        assert(j.dump() == "{}");
        Json j2 = Json::parse("[]");
        assert(j2.dump() == "[]");
        pass("dump() empty object/array");
    }

    // dump(4) 缩进格式不为空
    {
        Json j = Json::parse(R"({"x":1})");
        std::string s = j.dump(4);
        assert(s.find('\n') != std::string::npos);
        assert(s.find("    ") != std::string::npos); // 4 个空格
        pass("dump(4) pretty print");
    }
}


// 5. 错误输入处理测试

void test_error_handling() {
    section("Error Handling");

    // 完全空输入（EndInp 直接作为 value）
    ASSERT_THROWS(Json::parse(""));
    pass("empty input throws");

    // 未闭合字符串
    ASSERT_THROWS(Json::parse("\"unterminated"));
    pass("unterminated string throws");

    // 未闭合 object
    ASSERT_THROWS(Json::parse("{\"key\": 1"));
    pass("unclosed object throws");

    // 未闭合 array
    ASSERT_THROWS(Json::parse("[1, 2"));
    pass("unclosed array throws");

    // object 缺少冒号
    ASSERT_THROWS(Json::parse("{\"key\" 1}"));
    pass("missing colon throws");

    // object key 不是字符串
    ASSERT_THROWS(Json::parse("{1: \"val\"}"));
    pass("non-string key throws");

    // 非法字符
    ASSERT_THROWS(Json::parse("@"));
    pass("unexpected character throws");

    // 非法 \uXXXX（不足4位十六进制）
    ASSERT_THROWS(Json::parse("\"\\u00GG\""));
    pass("invalid \\uXXXX throws");

    // 顶层有多余内容
    ASSERT_THROWS(Json::parse("1 2"));
    pass("extra content after value throws");

    // 非法转义序列
    ASSERT_THROWS(Json::parse("\"\\q\""));
    pass("invalid escape sequence throws");

    // 访问越界数组
    {
        Json j = Json::parse("[1]");
        ASSERT_THROWS(j[5]);
        pass("array out-of-bounds throws");
    }

    // 对非 object 用 string key 访问
    {
        Json j = Json::parse("42");
        ASSERT_THROWS(j["key"]);
        pass("string key on non-object throws");
    }

    // 类型访问错误
    {
        Json j = Json::parse("\"hello\"");
        ASSERT_THROWS(j.as_int());  // string 不能 as_int
        pass("wrong type accessor throws");
    }
}


// 主函数

int main() {
#ifdef _WIN32
    system("chcp 65001");
#endif

    std::cout << "========================================" << std::endl;
    std::cout << "  JSON Parser Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;

    test_basic_types();
    test_escape_characters();
    test_nested_structures();
    test_edge_cases();
    test_error_handling();

    std::cout << "\n========================================" << std::endl;
    std::cout << "  All tests passed!" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}