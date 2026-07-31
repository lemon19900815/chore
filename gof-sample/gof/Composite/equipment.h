#ifndef __EQUIPMENT_INC_H__
#define __EQUIPMENT_INC_H__

#include "../stdafx.h"

#include <list>
#include <sstream>
#include <memory>

namespace Composite {

class CompositeEquipment;
//class CompositeEquipment::Ptr;

class Equipment {
public:
    Equipment(std::string name) { name_ = name; }
    virtual ~Equipment() { }
    CLASS_PTR(Equipment);

    // 这个接口为了屏蔽Composite类的接口Add&Remove;减少公共类中不需要的接口
    virtual std::shared_ptr<CompositeEquipment> GetComposite() { return nullptr; }
    virtual std::string toString() { return name_; }

    // 其他公共类接口定义...

private:
    std::string name_;
    // other member...
};

class CompositeEquipment : public Equipment,
                  public std::enable_shared_from_this<CompositeEquipment> {
public:
    CompositeEquipment(std::string name);
    ~CompositeEquipment();
    CLASS_PTR(CompositeEquipment);

    void Add(Equipment::Ptr&& p);
    void Remove(Equipment::Ptr&& p);

    CompositeEquipment::Ptr GetComposite() override;
    std::string toString() override;

private:
    std::list<Equipment::Ptr> composites_;
};

class FloppyDisk : public Equipment {
public:
    FloppyDisk(std::string name) : Equipment(name) { }
};

void test();

}

#endif // __EQUIPMENT_INC_H__
