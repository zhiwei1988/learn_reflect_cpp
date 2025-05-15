// #include <iostream>
// #include <rfl.hpp>
// #include <rfl/json.hpp>
#include "issl/issl_tuple.hpp"
#include <iostream>
#include <string>
#include <vector>
#include "rfl.hpp"

// 定义一个普通的 C++ 结构体，没有使用 rfl::Field 包装
struct Person {
    std::string name;          // 普通 std::string 类型
    int age;                   // 内置 int 类型
    double salary;             // 内置 double 类型
    std::vector<std::string> hobbies;  // 标准库容器类型
};

int main() {
   // 创建结构体实例
    Person person{"张三", 30, 8000.0, {"读书", "旅游"}};
    
    // 使用 fields() 获取结构体字段元信息
    auto meta_fields = rfl::fields<Person>();
    
    // 输出元信息
    std::cout << "Person 结构体有 " << meta_fields.size() << " 个字段:" << std::endl;
    for (const auto& field : meta_fields) {
        std::cout << "字段名: " << field.name() << ", 类型: " << field.type() << std::endl;
    }

    return 0;
}

#if 0
struct Person
{
    std::string first_name;
    std::string last_name;
    int age;
};

int main()
{
    std::cout << "Hello, CMake!" << std::endl;
    const auto homer = Person{.first_name = "Homer", .last_name = "Simpson", .age = 45};

    // We can now write into and read from a JSON string.
    const std::string json_string = rfl::json::write(homer);
    auto homer2 = rfl::json::read<Person>(json_string).value();
    std::cout << "json_string: " << json_string << std::endl;
    std::cout << "homer2: " << homer2.first_name << " " << homer2.last_name << " " << homer2.age << std::endl;
    return 0;
}
#endif