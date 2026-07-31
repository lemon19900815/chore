#ifndef __SESSION_MANAGER_INC_H__
#define __SESSION_MANAGER_INC_H__

#include "singleton.h"
#include "net/session.h"

#include <list>
#include <mutex>
#include <unordered_map>

typedef std::function<void(int32_t)> HandleProc;

class SessionManager
{
public:
  SessionManager();
  ~SessionManager();

  // 注册业务逻辑层回调
  void registerProc(HandleProc on_connection, HandleProc on_close);

  // bool active激活session；移除过期的session
  void update(bool active);

  // 新的连接
  void onAcceptSession(Session::PTR session);
  // 连接关闭
  void onCloseSession(Session::PTR session);

private:
  // 交换accept会话
  void swapAcceptSessions(std::list<Session::WPTR>& swap_accept_seesion);
  // 应用层激活会话
  void activeAcceptSessions(std::list<Session::WPTR>& swap_accept_seesion);

  // 交换close会话
  void swapCloseSessions(std::list<int>& swap_close_seesion);
  // 应用层关闭会话
  void detachCloseSessions(std::list<int>& swap_close_seesion);

  // tick有效的会话
  void tickActiveSessions();

  // 添加session到活动队列中
  bool addSession(Session::PTR session);
  // 从活动会话中移除session
  bool detachSession(int id);

private:
  std::unordered_map<int, Session::WPTR> active_sessions_;

  std::mutex accept_mutex_;
  std::unordered_map<int, Session::WPTR> accepted_sessions_;

  std::mutex close_mutex_;
  std::list<int> close_sessions_;

  // 业务逻辑层回调接口
  HandleProc on_connection_;
  HandleProc on_close_;
};

typedef util::singleton<SessionManager> gSessionManager;
#define gSessionManagerPtr gSessionManager::instance()

#endif // __SESSION_MANAGER_INC_H__
