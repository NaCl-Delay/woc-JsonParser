#include <iostream>
#include <string>
#include "json.h"

int main() {
#ifdef _WIN32
    system("chcp 65001"); // 将控制台设置为 UTF-8 编码
#endif


    // 1. 准备一个复杂的 JSON 字符串
    std::string raw_json = R"(
    {
        "name": "Gemini",
        "version": 3.1,
        "is_active": true,
        "features": ["parsing", "serialization", "recursive"],
        "metadata": {
            "author": "luolonghua",
            "year": 2026
        },
        "nil_val": null
    }
    )";

    try {
        // 2. 调用解析接口
        Json data = Json::parse(raw_json);

        // 3. 验证数据访问
        std::cout << "--- 基础字段访问 ---" << std::endl;
        std::cout << "Name: " << data["name"].as_string() << std::endl;
        std::cout << "Version: " << data["version"].as_double() << std::endl;
        std::cout << "Is Active: " << (data["is_active"].as_bool() ? "Yes" : "No") << std::endl;

        std::cout << "\n--- 数组访问 ---" << std::endl;
        // 使用 operator[] 访问数组元素
        std::cout << "First Feature: " << data["features"][0].as_string() << std::endl;

        std::cout << "\n--- 嵌套对象访问 ---" << std::endl;
        // 链式访问 data["key"]["subkey"]
        std::cout << "Author: " << data["metadata"]["author"].as_string() << std::endl;

        std::cout << "\n--- 序列化测试 (dump) ---" << std::endl;
        // 测试我们刚刚写好的 dump 函数
        std::cout << data.dump() << std::endl;

    } catch (const std::exception& e) {
        // 捕获 Scanner 或 Parser 抛出的异常
        std::cerr << "解析出错: " << e.what() << std::endl;
    }

    return 0;
}