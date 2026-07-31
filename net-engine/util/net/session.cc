#include "session.h"

#include "timehelp.h"
#include "ringbuffer/ringbuffer.h"

#include <iostream>
#include <algorithm>

#define INVALID_ID -1

Session::Session(int fd, char* ip, int port,
  int recvbufsize, int sendbufsize)
{
  fd_ = fd;
  port_ = port;
  ip_ = ip;
  recvbuf_ = std::make_shared<RingBuffer>(recvbufsize);
  sendbuf_ = std::make_shared<RingBuffer>(sendbufsize);
}

Session::~Session()
{
  onClose();
}

void Session::onClose()
{
  if (fd_ != INVALID_ID) {
    network::anetClose(fd_);
    fd_ = INVALID_ID;
  }
}

int Session::onRead()
{
  int len;
  char buf[8192]; // 8k

  unsigned int availablebuflen = recvbuf_->available();
  if (availablebuflen == 0)
    return 0;

  len = network::anetRead2(fd_, buf, std::min<unsigned int>(sizeof(buf), availablebuflen));
  if (len < 0) return -1; // read on fd error

  recvbuf_->__kfifo_put(buf, len);

  return 0;
}

int Session::onWrite()
{
  int len;
  char buf[8192]; // 8k

  unsigned int used = sendbuf_->used();
  if (used == 0)
    return 0;

  if (used > sizeof(buf))
    used = sizeof(buf);

  sendbuf_->__peek(buf, used);

  len = network::anetWrite2(fd_, buf, used);
  if (len < 0) return -1; // write on fd error

  sendbuf_->__seekread(len);

  return 0;
}

// 解析封包
void Session::dispatch()
{
  char buff[8192];
  auto len = recvbuf_->used();

  if (len > sizeof(buff))
    len = sizeof(buff);

  len = recvbuf_->__kfifo_get(buff, len);
  recvbuf_->__seekread(len);

  buff[len - 1] = '\0';

  std::cout << buff << std::endl;
}
