#ifndef __NETWORKING_H__
#define __NETWORKING_H__ 1
#include <vec3.hpp>

#include "client.h"
#include "server.h"

namespace al{
  class Component;
}

namespace al {
  
  class Networking {
  public:
    Networking();
    ~Networking();

    void ImguiMenu();
    
  private:
    Server server_;
    Client client_;
    friend class Client;
    friend class Server;
  };

  enum PackageType {
    kNone = -1,
    kUserCameraUpdate,
    kComponentUpdate
  };
  
  class Package {
  public:
    PackageType type();
  protected:
    PackageType type_;
  };

  class ComponentUpdate : public Package {
  public:
    ComponentUpdate();
    Component* component_;
  };

  class UserCameraUpdate : public Package {
  public:
    glm::vec3 position_;
    glm::vec3 rotation_;
    UserCameraUpdate();
  };
}
#endif