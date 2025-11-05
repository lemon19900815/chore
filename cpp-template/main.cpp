#include "meta_func.h"

#include <iomanip>
#include <iostream>
#include <chrono>

using namespace std::string_literals;
using namespace std::chrono_literals;

using namespace meta;

#include <utility>

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

struct st_foo
{
    void foo() {}
};

// log宏：多模板参数expr需要在外部使用括号把表达式括起来
#define LOG(expr) std::cout << #expr << " = " << expr << std::endl

class Empty {
    using Int = int;// type alias members don’ t make a class nonempty
};
class EmptyToo : public Empty {
};
class EmptyThree : public EmptyToo {
};

void test_ebco()
{
    LOG(sizeof(Empty));
    LOG(sizeof(EmptyToo));
    LOG(sizeof(EmptyThree));
}

int main(int argc, char** argv)
{
    {
        // test has_mem_func
        LOG(has_mem_func<int>::value);
        LOG(has_mem_func<st_foo>::value);
    }

    {
        struct test
        {
            test(test&&) = delete;
        };

        struct test2
        {
            test2(test2&&) { }
        };

        struct test3
        {
            test3(test3&&) noexcept { }
        };

        // test IsNothrowMoveContructibleT
        LOG(IsNothrowMoveContructibleT<test>::value);
        LOG(IsNothrowMoveContructibleT<test2>::value);
        LOG(IsNothrowMoveContructibleT<test3>::value);
    }

    {
        // c++17：实现任意成员的探测
        constexpr auto hasFirst = isValid([](auto x) ->
            decltype((void)valueT(x).first) {});

        LOG(hasFirst(type<int>));
        LOG(hasFirst(type<std::pair<int, int>>));
    }

    {
        // test meta add.
        // 多模板参数需要使用quote符号把表达式括起来
        LOG((add<1,2>::value));
    }

    {
        LOG(has_member_func_foo<int>::value);
        LOG(has_member_func_foo<st_foo>::value);

        // SFINAE(substitude failure is not an error)，
        // 可以通过enable_if来实现编译期的if-else逻辑
        LOG(call_func(1));
        LOG(call_func("lemon"s));
    }

    {
        // test for each on tuple.
        auto str = "1"s;
        auto xx = 1s;

        auto tp = std::make_tuple(1, 2, "lemon");
        constexpr auto SIZE = std::tuple_size_v<decltype(tp)>;

        // 遍历tuple
        for_each(tp, std::make_index_sequence<SIZE>{}, [](auto& item, auto idx) {
            std::cout << "idx = " << idx << ", value = " << item << std::endl;
        });
    }

    {
        LOG((IntMax_v<sizeof(int), sizeof(char*), sizeof(double), sizeof(int64_t)>));
        LOG((MaxAlign_v<int, char*, double, int64_t>));
    }

    {
        LOG((Contains_v<int, char*, double, int64_t>));
        LOG((Contains_v<int, char*, double, int64_t, int32_t>));
    }

    {
        // 注意IndexOf模板的问题，借助Contains模板解决
        LOG((IndexOf_v<int, char*, double, int64_t, int32_t>));
        LOG((IndexOf_v<int, char, char*, double, int64_t>));
    }

    {
        LOG(typeid(At_t<1, int, char, double, int64_t>).name());
        LOG(typeid(At_t<3, int, char, double, int64_t>).name());

        // 编译错误：不能匹配到索引类型，超出了参数列表的个数
        //std::cout << "At_t<4, int, char, double, int64_t> = "
        //          << typeid(At_t<4, int, char, double, int64_t>{}).name()
        //          << std::endl;
    }

    {
        // use initializer_list and () expand param list.
        print_for_each<int, double, char, int64_t>();
    }

    {
        print_args(1, 3.14, 20L, 30LL);
    }

    {
        std::function<int(int)> f = [](int a) {return a; };
        auto f2 = [](int a) {return a; };
        print_type<function_traits<int(int)>::fn_type>("function_traits<int(int)>::fn_type = ");
        print_type<function_traits<int(int)>::args<0>::type>("function_traits<int(int)>::args<0>::type = ");
        print_type<function_traits<decltype(f)>::fn_type>("function_traits<decltype(f)>::fn_type = ");
        print_type<function_traits<decltype(f2)>::fn_type>("function_traits<decltype(f2)>::fn_type = ");

        struct MyTest {
            int operator()() { return 1; }
            void foo(int ) { }
        };

        print_type<function_traits<decltype(&MyTest::foo)>::fn_type>("function_traits<decltype(&MyTest::foo) = ");
        print_type<function_traits<MyTest>::fn_type>("function_traits<decltype(MyTest) = ");
    }

    {
        struct MyTest {
            int a;
            int b;
            int c;
        };

        meta::Variant<int, double, char, int64_t> v(1);
        LOG(v.Type().name());

        v = 20ll;
        LOG(v.Type().name());

        v = 3.14;
        LOG(v.Type().name());

        v = 'a';
        LOG(v.Type().name());

        v.Visit([](int v) {
            std::cout << "int: " << v << std::endl;
        }, [](double v) {
            std::cout << "double: " << v << std::endl;
        }, [](char v) {
            std::cout << "char: " << v << std::endl;
        }, [](int64_t v) {
            std::cout << "int64_t: " << v << std::endl;
        });

        LOG(v.IndexOf<double>());
    }

#ifdef _WIN32
    system("pause");
#endif

    return 0;
}
