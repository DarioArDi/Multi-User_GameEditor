#ifndef __NETWORKING_CLIENT_H__
#define __NETWORKING_CLIENT_H__ 1
#include <cstdint>

#include "enet/enet.h"

namespace std{
  class thread;
}

namespace al {
  class Client {
  public:
    Client();
    ~Client();

    void CreateClient();

    void Update();

    void SendPacket();

    void Connect();

    void Disconnect();
    
    void ImguiMenu();

  private:
    void init();
    char message[255];
    
    char host_name[100] ;
    
    std::thread* connection_handle_thread_;
    ENetAddress address_;
    ENetPeer* server_;
    ENetHost* client_;
    int8_t max_clients_;
    uint32_t ticks_;
    bool initialized_;
    friend class Networking;
  };
}
#endif