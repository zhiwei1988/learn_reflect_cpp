// #include <iostream>
// #include <rfl.hpp>
// #include <rfl/json.hpp>
#include "issl/issl_tuple.hpp"
#include <iostream>
#include <string>

int main() {
    // 创建一个包含不同类型的 Tuple
    issl::Tuple<int, std::string, double> tuple(42, "Hello", 3.14);

    // 使用 get<N>() 访问元素
    std::cout << "第一个元素 (int): " << tuple.get<0>() << std::endl;
    std::cout << "第二个元素 (string): " << tuple.get<1>() << std::endl;
    std::cout << "第三个元素 (double): " << tuple.get<2>() << std::endl;

    // 使用移动构造创建新的 Tuple
    auto tuple2 = issl::Tuple<int, std::string, double>(123, "World", 2.718);

#ifdef NEED_USE
    // 比较两个 Tuple
    if (tuple < tuple2) {
        std::cout << "tuple1 小于 tuple2" << std::endl;
    }

    // 赋值操作
    tuple = tuple2;
    std::cout << "赋值后的第一个元素: " << tuple.get<0>() << std::endl;
    std::cout << "赋值后的第二个元素: " << tuple.get<1>() << std::endl;
#endif

    // 创建一个空的 Tuple
    issl::Tuple<> empty_tuple;

    // 创建一个只包含一个元素的 Tuple
    issl::Tuple<int> single_element(10);
    std::cout << "单元素 Tuple: " << single_element.get<0>() << std::endl;

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