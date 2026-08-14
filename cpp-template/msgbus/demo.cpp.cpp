// msgbus.cpp: 定义应用程序的入口点。
//

#include <iostream>
#include "msgbus.h"

enum MsgID {
    ID1,
    ID2,
    ID3,
    ID4,
    ID5,
    COUNT,
};

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
    MsgHelper<ID3>::Sender("abc").Invoke();  // 隐式转换在发送端完成，存储类型恒与签名一致

    std::string ss;
    auto sender = MsgHelper<ID4>::Sender(ss);  // 裸左值自动包装为 std::ref
    auto [out] = sender.Params();
    sender.Invoke();

    std::cout << "resp from ID4: " << out << std::endl;

    std::string x;
    MsgHelper<ID4>::Sender(std::ref(x)).Invoke();
    std::cout << "resp from ID4: " << x << std::endl;

    MsgHelper<ID5>::Sender(x, 5).Invoke();

    std::cout << "resp from ID5: " << x << std::endl;

    system("pause");
    return 0;
}
