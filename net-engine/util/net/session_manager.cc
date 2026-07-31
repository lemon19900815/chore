#include "session_manager.h"

SessionManager::SessionManager()
{
}

SessionManager::~SessionManager()
{
}

// 注册业务逻辑层回调
void SessionManager::registerProc(HandleProc on_connection, HandleProc on_close)
{
  on_connection_ = on_connection;
  on_close_ = on_close;
}

// 激活session；移除过期的session
void SessionManager::update(bool active)
{
  if (active)
  {
    std::list<Session::WPTR> swap_accepted_sessions;
    swapAcceptSessions(swap_accepted_sessions);

    activeAcceptSessions(swap_accepted_sessions);

    std::list<int> swap_close_sessions;
    swapCloseSessions(swap_close_sessions);

    detachCloseSessions(swap_close_sessions);
  }

  tickActiveSessions();
}

// tick有效的会话
void SessionManager::tickActiveSessions()
{
  std::list<int> need_close_sessions;

  for (auto& info : active_sessions_)
  {
    auto session = info.second.lock();
    if (session)
      session->dispatch();
    else
      need_close_sessions.push_back(info.first);
  }

  detachCloseSessions(need_close_sessions);
}

// 新的连接
void SessionManager::onAcceptSession(Session::PTR session)
{
  std::lock_guard<std::mutex> guard(accept_mutex_);
  accepted_sessions_[session->fd()] = session;
}

// 连接关闭
void SessionManager::onCloseSession(Session::PTR session)
{
  std::lock_guard<std::mutex> guard(close_mutex_);
  close_sessions_.push_back(session->fd());
}

// 交换accept会话
void SessionManager::swapAcceptSessions(std::list<Session::WPTR>& swap_accept_seesion)
{
  std::lock_guard<std::mutex> guard(accept_mutex_);

  for (auto& info : accepted_sessions_)
  {
    swap_accept_seesion.push_back(info.second);
  }

  accepted_sessions_.clear();
}

// 应用层激活会话
void SessionManager::activeAcceptSessions(std::list<Session::WPTR>& swap_accept_seesion)
{
  for (auto& info : swap_accept_seesion)
  {
    auto session = info.lock();
    if (session)
      addSession(session);
  }
}

// 交换close会话
void SessionManager::swapCloseSessions(std::list<int>& swap_close_seesion)
{
  std::lock_guard<std::mutex> guard(close_mutex_);
  swap_close_seesion = std::move(close_sessions_);
}

// 应用层关闭会话
void SessionManager::detachCloseSessions(std::list<int>& swap_close_seesion)
{
  for (auto& id : swap_close_seesion)
  {
    detachSession(id);
  }
}

// 添加session到活动队列中
bool SessionManager::addSession(Session::PTR session)
{
  auto ret = active_sessions_.insert(std::make_pair(session->fd(), session));
  return ret.second;
}

// 从活动会话中移除session
bool SessionManager::detachSession(int id)
{
  return active_sessions_.erase(id) > 0;
}
