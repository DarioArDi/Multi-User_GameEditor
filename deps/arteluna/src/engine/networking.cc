#include <cassert>

#include "engine/networking.h"
#include "imgui.h"

namespace al {
  Networking::Networking(ServiceManager& sm,
      std::shared_ptr<al::Material>& user_mat,
      std::shared_ptr<al::Mesh>& user_shape)
  : server_(sm,user_mat,user_shape),
    client_(sm,user_mat,user_shape) {
    
    sm_ = &sm;
  }

  Networking::~Networking() {
    
  }

  void Networking::ImguiMenu() {

    ImGui::Begin("Network");

    if (!client_.ClientCreated()) {
      server_.ImguiMenu();
    }
    if (!server_.ServerStarted()) {
      client_.ImguiMenu();
    }
    ImGui::End();
  }

  void Networking::SendPackage(Package* package) {
    ENetPacket* packet = enet_packet_create(package, 
                                            package->size(),
                                            ENET_PACKET_FLAG_RELIABLE);
    if (client_.ClientCreated()) {
      client_.SendPacket(packet);
    } else if (server_.ServerStarted()){
      server_.BroadcastChange(packet,nullptr);
    }
  }

  PackageType Package::type() const {
    return type_;
  }

  size_t Package::size() const {
    return size_;
  }

  ComponentUpdate::ComponentUpdate() {
    entity_id_ = 0;
  }

  TransformComponentUpdate::TransformComponentUpdate() {
    type_ = kTransformComponentUpdate;
    size_ = sizeof(TransformComponentUpdate);
  }

  LightComponentUpdate::LightComponentUpdate() {
    type_ = kLightComponentUpdate;
    size_ = sizeof(LightComponentUpdate);
  }

  UserCameraUpdate::UserCameraUpdate() {
    type_ = kUserCameraUpdate;
    size_ = sizeof(UserCameraUpdate);
    position_ = glm::vec3();
    rotate_x_ = 0.f;
    rotate_y_ = 0.f;
  }
}
