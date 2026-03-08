#include <iostream>
#include "json.h"

int main() {
#ifdef _WIN32
    system("chcp 65001");
#endif

    // 1. 解析一个完整的 JSON 字符串
    Json json = Json::parse(R"({
        "name": "张三",
        "age": 20,
        "is_student": true,
        "scores": [95, 87, 92],
        "address": {
            "city": "北京",
            "street": "中关村大街"
        },
        "remark": null
    })");

    // 2. 数据访问
    std::cout << "=== 数据访问 ===" << std::endl;
    std::cout << "姓名:   " << json["name"].as_string()          << std::endl;
    std::cout << "年龄:   " << json["age"].as_int()              << std::endl;
    std::cout << "在读:   " << (json["is_student"].as_bool() ? "是" : "否") << std::endl;
    std::cout << "第一门成绩: " << json["scores"][0].as_double() << std::endl;
    std::cout << "城市:   " << json["address"]["city"].as_string() << std::endl;
    std::cout << "备注是否为空: " << (json["remark"].is_null() ? "是" : "否") << std::endl;

    //3. 序列化
    std::cout << "\n=== 序列化（紧凑）===" << std::endl;
    std::cout << json.dump() << std::endl;

    std::cout << "\n=== 序列化（4 空格缩进）===" << std::endl;
    std::cout << json.dump(4) << std::endl;

    // 4. 错误处理示例
    std::cout << "=== 错误处理 ===" << std::endl;
    try {
        Json::parse("{bad json}");
    } catch (const std::exception& e) {
        std::cout << "捕获到错误: " << e.what() << std::endl;
    }

    return 0;
}