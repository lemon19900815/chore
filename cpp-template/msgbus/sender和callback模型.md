# sender和callback模型


- 该模型可以在编译期匹配sender发送的参数类型与callback接收的类型。
- 当参数不匹配时，编译时报错提前暴露问题。

`msgbus.h`：
```c++
#include <any>
#include <array>
#include <unordered_map>

struct Msg {
    std::vector<std::any> params;
};

template<typename MID>
class MsgBus {
    using MessageCallback = std::function<void(Msg&)>;

public:
    static MsgBus& Instance() {
        static MsgBus instance;
        return instance;
    }

    void Register(MID id, MessageCallback&& handler) {
        handlers_[id] = std::move(handler);
    }

    void Unregister(MID id) {
        handlers_[id] = nullptr;
    }

private:
     template<typename MID, typename... T>
     friend struct MsgSender;

    void Send(MID id, Msg msg) {
        if (handlers_[id]) {
            handlers_[id](msg);
        }
    }

private:
    std::array<MessageCallback, MID::COUNT> handlers_;
};

template<typename T>
T wrapperd_any_cast(std::any& any) {
    if constexpr (std::is_reference_v<T>) {
        if (auto r = std::any_cast<std::reference_wrapper<std::decay_t<T>>>(&any))
        {
            return *r;
        }
    }
    return std::any_cast<T>(any);
}

template<typename T, typename U>
auto tryRef(U&& value) {
    if constexpr (std::is_reference_v<T>)
        return std::ref(std::forward<U>(value));
    else
        return std::forward<U>(value);
}

template<auto MID>
struct MsgHelper;

#define DEFINE_MESSAGE(MID, ...)                                                                     \
template<>                                                                                           \
struct MsgHelper<MID> {                                                                              \
    template<typename Cls>                                                                           \
    using CbType = void(Cls::*)(##__VA_ARGS__);                                                      \
                                                                                                     \
    using ArgTypes = std::tuple<##__VA_ARGS__>;                                                      \
                                                                                                     \
    template<typename Cls>                                                                           \
    static void Register(Cls* p, CbType<Cls> cb) {                                                   \
        MsgBus<decltype(MID)>::Instance().Register(MID, [p, cb](Msg& msg) {                          \
            apply(msg, p, cb, std::make_index_sequence<std::tuple_size_v<ArgTypes>>{});              \
        });                                                                                          \
    }                                                                                                \
                                                                                                     \
    static void Unregister() {                                                                       \
        MsgBus<decltype(MID)>::Instance().Unregister(MID);                                           \
    }                                                                                                \
                                                                                                     \
    template<typename... U>                                                                          \
    static MsgSender<decltype(MID), ##__VA_ARGS__> Sender(U &&...args) {                             \
        static_assert(sizeof...(U) <= std::tuple_size_v<ArgTypes>);                                  \
        convertiableAssert<U...>(std::index_sequence_for<U...>{});                                   \
        return { MID, std::forward<U>(args)... };                                                    \
    }                                                                                                \
                                                                                                     \
private:                                                                                             \
    template<typename T, size_t... I>                                                                \
    static void apply(Msg &msg, T *plugin, CbType<T> cb, std::index_sequence<I...>) {                \
        (plugin->*cb)(wrapperd_any_cast<std::tuple_element_t<I, ArgTypes>>(msg.params[I])...);       \
    }                                                                                                \
                                                                                                     \
    template<typename... U, size_t... I>                                                             \
    static constexpr void convertiableAssert(std::index_sequence<I...>) {                            \
        static_assert((std::is_convertible_v<U, std::tuple_element_t<I, ArgTypes>> && ...),          \
                        "function signature not match");                                             \
    }                                                                                                \
};                                                                                                   \

template<typename MID, typename... T>
struct MsgSender {
    template<typename... U, std::enable_if_t<sizeof...(U) <= sizeof...(T), int> = 0>
    MsgSender(MID id, U &&...params)
        : id_{ id }, msg_{ { makeParams(std::index_sequence_for<U...>{},
                              std::make_index_sequence<sizeof...(T) - sizeof...(U)>{},
                              std::forward<U>(params)...) } } { }

    auto Params() {
        return params(std::index_sequence_for<T...>{});
    }

    void Invoke() {
        MsgBus<MID>::Instance().Send(id_, msg_);
    }

private:
    template<typename T>
    using ParamRef = std::add_lvalue_reference_t<std::decay_t<T>>;

    template<size_t... I>
    std::tuple<ParamRef<T>...> params(std::index_sequence<I...>) {
        return { wrapperd_any_cast<ParamRef<T>>(msg_.params[I])... };
    }

    template<size_t... ParamI, size_t... DefaultI, typename... U>
    std::vector<std::any> makeParams(std::index_sequence<ParamI...>, std::index_sequence<DefaultI...>, U &&...params) {
        constexpr auto offset = sizeof...(U);
        using ParamTypes = std::tuple<T...>;
        return { tryRef<std::tuple_element_t<ParamI, ParamTypes>>(std::forward<U>(params))...,
                 std::remove_reference_t<std::tuple_element_t<DefaultI + offset, ParamTypes>>{}... };
    }

    MID id_;
    Msg msg_;
};
```

`demo.cpp`：
```c++
#include "msg_bus.h"

DEFINE_MESSAGE(ID1);
DEFINE_MESSAGE(ID2, int);
DEFINE_MESSAGE(ID3, std::string);
DEFINE_MESSAGE(ID4, std::string&);
DEFINE_MESSAGE(ID5, std::string&, int);


struct Event {
    Event() {
        MsgHelper<ID1>::Register(this, &Event::handle1);
        MsgHelper<ID2>::Register(this, &Event::handle2);
        MsgHelper<ID3>::Register(this, &Event::handle3);
        MsgHelper<ID4>::Register(this, &Event::handle4);
        MsgHelper<ID5>::Register(this, &Event::handle5);
    }

    ~Event() {
        MsgHelper<ID1>::Unregister();
        MsgHelper<ID2>::Unregister();
        MsgHelper<ID3>::Unregister();
        MsgHelper<ID4>::Unregister();
        MsgHelper<ID5>::Unregister();
    }

    void handle1() {
        std::cout << "handle1." << std::endl;
    }

    void handle2(int value) {
        std::cout << "handle2: " << value << std::endl;
    }

    void handle3(std::string s) {
        std::cout << "handle3: " << s << std::endl;
    }

    void handle4(std::string& s) {
        s = __FUNCTION__;
    }

    void handle5(std::string& s, int v) {
        s = __FUNCTION__;
    }
};

int main()
{
    Event h;

    MsgHelper<ID1>::Sender().Invoke();
    MsgHelper<ID2>::Sender(10).Invoke();
    MsgHelper<ID3>::Sender(std::string("abc")).Invoke();

    auto sender = MsgHelper<ID4>::Sender();
    auto [ss] = sender.Params();
    sender.Invoke();

    std::cout << "resp from ID4: " << ss << std::endl;

    std::string x;
    MsgHelper<ID4>::Sender(std::ref(x)).Invoke();
    std::cout << "resp from ID4: " << x << std::endl;

    auto sender5 = MsgHelper<ID5>::Sender(std::ref(x));
    //auto [p1, p2] = sender5.Params();
    sender5.Invoke();

    std::cout << "resp from ID5: " << x << std::endl;

    return 0;
}
```

**注意：**
sender构建的参数个数可以少于callback的个数，从MsgSender的构造函数中可以知道这一点。

如果希望在Send内打印消息ID，可以使用 [magic_enum](https://github.com/Neargye/magic_enum)

`magic_enum log` 示例：
```c++
Send(MsgID id, Msg msg) {
	std::cout << magic_enum::enum_name(id) << std::endl;
	...
}
```

`msgbus.h & msgbus.cpp` 模型是严格限制了参数数量的，Sender必须传入参数与callback参数必须严格匹配。