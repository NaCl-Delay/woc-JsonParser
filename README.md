# JSON 解析器

一个用 C++17 实现的轻量级 JSON 解析器，支持标准 JSON 的解析、数据访问和序列化。

---

## 功能列表


### 基础功能
- 支持 JSON 六种基本类型：`null`、`boolean`、`number`、`string`、`array`、`object`
- 完整的转义字符处理：`\n`、`\t`、`\"`、`\\`、`\uXXXX`（含 Unicode 代理对）
- 解析失败时提供包含**行号/列号**的错误信息
- 支持将解析结果序列化回 JSON 字符串（紧凑格式 / 缩进格式）

### 数据访问 API
| 方法 | 说明 |
|------|------|
| `as_string()` | 取字符串值 |
| `as_int()` | 取整数值 |
| `as_double()` | 取浮点数值 |
| `as_bool()` | 取布尔值 |
| `as_array()` | 取数组 |
| `as_object()` | 取对象 |
| `operator[](size_t)` | 按下标访问数组元素 |
| `operator[](string)` | 按键名访问对象成员 |
| `is_null()` / `is_bool()` 等 | 类型判断 |
| `dump(int indent)` | 序列化，indent 为缩进空格数（-1 为紧凑格式）|

---

## 项目结构

```
JosnParserTry/
├── README.md
├── CMakeLists.txt
├── .gitignore
├── include/
│   ├── json.h        # Json 类定义
│   └── parser.h      # Scanner / Parser / Token 定义
├── src/
│   ├── json.cpp      # Json 类实现（访问、序列化）
│   ├── parser.cpp    # 词法分析 + 递归下降解析
│   └── main.cpp      # 使用示例
└── test/
    └── text.cpp      # 测试用例（40 个）
```

---

## 依赖

- C++17 或更高标准
- CMake 3.10+
- 无第三方库依赖，仅使用 STL

---

## 编译与运行

### 使用 CMake（推荐）

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### 运行主程序（使用示例）

```bash
./JosnParserTry
```

### 运行测试

```bash
./JsonTest
```

---

## 使用示例

```cpp
Json json = Json::parse(R"({
    "name": "张三",
    "age": 20,
    "scores": [95, 87, 92],
    "address": {
        "city": "北京",
        "street": "中关村大街"
    }
})");

std::string name = json["name"].as_string();       // "张三"
int age          = json["age"].as_int();            // 20
double score     = json["scores"][0].as_double();   // 95.0
std::string city = json["address"]["city"].as_string(); // "北京"

// 序列化（4 空格缩进）
std::string output = json.dump(4);
```

### 错误处理

```cpp
try {
    Json::parse("{bad json}");
} catch (const std::exception& e) {
    // 输出：Line 1, Col 2: Unexpected character
    std::cout << e.what() << std::endl;
}
```

---

## 设计说明

- **词法分析（Scanner）**：逐字符扫描，将输入切分为 Token，每个 Token 记录行号和列号
- **语法分析（Parser）**：递归下降解析，按照 JSON 语法规则递归构建 Json 对象
- **数据存储**：使用 `std::variant` 存储六种类型，避免手动内存管理
- **性能优化**：Token 的 `value` 字段使用 `std::string_view` 直接指向原始字符串，减少拷贝