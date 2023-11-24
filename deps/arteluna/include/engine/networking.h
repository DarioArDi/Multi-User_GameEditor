#ifndef __NETWORKING_H__
#define __NETWORKING_H__ 1
#include <cstdint>

#include "enet/enet.h"

namespace std{
  class thread;
}

namespace al {
  class Networking {
  public:
    Networking();
    ~Networking();

    void StartServer();
    
    void EndServer();

    void CreateClient();

    void ConnectionProcess();

    void SendPacket();

    void Connect();

    void Disconnect();
    
    void ImguiMenu();

  private:
    void init();
    char message[255];
    
    const char* host_name = "127.0.0.1";
    
    std::thread* connection_handle_thread_;
    ENetAddress address_;
    ENetHost* server_;
    ENetPeer* peer_;
    ENetHost* client_;
    int8_t max_clients_;
    uint32_t ticks_;
    bool initialized_;
  };
}
#endif