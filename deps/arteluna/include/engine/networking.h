#ifndef __NETWORKING_H__
#define __NETWORKING_H__ 1
#include <vec3.hpp>

#include "client.h"
#include "server.h"
#include "components/light_component.h"
#include "components/transform_component.h"

namespace al{
  class Component;
}

namespace al {

  enum PackageType {
    kNone = -1,
    kUserCameraUpdate,
    kTransformComponentUpdate,
    kLightComponentUpdate
  };
  
  class Package {
  public:
    PackageType type() const;
    size_t size() const;
  protected:
    PackageType type_ = kNone;
    size_t size_ = 0;
  };

  class ComponentUpdate : public Package {
  public:
    ComponentUpdate();
    uint32_t entity_id_;
  };
  
  class TransformComponentUpdate : public ComponentUpdate {
  public:
    TransformComponentUpdate();
    TransformComponent component_;
  };

  class LightComponentUpdate : public ComponentUpdate {
  public:
    LightComponentUpdate();
    LightComponent component_;
  };

  class UserCameraUpdate : public Package {
  public:
    glm::vec3 position_;
    float rotate_x_;
    float rotate_y_;
    UserCameraUpdate();
  };
  
  class Networking {
  public:
    Networking(ServiceManager& sm,
      std::shared_ptr<al::Material>& user_mat,
      std::shared_ptr<al::Mesh>& user_shape);
    ~Networking();

    void ImguiMenu();

    void SendPackage(Package* package);
    
    
  private:
    ServiceManager* sm_;
    
    Server server_;
    Client client_;
    friend class Systems;
  };

 
  

  
}
#endif