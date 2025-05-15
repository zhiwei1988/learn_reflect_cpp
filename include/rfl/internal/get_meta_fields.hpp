#ifndef RFL_INTERNAL_GETMETAFIELDS_HPP_
#define RFL_INTERNAL_GETMETAFIELDS_HPP_

#include <array>
#include <tuple>
#include <utility>

#include "../MetaField.hpp"
#include "../Tuple.hpp"
#include "../type_name_t.hpp"

namespace rfl {
namespace internal {

// class NamedTupleType 只接受 rfl::NamedTuple 类型
template <class NamedTupleType, class... AlreadyExtracted>
auto get_meta_fields(AlreadyExtracted&&... _already_extracted) {
  constexpr size_t i = sizeof...(_already_extracted);
  if constexpr (NamedTupleType::size() == i) {
    return std::array<MetaField, i>{std::move(_already_extracted)...};
  } else {
    // Fields 的类型是 using Fields = rfl::Tuple<std::remove_cvref_t<FieldTypes>...>;
    // template <int N, class T>
    // using tuple_element_t = typename rfl::tuple_element<N, std::remove_cvref_t<T>>::type;
    
    // template <int N, class... Ts>
    // struct tuple_element<i, typename NamedTupleType::Fields> {
    //   using type = internal::nth_element_t<i, std::remove_cvref_t<FieldTypes>...>;
    // };
    using FieldType = tuple_element_t<i, typename NamedTupleType::Fields>;
    // FieldType 是 rfl::Tuple 中第 i 个元素的类型
    // 这里假设 Tuple 中的类型都是 Field<Name, Type>
    auto name = typename FieldType::Name().str();
    auto type = type_name_t<typename FieldType::Type>().str();
    return get_meta_fields<NamedTupleType>(
        std::move(_already_extracted)...,
        MetaField(std::move(name), std::move(type)));
  }
}

}  // namespace internal
}  // namespace rfl

#endif
