#ifndef __NETWORKING_CLIENT_H__
#define __NETWORKING_CLIENT_H__ 1
#include <cstdint>
#include <memory>

#include "service_manager.h"
#include "enet/enet.h"

namespace std{
  class thread;
}

namespace al {
  class Material;
  class Mesh;

  class Client {
  public:
    Client(ServiceManager& sm,
           const std::shared_ptr<al::Material>& user_mat,
      const std::shared_ptr<al::Mesh>& user_shape);
    ~Client();

    void CreateClient();
    bool ClientCreated();

    void Update();

    void SendPacket(ENetPacket* package);

    void Connect();

    void Disconnect();
    
    void ImguiMenu();
    
    void CloseClient();

  private:
    void init();

    ServiceManager* sm_;
    std::shared_ptr<al::Material> user_mat_;
    std::shared_ptr<al::Mesh> user_shape_;
    char host_name[100];

    std::thread* connection_handle_thread_;
    ENetAddress address_;
    ENetPeer* server_;
    uint32_t server_entity_id_;
    ENetHost* client_;
    int8_t max_clients_;
    uint32_t ticks_;
    bool initialized_;
    friend class Networking;
  };
}
#endif