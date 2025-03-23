#ifndef ISSL_ISSL_TUPLE_HPP_
#define ISSL_ISSL_TUPLE_HPP_

#include <array>
#include <tuple>
#include <utility>

namespace issl::internal {
template <class T, int N, int _i>
struct ElementWrapper
{
    T t_;
};

template <int N, class T1, int _i1, class T2, int _i2>
constexpr auto operator|(const ElementWrapper<T1, N, _i1> &_e1, const ElementWrapper<T2, N, _i2> &_e2)
{
    if constexpr (_i1 == N) {
        return _e1;
    } else {
        return _e2;
    }
}

template <class Head, class... Tail>
constexpr auto find_matching_element(const Head &_head, const Tail &..._tail)
{
    return (_head | ... | _tail);
};

template <int N, class... Ts, int... _is>
constexpr auto wrap_elements(Ts... _ts, std::integer_sequence<int, _is...>)
{
    return find_matching_element(ElementWrapper<Ts, N, _is>{_ts}...).t_;
}

template <int N, class... Ts>
constexpr auto nth_element(Ts... _ts)
{
    static_assert(N >= 0, "N out of bounds.");
    static_assert(N < sizeof...(Ts), "N out of bounds.");
    return wrap_elements<N, Ts...>(_ts..., std::make_integer_sequence<int, sizeof...(Ts)>());
}

// std::tuple 可以通过 std::tuple_element_t 获取第N个元素的类型
// 但是这里我们用的是自定义的 Tuple，所以需要自己实现
#if defined(__clang__)

template <int N, class... Ts>
using nth_element_t = __type_pack_element<N, Ts...>;

#else

template <class T>
struct TypeWrapper {
  using Type = T;
};

template <int N, class... Ts>
using nth_element_t =
    typename std::invoke_result_t<decltype(nth_element<N, TypeWrapper<Ts>...>),
                                  TypeWrapper<Ts>...>::Type;

#endif

template <class T1, class T2>
inline T1 ptr_cast(T2* _ptr) {
  return static_cast<T1>(static_cast<void*>(_ptr));
}

template <class T1, class T2>
inline T1 ptr_cast(const T2* _ptr) {
  return static_cast<T1>(static_cast<const void*>(_ptr));
}
} // namespace issl::internal

namespace issl {

template <class T>
struct PositionWrapper
{
};

template <unsigned long _last, unsigned long... _is>
struct Positions
{
    static consteval auto to_array() { return std::array<unsigned long, sizeof...(_is) + 1>({_is..., _last}); }
};

template <class T, unsigned long _last, unsigned long... _is>
consteval auto operator+(const Positions<_last, _is...> &, const PositionWrapper<T> &)
{
    if constexpr (_last % alignof(T) == 0) {
        constexpr auto last_new = _last + sizeof(T);
        return Positions<last_new, _is..., _last>{};
    } else {
        constexpr auto last_corrected = _last + alignof(T) - (_last % alignof(T));
        constexpr auto last_new = last_corrected + sizeof(T);
        return Positions<last_new, _is..., last_corrected>{};
    }
}

template <class... Types>
consteval auto calculate_positions()
{
    return (Positions<0>{} + ... + PositionWrapper<Types>{}).to_array();
}

template <class... FieldTypes>
class Tuple;

template <>
class Tuple<> {
  public:
    Tuple() {}
};

template <class... Types>
class Tuple {
    static constexpr size_t size_ = sizeof...(Types);

    // 计算Tuple中每个元素在内存中的位置
    static constexpr auto positions_ = calculate_positions<Types...>();

    // 创建一个整数序列，用于遍历Tuple中的每个元素
    static constexpr auto seq_ = std::make_integer_sequence<int, size_>{};

    // 计算Tuple中所有元素占用的内存大小
    static constexpr unsigned int num_bytes_ = std::get<size_>(positions_);

    // 定义一个用于存储Tuple中所有元素的内存块
    using DataType = std::array<unsigned char, num_bytes_>;

  public:
    // 构造函数，接受任意数量的参数，并将其存储在Tuple中
    Tuple(const Types &..._t) { copy_from_types(_t..., seq_); }

    Tuple(Types &&..._t) noexcept { move_from_types(std::move(_t)..., seq_); }

    ~Tuple() { destroy_if_necessary(seq_); }

    // 通过 index 获取元素的引用
    template <int _index>
    constexpr auto &get()
    {
        using Type = internal::nth_element_t<_index, Types...>;
        return *internal::ptr_cast<Type *>(data_.data() + pos<_index>());
    }

    // 通过 index 获取元素的常量引用
    template <int _index>
    constexpr const auto &get() const
    {
        using Type = internal::nth_element_t<_index, Types...>;
        return *internal::ptr_cast<const Type *>(data_.data() + pos<_index>());
    }

#ifdef NEED_USE
    /// Assigns the underlying object.
    Tuple<Types...> &operator=(const Tuple<Types...> &_other)
    {
        if (this == &_other) {
            return *this;
        }
        auto temp = Tuple<Types...>(_other);
        destroy_if_necessary(seq_);
        move_from_other(std::move(temp), seq_);
        return *this;
    }

    /// Assigns the underlying object.
    Tuple<Types...> &operator=(Tuple<Types...> &&_other) noexcept
    {
        if (this == &_other) {
            return *this;
        }
        destroy_if_necessary(seq_);
        move_from_other(std::move(_other), seq_);
        return *this;
    }

    /// Equality operator.
    template <class... OtherTypes>
    bool operator==(const Tuple<OtherTypes...> &_other) const noexcept
    {
        static_assert(sizeof...(Types) == sizeof...(OtherTypes), "The size of the two tuples must be the same.");
        const auto is_same = [&]<int _i>(std::integral_constant<int, _i>) -> bool {
            return this->get<_i>() == _other.template get<_i>();
        };
        return [&]<int... _is>(std::integer_sequence<int, _is...>) {
            return (true && ... && is_same(std::integral_constant<int, _is>{}));
        }(std::make_integer_sequence<int, sizeof...(Types)>());
    }

    /// Three-way comparison operator.
    template <class... OtherTypes>
    auto operator<=>(const Tuple<OtherTypes...> &_other) const noexcept
    {
        static_assert(sizeof...(Types) == sizeof...(OtherTypes), "The size of the two tuples must be the same.");

        const auto compare = [&]<int _i>(std::strong_ordering *_ordering, std::integral_constant<int, _i>) {
            if (*_ordering != std::strong_ordering::equivalent && this->get<_i>() != _other.template get<_i>()) {
                *_ordering = (this->get<_i>() <=> _other.template get<_i>());
            }
        };

        return [&]<int... _is>(std::integer_sequence<int, _is...>) {
            auto ordering = std::strong_ordering::equivalent;
            (compare(&ordering, std::integral_constant<int, _is>{}), ...);
            return ordering;
        }(std::make_integer_sequence<int, sizeof...(Types)>());
    }
#endif

private:
#ifdef NEED_USE
    template <int... _is>
    void copy_from_other(const Tuple<Types...> &_other, std::integer_sequence<int, _is...>)
    {
        const auto copy_one = [this]<int _i>(const auto &_other, std::integral_constant<int, _i>) {
            using Type = internal::nth_element_t<_i, Types...>;
            ::new (static_cast<void *>(data_.data() + pos<_i>())) Type(_other.template get<_i>());
        };
        (copy_one(_other, std::integral_constant<int, _is>{}), ...);
    }
#endif
    template <int... _is>
    void copy_from_types(const Types &..._types, std::integer_sequence<int, _is...>)
    {
        const auto copy_one = [this]<int _i>(const auto &_t, std::integral_constant<int, _i>) {
            using Type = internal::nth_element_t<_i, Types...>;
            // 使用 placement new 在预定位置构造对象
            ::new (static_cast<void *>(data_.data() + pos<_i>())) Type(_t);
        };
        (copy_one(_types, std::integral_constant<int, _is>{}), ...);
    }

    template <int... _is>
    void destroy_if_necessary(std::integer_sequence<int, _is...>)
    {
        const auto destroy_one = [](auto &_t) {
            using Type = std::remove_cvref_t<decltype(_t)>;
            if constexpr (std::is_destructible_v<Type>) {
                _t.~Type();
            }
        };
        (destroy_one(get<_is>()), ...);
    }

#ifdef NEED_USE
    template <int... _is>
    void move_from_other(Tuple<Types...> &&_other, std::integer_sequence<int, _is...>)
    {
        const auto move_one = [this]<int _i>(auto &&_other, std::integral_constant<int, _i>) {
            using Type = internal::nth_element_t<_i, Types...>;
            ::new (static_cast<void *>(data_.data() + pos<_i>())) Type(std::move(_other.template get<_i>()));
        };
        (move_one(_other, std::integral_constant<int, _is>{}), ...);
    }
#endif
    template <int... _is>
    void move_from_types(Types &&..._types, std::integer_sequence<int, _is...>)
    {
        const auto move_one = [this]<int _i>(auto &&_t, std::integral_constant<int, _i>) {
            using Type = internal::nth_element_t<_i, Types...>;
            ::new (static_cast<void *>(data_.data() + pos<_i>())) Type(std::move(_t));
        };
        (move_one(std::move(_types), std::integral_constant<int, _is>{}), ...);
    }

    template <int _i>
    static consteval unsigned int pos()
    {
        return std::get<_i>(positions_);
    }

  private:
    /// The underlying data, can be any of the underlying types.
    // 当有多个 alignas 说明符时，实际的对齐要求是所有指定对齐值的最大值
    alignas(Types...) DataType data_;
};

} // namespace issl

#endif