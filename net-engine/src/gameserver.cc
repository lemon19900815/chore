#include "gameserver.h"

#include "timehelp.h"
#include "mainloop.h"
#include "net/netcore.h"
#include "net/session_manager.h"
#include "json/RapidJsonHelper.h"

util::Time gtime;
util::MainLoop gloop;

// 每帧时间定义
constexpr uint32_t kPerFrameTime = 20;

// 配置信息
util::JsonObj gconf;

RunApplication(GameServer);

GameServer::GameServer()
{
  stoped_ = true;
}

GameServer::~GameServer()
{
}

bool GameServer::startup(int argc, char* argv[])
{
  NOTUSED(argc);
  NOTUSED(argv);

  if (!gconf.ParseFile("../conf.json"))
  {
    std::cerr << "open conf file failed.";
    return false;
  }

  auto server_port = 0;
  if (!gconf.Get("ServerPort", server_port))
    return false;

  auto max_event_num = 0;
  if (!gconf.Get("MaxEventNum", max_event_num))
    return false;

  stoped_ = false;

  // 注册主循环调度方式和间隔
  gloop.registerSchema(
    kPerFrameTime, std::bind(&GameServer::schema, this));

  auto accept_proc = std::bind(&SessionManager::onAcceptSession, gSessionManagerPtr, std::placeholders::_1);
  auto close_proc = std::bind(&SessionManager::onCloseSession, gSessionManagerPtr, std::placeholders::_1);

  netcore_ = std::make_shared<NetCore>();
  if (!netcore_->initialize(max_event_num, accept_proc, close_proc))
  {
    std::cerr << "initialize netcore failed." << std::endl;
    return false;
  }

  return netcore_->startup(server_port);
}

void GameServer::run()
{
  while (!stoped_)
  {
    gtime.update();

    // 主循环
    gloop.update();

    // sleep for 10 ms
    ThreadHelper::sleep(10);
  }

  std::cout << "GameServer is closing..." << std::endl;
}

void GameServer::schema()
{
  bool active = false;
  if (gloop.frame_count() % 5 == 0)
    active = true;

  gSessionManagerPtr->update(active);
}

void GameServer::stop()
{
  stoped_ = true;
  netcore_->stop();
}
