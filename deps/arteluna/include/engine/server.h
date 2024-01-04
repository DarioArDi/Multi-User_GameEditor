#ifndef __NETWORKING_SERVER_H__
#define __NETWORKING_SERVER_H__ 1
#include <cstdint>
#include <memory>
#include <vector>

#include "service_manager.h"
#include "enet/enet.h"

namespace std{
  class thread;
}

namespace al {
  class Material;
  class Mesh;

  class Server {
  public:
    Server() = delete;
    Server(ServiceManager& sm,
      std::shared_ptr<al::Material>& user_mat,
      std::shared_ptr<al::Mesh>& user_shape
    );
    ~Server();

    void StartServer();
    bool ServerStarted() const;
    
    void EndServer();

    void ConnectionProcess();

    // When the server receives a package, send it to the rest of the clients
    void BroadcastChange(ENetPacket* package, ENetPeer* origin);

    void ImguiMenu();
    
    // void CloseServer();

  private:
    void init();
    ServiceManager* sm_;
    
    char host_name_[100];
    std::shared_ptr<al::Material> user_mat_;
    std::shared_ptr<al::Mesh> user_shape_;
    
    std::thread* connection_handle_thread_;
    
    ENetAddress address_;
    
    ENetHost* server_;
    
    std::vector<std::pair<_ENetPeer*,size_t>> clients_;
    
    int8_t max_clients_;
    uint32_t ticks_;
    bool initialized_;
    int next_user_id = 0;
    friend class Networking;
    friend class Systems;
  };
}
#endif