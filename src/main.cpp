#include <iostream>
#include <rfl/json.hpp>
#include <rfl.hpp>

struct Person {
  std::string first_name;
  std::string last_name;
  int age;
};

int main() {
    std::cout << "Hello, CMake!" << std::endl;
    const auto homer =
    Person{.first_name = "Homer",
           .last_name = "Simpson",
           .age = 45};

    // We can now write into and read from a JSON string.
    const std::string json_string = rfl::json::write(homer);
    auto homer2 = rfl::json::read<Person>(json_string).value();
    std::cout << "json_string: " << json_string << std::endl;
    std::cout << "homer2: " << homer2.first_name << " " << homer2.last_name << " " << homer2.age << std::endl;
    return 0;
} 