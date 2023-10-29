#ifndef __NETWORKING_H__
#define __NETWORKING_H__ 1
#include <cstdint>

#include "enet/enet.h"

namespace al {
  class Networking {
  public:
    Networking();
    ~Networking();

    void CreateServer();
    void CreateClient();
  private:

    void init();
    void ImguiMenu();
    ENetAddress address_;
    ENetHost* server_;
    ENetHost* client_;
    int8_t max_clients_;
    bool initialized;
  };
}
#endif