#pragma once

#include <type_traits>
#include <typeindex>
#include <string>
#include <iostream>
#include <tuple>
#include <functional>
#include <string_view>

// reference: https://www.cnblogs.com/qicosmos/p/4480460.html

namespace meta
{

template<int N, int M>
struct add
{
    static constexpr auto value = N + M;
};

template<typename... Types>
struct size_count;

template<typename First, typename... Rest>
struct size_count<First, Rest...>
{
    static constexpr auto value =
        size_count<First>::value + size_count<Rest...>::value;
};

template<typename First>
struct size_count<First>
{
    static constexpr auto value = sizeof(First);
};

template<typename... Types>
constexpr auto size_count_v = size_count<Types...>::value;

template<typename T>
struct has_member_func_foo
{
private:
    template<typename U>
    static constexpr auto check(U*) -> decltype(std::declval<U>().foo(), std::true_type{});

    template<typename>
    static constexpr std::false_type check(...);

public:
    static constexpr bool value = decltype(check<T>(nullptr))::value;
};

template<typename T, typename = std::void_t<>>
struct has_mem_func : std::false_type { };

// partial specialization (may be SFINAE’ d away):
template<typename T>
struct has_mem_func<T, std::void_t<decltype(std::declval<T>().foo())>> : std::true_type { };

// test sturct/class has member field or not.
#define HAS_MEMBER_FIELD(FieldName)                                                          \
    template<typename T, typename = std::void_t<>>                                           \
    struct has_##FieldName##_impl : std::false_type { };                                     \
    template<typename T>                                                                     \
    struct has_##FieldName##_impl<T, std::void_t<decltype(std::declval<T>().FieldName)>>     \
        : std::true_type { };                                                                \
    template<typename T>                                                                     \
    constexpr bool Has##FieldName = has_##FieldName##_impl<T>::value;                        \

// test struct/class has member function or not.
#define HAS_MEMBER_FUNCTION(FuncName)                                                        \
    template<typename T, typename A, typename = std::void_t<>>                               \
    struct has_##FuncName##_impl : std::false_type { };                                      \
    template<typename T, typename... Args>                                                   \
    struct has_##FuncName##_impl<T, std::tuple<Args...>,                                     \
        std::void_t<decltype(std::declval<T>().FuncName(std::declval<Args>()...))>>          \
        : std::true_type { };                                                                \
    template<typename T, typename... Args>                                                   \
    constexpr bool Has##FuncName = has_##FuncName##_impl<T, std::tuple<Args...>>::value;     \

/*
struct test {
    void print() { }
    void print(int v) { }

    int x;

private:
    int y;
};

HAS_MEMBER_FUNCTION(print);
HAS_MEMBER_FIELD(x);
HAS_MEMBER_FIELD(y);

constexpr auto has_print_void = Hasprint<test>; // true
constexpr auto has_print_int = Hasprint<test, int>; // true
constexpr auto has_print_str = Hasprint<test, std::string>; // false

constexpr auto has_x = Hasx<test>; // true
constexpr auto has_y = Hasy<test>; // false, private member.
*/

template<typename T>
struct IsDefaultConstructibleT {
private:
    template<typename U, typename = decltype(U())>
    static char test(void*);
    template<typename>
    static long test(...); // test() fallback:
public:
    static constexpr bool value = std::is_same<decltype(test<T>(nullptr)), char>::value;
};

template<typename T>
struct IsDefaultConstructibleTV2 {
private:
    template<typename U, typename = decltype(U())>
    static std::true_type test(void*);
    template<typename>
    static std::false_type test(...); // test() fallback:
public:
    static constexpr bool value = decltype(test<T>(nullptr))::value;
};

template<typename T>
struct IsDefaultConstructibleTHelper {
private:
    template<typename U, typename = decltype(U())>
    static std::true_type test(void*);
    template<typename>
    static std::false_type test(...); // test() fallback:
public:
    using Type = decltype(test<T>(nullptr));
};

template<typename T>
struct IsDefaultConstructibleTV3 : public IsDefaultConstructibleTHelper<T>::Type {};

// 判断是否存在不抛出异常的右值构造函数
template<typename T, typename = std::void_t<>>
struct IsNothrowMoveContructibleT : std::false_type {};

template<typename T>
struct IsNothrowMoveContructibleT<T, std::void_t<decltype(T(std::declval<T>()))>>
    : std::bool_constant<noexcept(T(std::declval<T>()))> {};

// c++14
template<typename T>
constexpr bool has_member_func_foo_v = has_member_func_foo<T>::value;

template<class T>
typename std::enable_if_t<std::is_arithmetic_v<T>, int> call_func(T t)
{
    //std::cout << "call_func(T t)" << std::endl;
    return 0;
}

template<class T>
typename std::enable_if_t<std::is_same_v<T, std::string>, int> call_func(T&& t)
{
    //std::cout << "call_func(T&& t)" << std::endl;
    return 1;
}

// tuple,c++17
template<typename... Args, typename F, size_t... Idx>
void for_each(const std::tuple<Args...>& tp, std::index_sequence<Idx...>, F&& f)
{
    (std::forward<F>(f)(std::get<Idx>(tp), std::integral_constant<size_t, Idx> {}),...);
}

// for each tuple on c++11, std::index_sequence for c++14
// must use self define idx_seq replace std::index_sequence
// although we can for each tuple on c++11, but that can't generic function (e.g. print)
template<typename Tuple, size_t... Idx>
void for_each_c11(Tuple& tp, std::index_sequence<Idx...>)
{
    // 方式1：使用swallow
    using swallow = int[];
    (void)swallow { 0, (void(print(std::get<Idx>(tp))), 0)... };

    // 方式2：使用std::initializer_list
    //std::initializer_list<int>{ (print(std::get<Idx>(tp)), 0)... };
}

template<typename T>
void print(T v)
{
    std::cout << v << std::endl;
}

// 获取整数序列的最大值

// 模板声明
template<size_t v, size_t... rest>
struct IntMax;

// 递归调用结束条件
template<size_t v>
struct IntMax<v> : std::integral_constant<size_t, v> { };

// template alias
template<size_t v, size_t... rest>
constexpr int IntMax_v = IntMax<v, rest...>::value;

// 模板实现
template<size_t v1, size_t v2, size_t... rest>
struct IntMax<v1, v2, rest...> : std::integral_constant<size_t,
    v1 >= v2 ? IntMax_v<v1, rest...> : IntMax_v<v2, rest...>> {};

// 内存对齐
template<typename... Args>
struct MaxAlign : std::integral_constant<size_t, IntMax_v<sizeof(Args)...>> {};

template<typename... Args>
constexpr auto MaxAlign_v = MaxAlign<Args...>::value;

// 参数类型是否包含指定类型
template<typename T, typename... Args>
struct Contains;

template<typename T, typename Head, typename... Rest>
struct Contains<T, Head, Rest...> :
    std::conditional_t<std::is_same_v<T, Head>, std::true_type, Contains<T, Rest...>> {};

template<typename T>
struct Contains<T> : std::false_type {};

template<typename T, typename...Args>
constexpr auto Contains_v = Contains<T, Args...>::value;

// 参数列表的索引：**这个模板函数是有问题的**
// 参数在展开的过程中首先匹配模板IndexOf<T, Head, Args...>，IndexOf<T, Args...>::value+1;
// 当匹配到最后成功0或者未匹配到-1时，依次往上递归，
// IndexOf<int, char,char*,int,int64>：2
// IndexOf<int, char,char*,double,int64>：3 错误，结果应为-1
// 使用第2个Index_v模板，借助Contains模板来完成IndexOf的工作
template<typename... Args>
struct IndexOf;

template<typename T>
struct IndexOf<T>
{
    static constexpr auto value = -1;
};

template<typename T, typename... Args>
struct IndexOf<T, T, Args...>
{
    static constexpr auto value = 0;
};

template<typename T, typename Head, typename... Args>
struct IndexOf<T, Head, Args...>
{
    static constexpr auto value = IndexOf<T, Args...>::value + 1;
};

//error：如果不含T类型，结果是错误的
//template<typename T, typename...Args>
//constexpr auto IndexOf_v = IndexOf<T, Args...>::value;

template<typename T, typename... Args>
constexpr auto IndexOf_v = std::conditional_t<Contains_v<T, Args...>,
    IndexOf<T, Args...>, std::integral_constant<int32_t, -1>>::value;

// 编译器查找类型
template<size_t Idx, typename... Args>
struct At;

template<size_t Idx, typename Head, typename... Args>
struct At<Idx, Head, Args...>
{
    static_assert(Idx <= sizeof...(Args), "At: index is out of range.");
    using type = typename At<Idx - 1, Args...>::type;
};

template<typename Head, typename... Args>
struct At<0, Head, Args...>
{
    using type = Head;
};

template<size_t Idx, typename... Args>
using At_t = typename At<Idx, Args...>::type;

template<typename T>
void print_type(std::string prefix="")
{
    std::cout << prefix << typeid(T).name() << std::endl;
}

template<typename... Args>
void print_for_each()
{
    std::initializer_list<int>{ (print_type<Args>(), 0)... };
}

// 定义整型序列
template<int...>
struct IndexSeq {};

// 生产整型序列
template<int N, int... Seq>
struct MakeSequence : MakeSequence<N - 1, N - 1, Seq...> {};

template<int... Seq>
struct MakeSequence<0, Seq...> {
    using type = IndexSeq<Seq...>;
};

template<typename T, typename... Args>
void print_arg(T t, Args... args)
{
    std::cout << typeid(T).name() << " = " << t << std::endl;
    if constexpr (sizeof...(Args) == 0) {
    } else {
        print_arg(args...);
    }
}

template<int... Seq, typename... Args>
void print_helper(IndexSeq<Seq...>, std::tuple<Args...>&& tp)
{
    // 2.再将tuple转换为可变参数模板，将参数还原回来，再调用print
    print_arg(std::get<Seq>(tp)...);
}

template<typename... Args>
void print_args(Args... args)
{
    // 1.先将可变模板参数保存到tuple中
    print_helper(typename MakeSequence<sizeof...(Args)>::type{}, std::make_tuple(args...));
}

///////////////////////////function_traits/////////////////////////////////////

template<typename T>
struct function_traits;

// 普通函数
template<typename Ret, typename... Args>
struct function_traits<Ret(Args...)>
{
    // enum { arity = sizeof...(Args) }
    static constexpr auto arity = sizeof...(Args);

    using return_type = Ret;
    using fn_type = return_type(Args...);
    using stl_fn_type = std::function<fn_type>;

    using fn_pointer = return_type(*)(Args...);

    template<size_t I>
    struct args
    {
        static_assert(I < arity, "index is out of range, index must less than sizeof Args");
        using type = typename std::tuple_element<I, std::tuple<Args...>>::type;
    };
};

// 函数指针
template<typename Ret, typename... Args>
struct function_traits<Ret(*)(Args...)> : function_traits<Ret(Args...)> {};

// std::function
template<typename Ret, typename... Args>
struct function_traits<std::function<Ret(Args...)>> : function_traits<Ret(Args...)> {};

// member function
#define FUNCTION_TRAITS(...)\
    template<typename Ret, typename Cls, typename... Args>\
    struct function_traits<Ret(Cls::*)(Args...) __VA_ARGS__> : function_traits<Ret(Args...)> {};

FUNCTION_TRAITS()
FUNCTION_TRAITS(const)
FUNCTION_TRAITS(volatile)
FUNCTION_TRAITS(const volatile)

// 函数对象
template<typename Callable>
struct function_traits : function_traits<decltype(&Callable::operator())> {};

template<typename Function>
typename function_traits<Function>::stl_fn_type to_function(Function&& lbd) {
    return static_cast<typename function_traits<Function>::stl_fn_type>(std::forward<Function>(lbd));
}

template<typename Function>
typename function_traits<Function>::fn_pointer to_function(Function&& lbd) {
    return static_cast<typename function_traits<Function>::fn_pointer>(std::forward<Function>(lbd));
}

///////////////////////////function_traits/////////////////////////////////////

template<typename... Types>
class Variant
{
    static constexpr auto data_size = IntMax_v<sizeof...(Types)>;
    static constexpr auto align_size = MaxAlign_v<Types...>;

    using data_type = typename std::aligned_storage_t<data_size, align_size>;

public:
    template<int index>
    using index_type = At_t<index, Types...>;

    Variant(void) : type_idx_(typeid(void)) {
    }
    ~Variant() {
        Destory(type_idx_, &data_);
    }

    Variant(Variant&& rhs) : type_idx_(rhs.type_idx_) {
        Move0(rhs.type_idx_, &rhs.data_, &data_);
    }

    Variant(const Variant& rhs) : type_idx_(rhs.type_idx_) {
        Copy0(rhs.type_idx_, &rhs.data_, &data_);
    }

    template<typename T,
        typename = typename std::enable_if_t<Contains_v<std::remove_cv_t<T>, Types...>>>
    Variant(T&& value) : type_idx_(typeid(void)) {
        Destory(type_idx_, &data_);

        using U = std::remove_cv_t<T>;

        type_idx_ = std::type_index(typeid(U));
        new (&data_) U(std::forward<T>(value));
    }

    template<typename T,
        typename = typename std::enable_if_t<Contains_v<std::remove_cv_t<T>, Types...>>>
    Variant& operator=(T&& value) {
        Destory(type_idx_, &data_);

        using U = std::remove_cv_t<T>;

        type_idx_ = std::type_index(typeid(U));
        new (&data_) U(std::forward<T>(value));
        return *this;
    }

    template<typename T>
    std::decay_t<T> Get() {
        using U = std::decay_t<T>;
        if (!Is<U>()) {
            std::cout << typeid(U).name()
                      << " is not defined, current type = "
                      << type_idx_.name() << std::endl;
            throw std::bad_cast{};
        }

        return *(U*)(&data_);
    }

    template<typename F, typename... Rest>
    void Visit(F&& f, Rest&&... rest) {
        using U = typename function_traits<F>::template args<0>::type;
        if (Is<U>()) {
            f(Get<U>());
        } else if constexpr(sizeof...(rest) != 0) {
            Visit(std::forward<Rest>(rest)...);
        } else { }
    }

    bool IsEmpty() {
        return Is<void>();
    }

    std::type_index Type() {
        return type_idx_;
    }

    template<typename T>
    int32_t IndexOf() {
        return IndexOf_v<T, Types...>;
    }

private:
    template<typename T>
    bool Is() { return Is<T>(type_idx_); }

    template<typename T>
    bool Is(const std::type_index& id) {
        return id == std::type_index(typeid(T));
    }

    void Destory(const std::type_index& id, void* buf) {
        std::initializer_list<int> { (Destory0<Types>(id, buf), 0)... };
    }

    template<typename T>
    void Destory0(const std::type_index& id, void* data) {
        if (Is<T>(id)) {
            reinterpret_cast<T*>(data)->~T();
        }
    }

    void Move(const std::type_index& old_id, void* old_v, void* new_v) {
        std::initializer_list<int> { (Move0<Types>(old_id, old_v, new_v), 0)... };
    }

    template<typename T>
    void Move0(const std::type_index& old_id, void* old_v, void* new_v) {
        if (Is<T>(old_id)) {
            new (new_v) T(std::move(*reinterpret_cast<T*>(old_v)));
        }
    }

    void Copy(const std::type_index& old_id, void* old_v, void* new_v) {
        std::initializer_list<int> { (Copy0<Types>(old_id, old_v, new_v), 0)... };
    }

    template<typename T>
    void Copy0(const std::type_index& old_id, void* old_v, void* new_v) {
        if (Is<T>(old_id)) {
            new (new_v) T(*reinterpret_cast<T*>(old_v));
        }
    }

private:
    data_type data_;
    std::type_index type_idx_{ typeid(void) }; // 类型id
};

template<typename... T>
struct Overload : T...
{
    using T::operator()...;
};

template<typename... T>
Overload(T &&...) -> Overload<T...>;

template<typename T>
constexpr auto TypeNameOf()
{
    using namespace std::string_view_literals;

#ifdef __GNUC__
    std::string_view name = __PRETTY_FUNCTION__;
#ifdef __clang__
    constexpr auto prefix = "auto TypeNameOf() [T = "sv;
#else
    constexpr auto prefix = "constexpr auto TypeNameOf() [with T = "sv;
#endif
    constexpr auto suffix = "]"sv;
#endif

#ifdef _MSC_VER
    std::string_view name = __FUNCSIG__;
    constexpr auto prefix = "auto __cdecl TypeNameOf<class "sv;
    constexpr auto suffix = ">(void)"sv;
#endif

    name.remove_prefix(prefix.size());
    name.remove_suffix(suffix.size());
    return name;
}

template<typename T>
constexpr auto TypeNameOf(T&&)
{
    return TypeNameOf<std::remove_cv_t<T>>();
}

// helper: checking validity of f (args...) for F f and Args... args:
template<typename F, typename... Args,
    typename = decltype(std::declval<F>() (std::declval<Args&&>()...))>
std::true_type isValidImpl(void*);

// fallback if helper SFINAE’ d out:
template<typename F, typename... Args>
std::false_type isValidImpl(...);

// define a lambda that takes a lambda f and returns whether calling f with args is valid
inline constexpr auto isValid = [](auto f) {
    return [](auto&& ... args) {
        return decltype(isValidImpl<decltype(f),
            decltype(args) && ...>(nullptr)){};
        };
    };

// helper template to represent a type as a value
template<typename T>
struct TypeT {
    using Type = T;
};

// helper to wrap a type as a value
template<typename T>
constexpr auto type = TypeT<T>{};

// helper to unwrap a wrapped type in unevaluated contexts
template<typename T>
T valueT(TypeT<T>); // no definition needed

// primary template: yield the second argument by default and rely on
// a partial specialization to yield the third argument
// if Cond is false
template<bool Cond, typename TrueType, typename FalseType>
struct IfThenElseT {
    using Type = TrueType;
};

// partial specialization: false yields third argument
template<typename TrueType, typename FalseType>
struct IfThenElseT<false, TrueType, FalseType> {
    using Type = FalseType;
};

template<bool Cond, typename TrueType, typename FalseType>
using IfThenElse = typename IfThenElseT<Cond, TrueType,
    FalseType>::Type;

// yield T when using member Type:
template<typename T>
struct IdentityT {
    using Type = T;
};

// to make unsigned after IfThenElse was evaluated:
template<typename T>
struct MakeUnsignedT {
    using Type = typename std::make_unsigned<T>::type;
};

//template<typename T>
//struct UnsignedT {
//    using Type = IfThenElse<std::is_integral<T>::value
//        && !std::is_same<T, bool>::value, typename std::make_unsigned<T>::type,
//        T>;
//};

template<typename T>
struct UnsignedT {
    using Type = typename IfThenElse<std::is_integral<T>::value && !std::is_same<T, bool>::value,
        MakeUnsignedT<T>,
        IdentityT<T>>::Type;
};

} // namespace meta
