// msgbus.h: 标准系统包含文件的包含文件
// 或项目特定的包含文件。

#pragma once

#include <any>
#include <array>
#include <cassert>
#include <functional>
#include <iostream>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "magic_enum.hpp"

struct Msg {
    std::vector<std::any> params;
};

template<typename MsgID>
class MsgBus {
    using MessageCallback = std::function<void(Msg&)>;

public:
    static MsgBus& Instance() {
        static MsgBus instance;
        return instance;
    }

    void Register(MsgID id, MessageCallback&& handler) {
        handlers_[id] = std::move(handler);
    }

    void Unregister(MsgID id) {
        handlers_[id] = nullptr;
    }

private:
    template<typename MID, typename ArgTuple>
    friend struct MsgSender;

    void Send(MsgID id, Msg& msg) {
        std::cout << "Send: " << magic_enum::enum_name(id) << std::endl;
        assert(handlers_[id] && "message handler not registered");
        handlers_[id](msg);
    }

private:
    std::array<MessageCallback, MsgID::COUNT> handlers_;
};

template<typename T>
T wrapped_any_cast(std::any& any) {
    if constexpr (std::is_reference_v<T>) {
        if (auto r = std::any_cast<std::reference_wrapper<std::decay_t<T>>>(&any)) {
            return r->get();
        }
    }
    return std::any_cast<T>(any);
}

// 消息签名 trait：DEFINE_MESSAGE 的全部产物，签名定义与逻辑实现分离
template<auto MID>
struct MsgSig;

#define DEFINE_MESSAGE(MID, ...)                                          \
    template<>                                                            \
    struct MsgSig<MID> {                                                  \
        using Args = std::tuple<__VA_ARGS__>;                             \
    };

template<typename MID, typename ArgTuple>
struct MsgSender;

template<auto MID>
struct MsgHelper {
    using ArgTypes = typename MsgSig<MID>::Args;
    static constexpr size_t ArgCount = std::tuple_size_v<ArgTypes>;

    // 注册端：从成员函数指针推导参数列表，与 DEFINE_MESSAGE 声明的签名编译期比对
    template<typename Cls, typename... A>
    static void Register(Cls* p, void (Cls::*cb)(A...)) {
        static_assert(std::is_same_v<std::tuple<A...>, ArgTypes>,
                      "handler signature does not match DEFINE_MESSAGE");
        MsgBus<decltype(MID)>::Instance().Register(MID, [p, cb](Msg& msg) {
            apply(msg, p, cb, std::make_index_sequence<ArgCount>{});
        });
    }

    static void Unregister() {
        MsgBus<decltype(MID)>::Instance().Unregister(MID);
    }

    // 发送端：参数个数必须与签名严格一致，逐参数做编译期类型检查
    template<typename... U>
    static MsgSender<decltype(MID), ArgTypes> Sender(U&&... args) {
        static_assert(sizeof...(U) == ArgCount,
                      "argument count must match message signature");
        checkArgs<U...>(std::index_sequence_for<U...>{});
        return { MID, std::forward<U>(args)... };
    }

private:
    template<typename Cls, typename... A, size_t... I>
    static void apply(Msg& msg, Cls* plugin, void (Cls::*cb)(A...),
                      std::index_sequence<I...>) {
        (plugin->*cb)(wrapped_any_cast<std::tuple_element_t<I, ArgTypes>>(msg.params[I])...);
    }

    template<typename... U, size_t... I>
    static constexpr void checkArgs(std::index_sequence<I...>) {
        static_assert((argMatched<std::tuple_element_t<I, ArgTypes>, U>() && ...),
                      "argument type does not match message signature");
    }

    // 引用参数要求可绑定到左值（裸左值或 std::reference_wrapper）；
    // 值参数要求可按声明类型构造
    template<typename Arg, typename U>
    static constexpr bool argMatched() {
        if constexpr (std::is_reference_v<Arg>)
            return std::is_convertible_v<U, std::decay_t<Arg>&>;
        else
            return std::is_constructible_v<Arg, U>;
    }
};

template<typename MID, typename ArgTuple>
struct MsgSender {
    template<typename... U>
    MsgSender(MID id, U&&... params)
        : id_{ id }, msg_{ { pack(std::index_sequence_for<U...>{},
                                   std::forward<U>(params)...) } } {
    }

    auto Params() {
        return params(std::make_index_sequence<std::tuple_size_v<ArgTuple>>{});
    }

    void Invoke() {
        MsgBus<MID>::Instance().Send(id_, msg_);
    }

private:
    template<typename T>
    using ParamRef = std::add_lvalue_reference_t<std::decay_t<T>>;

    template<size_t... I>
    std::tuple<ParamRef<std::tuple_element_t<I, ArgTuple>>...>
    params(std::index_sequence<I...>) {
        return { wrapped_any_cast<ParamRef<std::tuple_element_t<I, ArgTuple>>>(msg_.params[I])... };
    }

    // 按声明类型存储：值参数原地构造，引用参数统一包装为 std::ref，
    // 保证 any 中的存储类型与签名恒一致，接收端还原不会失败
    template<typename Arg, typename U>
    static std::any packOne(U&& arg) {
        if constexpr (std::is_reference_v<Arg>) {
            std::decay_t<Arg>& ref = arg;
            return std::ref(ref);
        } else {
            return std::any(std::in_place_type_t<Arg>{}, std::forward<U>(arg));
        }
    }

    template<size_t... I, typename... U>
    static std::vector<std::any> pack(std::index_sequence<I...>, U&&... params) {
        return { packOne<std::tuple_element_t<I, ArgTuple>>(std::forward<U>(params))... };
    }

    MID id_;
    Msg msg_;
};
