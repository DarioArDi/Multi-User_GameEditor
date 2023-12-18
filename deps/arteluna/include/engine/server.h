#ifndef __NETWORKING_SERVER_H__
#define __NETWORKING_SERVER_H__ 1
#include <cstdint>
#include <vector>

#include "enet/enet.h"

namespace std{
  class thread;
}

namespace al {
  class Server {
  public:
    Server();
    ~Server();

    void StartServer();
    
    void EndServer();

    void ConnectionProcess();

    void BroadCastChange();

    void ImguiMenu();
    
  private:
    void init();
    
    const char host_name[20] = "127.0.0.1";
    
    std::thread* connection_handle_thread_;
    ENetAddress address_;
    ENetHost* server_;
    std::vector<ENetPeer> clients_;
    int8_t max_clients_;
    uint32_t ticks_;
    bool initialized_;
    
    friend class Networking;
  };
}
#endif