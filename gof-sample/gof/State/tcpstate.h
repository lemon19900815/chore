#pragma once

#include "../stdafx.h"

namespace state {

class TcpContext;
class TcpState {
public:
  virtual ~TcpState() { }
  CLASS_PTR(TcpState);

  virtual void Enter(TcpContext& ctx) = 0;
  virtual void Update(TcpContext& ctx) = 0;
  virtual void Exit(TcpContext& ctx) = 0;
};

class TcpContext {
public:
  TcpContext() {
    frame_ = 0;
    tcp_state_ = nullptr;
  }

  TcpState::Ptr GetState() {
    return tcp_state_;
  }

  void SetState(TcpState::Ptr tcp_state) {
    tcp_state_ = tcp_state;
  }

  // 实现在tcp状态定义之后
  void Update();

private:
  int32_t frame_;
  TcpState::Ptr tcp_state_;
};

class TcpEstablished
  : public TcpState,
    public std::enable_shared_from_this<TcpEstablished> {
public:
  void Enter(TcpContext& ctx) override {
    LOG(__FUNCTION__);
    ctx.SetState(shared_from_this());
  }

  void Update(TcpContext& ctx) override {
  }

  void Exit(TcpContext& ctx) override {
    ctx.SetState(nullptr);
  }
};

class TcpRun
  : public TcpState,
    public std::enable_shared_from_this<TcpRun> {
public:
  void Enter(TcpContext& ctx) override {
    LOG(__FUNCTION__);
    ctx.SetState(shared_from_this());
  }

  void Update(TcpContext& ctx) override {
  }

  void Exit(TcpContext& ctx) override {
    ctx.SetState(nullptr);
  }
};

class TcpReconnect
  : public TcpState,
    public std::enable_shared_from_this<TcpReconnect> {
public:
  void Enter(TcpContext& ctx) override {
    LOG(__FUNCTION__);
    ctx.SetState(shared_from_this());
  }

  void Update(TcpContext& ctx) override {
  }

  void Exit(TcpContext& ctx) override {
    ctx.SetState(nullptr);
  }
};

class TcpClose
  : public TcpState,
    public std::enable_shared_from_this<TcpClose> {
public:
  void Enter(TcpContext& ctx) override {
    LOG(__FUNCTION__);
    ctx.SetState(shared_from_this());
  }

  void Update(TcpContext& ctx) override {
  }

  void Exit(TcpContext& ctx) override {
    ctx.SetState(nullptr);
  }
};

void TcpContext::Update() {
  int loop = 0;

  while (loop++ < 100) {
    ++frame_;

    if (tcp_state_) {
      if (frame_ % 100 != 0) {
        tcp_state_->Update(*this);
      }
      else {
        tcp_state_->Exit(*this);
      }
    }

    SleepMs(10);
  }
}

void test() {
  TcpContext ctx;

  auto s1 = std::make_shared<TcpEstablished>();
  s1->Enter(ctx);

  //ctx.Update();

  auto s2 = std::make_shared<TcpRun>();
  s2->Enter(ctx);

  //ctx.Update();

  auto s3 = std::make_shared<TcpReconnect>();
  s3->Enter(ctx);

  //ctx.Update();

  auto s4 = std::make_shared<TcpClose>();
  s4->Enter(ctx);

  //ctx.Update();
}

}
