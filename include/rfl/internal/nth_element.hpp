#ifndef RFL_INTERNAL_NTH_ELEMENT_HPP_
#define RFL_INTERNAL_NTH_ELEMENT_HPP_

#include <tuple>
#include <utility>

namespace rfl::internal {

template <class T, int N, int _i>
struct ElementWrapper {
  T t_;
};

template <int N, class T1, int _i1, class T2, int _i2>
constexpr auto operator|(const ElementWrapper<T1, N, _i1>& _e1,
                         const ElementWrapper<T2, N, _i2>& _e2) {
  if constexpr (_i1 == N) {
    return _e1;
  } else {
    return _e2;
  }
}

template <class Head, class... Tail>
constexpr auto find_matching_element(const Head& _head, const Tail&... _tail) {
  return (_head | ... | _tail);
};

template <int N, class... Ts, int... _is>
constexpr auto wrap_elements(Ts... _ts, std::integer_sequence<int, _is...>) {
  return find_matching_element(ElementWrapper<Ts, N, _is>{_ts}...).t_;
}

// 如果 Ts... Ts 是 TypeWrapper<int>, TypeWrapper<double>, TypeWrapper<char>
// N = 1
// 那么会生成以下
// ElementWrapper<TypeWrapper<int>, 1, 0>{}
// ElementWrapper<TypeWrapper<double>, 1, 1>{}
// ElementWrapper<TypeWrapper<char>, 1, 2>{}
// 然后会调用 find_matching_element
// 返回类型是 TypeWrapper<double>
template <int N, class... Ts>
constexpr auto nth_element(Ts... _ts) {
  static_assert(N >= 0, "N out of bounds.");
  static_assert(N < sizeof...(Ts), "N out of bounds.");
  return wrap_elements<N, Ts...>(
      _ts..., std::make_integer_sequence<int, sizeof...(Ts)>());
}

}  // namespace rfl::internal

#endif
