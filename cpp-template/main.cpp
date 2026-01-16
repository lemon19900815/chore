#include "meta_func.h"

#include <iomanip>
#include <iostream>
#include <chrono>

using namespace std::string_literals;
using namespace std::chrono_literals;

using namespace meta;

#include <utility>
#include <variant>

// log宏：多模板参数expr需要在外部使用括号把表达式括起来
#define LOG(expr) std::cout << __FUNCTION__ << ": " << #expr << " = " << expr << std::endl
#define LOG_LINE() std::cout << "===========================================================\n"

// test basic meta skills.
void test_basic();

// test empty base class optimization.
void test_ebco();

// test foreach tuple elements.
void test_for_tuple();

// test function traits.
void test_function_traits();

// test struct/class has member field or member function.
void test_has_field_or_func();

// test nothrow constructiable trait.
void test_nothrow_contructiable() noexcept;

// test std variant, vist.
void test_variant();

// test my variant implementation.
void test_my_variant();

int main(int argc, char** argv)
{
    // test basic meta skills.
    test_basic();

    // test empty base class optimization.
    test_ebco();

    // test foreach tuple elements.
    test_for_tuple();

    // test function traits.
    test_function_traits();

    // test struct/class has member field or member function.
    test_has_field_or_func();

    // test nothrow constructiable trait.
    test_nothrow_contructiable();

    // test std variant, vist.
    test_variant();

    // test my variant implementation.
    test_my_variant();

#ifdef _WIN32
    system("pause");
#endif

    return 0;
}

void test_ebco()
{
    LOG_LINE();

    class Empty {
        using Int = int;// type alias members don’ t make a class nonempty
    };
    class EmptyToo : public Empty {
    };
    class EmptyThree : public EmptyToo {
    };

    LOG(sizeof(Empty));
    LOG(sizeof(EmptyToo));
    LOG(sizeof(EmptyThree));
}

HAS_MEMBER_FUNCTION(print);
HAS_MEMBER_FIELD(x);
HAS_MEMBER_FIELD(y);

void test_has_field_or_func()
{
    LOG_LINE();

    struct test {
        void print() {}
        void print(int v) {}

        int x;

    private:
        int y;
    };

    struct st_foo
    {
        void foo() {}
    };

    constexpr auto has_print_void = Hasprint<test>; // true
    constexpr auto has_print_int = Hasprint<test, int>; // true
    constexpr auto has_print_str = Hasprint<test, std::string>; // false

    constexpr auto has_x = Hasx<test>; // true
    constexpr auto has_y = Hasy<test>; // false, private member.

    {
        // test has_mem_func
        LOG(has_mem_func<int>::value);
        LOG(has_mem_func<st_foo>::value);
    }

    {
        LOG(has_member_func_foo<int>::value);
        LOG(has_member_func_foo<st_foo>::value);

        // SFINAE(substitude failure is not an error)，
        // 可以通过enable_if来实现编译期的if-else逻辑
        LOG(call_func(1));
        LOG(call_func("lemon"s));
    }

    // c++17：实现任意成员的探测
    constexpr auto hasFirst = isValid([](auto x) ->
        decltype((void)valueT(x).first) {});

    LOG(hasFirst(type<int>));
    LOG(hasFirst(type<std::pair<int, int>>));
}

void test_variant()
{
    LOG_LINE();

    std::unordered_map<int32_t, std::string> map_str;
    std::vector<std::string> vec_str;

    LOG(TypeNameOf(map_str));
    LOG(TypeNameOf(vec_str));

    std::variant<decltype(map_str), decltype(vec_str)> var = map_str;

    std::visit([](auto&& r) {
        if constexpr (std::is_same_v<std::decay_t<decltype(r)>, std::unordered_map<int32_t, std::string>>) {
            std::cout << "test_variant: unordered_map constexpr " << std::endl;
        } else if constexpr (std::is_same_v<std::decay_t<decltype(r)>, std::vector<std::string>>) {
            std::cout << "test_variant: vector  constexpr" << std::endl;
        }
    }, var);

    // use Overload to visit variant. like my_variant.Visit(...)
    std::visit(Overload{ [](std::unordered_map<int32_t, std::string>& v) {
        std::cout << "test_variant: unordered_map Overload" << std::endl;
    }, [](std::vector<std::string>& v) {
        std::cout << "test_variant: vector Overload" << std::endl;
    } }, var);
}

void test_my_variant()
{
    LOG_LINE();

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
        std::cout << "test_my_variant: int: " << v << std::endl;
    }, [](double v) {
        std::cout << "test_my_variant: double: " << v << std::endl;
    }, [](char v) {
         std::cout << "test_my_variant: char: " << v << std::endl;
    }, [](int64_t v) {
        std::cout << "test_my_variant: int64_t: " << v << std::endl;
    });

    LOG(v.IndexOf<double>());
}

void test_function_traits()
{
    LOG_LINE();

    std::function<int(int)> f = [](int a) { return a; };
    auto f2 = [](int a) { return a; };
    print_type<function_traits<int(int)>::fn_type>("function_traits<int(int)>::fn_type = ");
    print_type<function_traits<int(int)>::args<0>::type>("function_traits<int(int)>::args<0>::type = ");
    print_type<function_traits<decltype(f)>::fn_type>("function_traits<decltype(f)>::fn_type = ");
    print_type<function_traits<decltype(f2)>::fn_type>("function_traits<decltype(f2)>::fn_type = ");

    struct MyTest {
        int operator()() { return 1; }
        void foo(int) {}
    };

    print_type<function_traits<decltype(&MyTest::foo)>::fn_type>("function_traits<decltype(&MyTest::foo) = ");
    print_type<function_traits<MyTest>::fn_type>("function_traits<decltype(MyTest) = ");
}

void test_for_tuple()
{
    LOG_LINE();

    // test for each on tuple.
    auto str = "1"s;
    auto xx = 1s;

    auto tp = std::make_tuple(1, 2, "lemon");
    constexpr auto SIZE = std::tuple_size_v<decltype(tp)>;

    // 遍历tuple
    for_each(tp, std::make_index_sequence<SIZE>{}, [](auto& item, auto idx) {
        std::cout << "test_for_tuple: idx = " << idx << ", value = " << item << std::endl;
    });
}

void test_basic()
{
    LOG_LINE();

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
        // test meta add.
        // 多模板参数需要使用quote符号把表达式括起来
        LOG((add<1, 2>::value));
    }

    {
        // use initializer_list and () expand param list.
        print_for_each<int, double, char, int64_t>();
    }

    {
        print_args(1, 3.14, 20L, 30LL);
    }
}

void test_nothrow_contructiable() noexcept
{
    LOG_LINE();

    struct test
    {
        test(test&&) = delete;
    };

    struct test2
    {
        test2(test2&&) {}
    };

    struct test3
    {
        test3(test3&&) noexcept {}
    };

    // test IsNothrowMoveContructibleT
    LOG(IsNothrowMoveContructibleT<test>::value);
    LOG(IsNothrowMoveContructibleT<test2>::value);
    LOG(IsNothrowMoveContructibleT<test3>::value);
}
