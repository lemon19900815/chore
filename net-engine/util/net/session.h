#ifndef __SESSION_INC_H__
#define __SESSION_INC_H__

class RingBuffer;

#define DEFAULT_RECV_BUFSIZE (64 * 1024)
#define DEFAULT_SEND_BUFSIZE (64 * 1024)

#include "networking.h"

#include <memory>
#include <cstring>
#include <iostream>
#include <functional>

class Session final
{
public:
  typedef std::shared_ptr<Session> PTR;
  typedef std::weak_ptr<Session> WPTR;

public:
  Session(int fd, char* ip, int port,
    int recvbufsize = DEFAULT_RECV_BUFSIZE,
    int sendbufsize = DEFAULT_SEND_BUFSIZE);

  ~Session();

  /*
  * close session
  */
  void onClose();
  /*
  * read socket, 0-success, -1 error[close session]
  */
  int onRead();
  /*
  * write socket, 0-success, -1 error[close session]
  */
  int onWrite();

  // 解析封包
  void dispatch();

public:
  /*
  * session handle file descriptor
  */
  inline int fd() const {
    return fd_;
  }

  /*
  * session unique description
  */
  inline std::string description() const {
    char desc[128];
    snprintf(desc, sizeof(desc), "%s:%d", ip_.c_str(), port_);
    return std::string(desc);
  }

private:
  // socket file descriptor
  int fd_;

  std::string ip_;
  int port_;

  std::shared_ptr<RingBuffer> recvbuf_;
  std::shared_ptr<RingBuffer> sendbuf_;
};

typedef std::function<void(Session::PTR session)> aeAccept;
typedef std::function<void(Session::PTR session)> aeClose;

#endif // __SESSION_INC_H__
