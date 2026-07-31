#ifndef __Command_Order_Inc_H__
#define __Command_Order_Inc_H__

#include "../stdafx.h"
#include <list>

namespace Command {

class Stock {
  public:
  Stock(std::string name, int amount) {
    name_ = name;
    amount_ = amount;
  }
  ~Stock() { }
  CLASS_PTR(Stock);

  void buy() {
    std::cout << "buy stock[name: " << name_
      << ", amount: " << amount_ << "]" << std::endl;
  }

  void sell() {
    std::cout << "sell stock[name: " << name_
      << ", amount: " << amount_ << "]" << std::endl;
  }

private:
  std::string name_;
  int32_t amount_;
};

// command接口
class Order {
public:
  virtual ~Order() { }
  CLASS_PTR(Order);
  virtual void execute() = 0;
  };

class BuyStock : public Order {
public:
  BuyStock(Stock::Ptr stock) {
    stock_ = stock;
  }

  void execute() {
    stock_->buy();
  }

private:
    Stock::Ptr stock_;
};

class SellStock : public Order {
public:
  SellStock(Stock::Ptr stock) {
    stock_ = stock;
  }

  void execute() {
    stock_->sell();
  }

private:
  Stock::Ptr stock_;
};

class Broker {
public:
  Broker() { }

  ~Broker() {
    orderList_.clear();
  }

  void takeOrder(Order::Ptr order) {
    orderList_.emplace_back(order);
  }

  void placeOrders() {
    for (auto& it : orderList_) {
      it->execute();
    }
  }

private:
  std::list<Order::Ptr> orderList_;
};

void test() {
  std::cout << "\n\n command pattern." << std::endl;

  Stock::Ptr qq = std::make_shared<Stock>("qq", 100);
  Stock::Ptr alibaba = std::make_shared<Stock>("alibaba", 400);

  auto broker = std::make_shared<Broker>();
  broker->takeOrder(std::make_shared<BuyStock>(qq));
  broker->takeOrder(std::make_shared<SellStock>(qq));

  broker->takeOrder(std::make_shared<BuyStock>(alibaba));
  broker->takeOrder(std::make_shared<SellStock>(alibaba));

  broker->placeOrders();
}

}

#endif //  __Command_Order_Inc_H__
