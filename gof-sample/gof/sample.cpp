#include "Factory/shape.h"
#include "Factory/producer_factory.h"
#include "Singleton/gameworld.h"
#include "Builder/meal.h"
#include "Prototype/shape.h"
#include "Adapter/mediaplayer.h"
#include "Bridge/shape.h"
#include "Filter/criteria.h"
#include "Composite/equipment.h"
#include "Decoraator/shape.h"
#include "Facade/shapemaker.h"
#include "Proxy/image.h"
#include "Chain/logger.h"
#include "Command/order.h"
#include "Interpreter/expression.h"
#include "Iterator/container.h"
#include "Mediator/user.h"
#include "Memento/memento.h"
#include "Observer/subject.h"
#include "State/context.h"
#include "State/tcpstate.h"
#include "NullObject/customer.h"
#include "Strategy/strategy.h"
#include "Template/game.h"
#include "Visitor/computer.h"
#include "Factory/maze.h"

int main(int argc, char** argv)
{
    // 工厂模式
    Factory::FactoryPatternDemo::test();

    // 抽象工厂模式
    AbstractFactory::AbstractFactoryPatternDemo::test();

    // 单例模式
    Singleton::SingletonPatternDemo::test();

    // 建造者模式
    Builder::BuilderPatternDemo::test();

    // 原型模式
    Prototype::PrototypePatternDemo::test();

    // 适配器模式
    Adapter::test();

    // 桥接模式
    Bridge::test();

    // 过滤器模式
    Filter::CriteriaPatternDemo::test();

    // 组合模式
    Composite::test();

    // 装饰者模式
    Decorator::test();

    // 外观模式
    Facade::test();

    // 代理模式
    Proxy::test();

    // 任务链模式
    Chain::test();

    // 命令模式
    Command::test();

    // 解释器模式
    Interpreter::test();

    // 迭代器模式
    Iterator::IteratorPatternDemo::test();

    // 中介模式
    Mediator::test();

    // 备忘录模式
    Memento::test();

    // 观察者模式
    Observer::test();

    // 状态模式
    State::StatePatternDemo::test();
    state::test();

    // 空对象模型
    NullObject::NullObjectPatternDemo::test();

    // 策略模式
    Strategy::test();

    // 模板模式
    Template::TemplatePatternDemo::test();

    // 访问者模式
    Visitor::test();

    maze::test();

#ifdef _WIN32
    system("pause");
#endif

    return 0;
}
