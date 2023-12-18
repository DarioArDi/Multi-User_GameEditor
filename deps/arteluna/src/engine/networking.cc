#include <cassert>

#include "engine/networking.h"
#include "imgui.h"

namespace al {
  Networking::Networking() {
    
  }

  Networking::~Networking() {
  }

  void Networking::ImguiMenu() {
    ImVec4 color_green = ImVec4(0,1,0,1);
    ImVec4 color_red = ImVec4(1,0,0,1);
    ImGui::Begin("Network");
    ImGui::Text("Tick %d",server_.ticks_);
    if (ImGui::Button("Create Server")){
      server_.StartServer();
    }
    
    ImGui::SameLine(); if (ImGui::Button("Destroy Server")){
      server_.EndServer();
    }
    ImGui::SameLine(); ImGui::ColorEdit4("##3",
      server_.server_ != nullptr ? (float*)&color_green : (float*)&color_red,
      ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
    
    if (ImGui::Button("Connect")){
      client_.CreateClient();
      client_.Connect();
    }
    
    ImGui::SameLine(); if (ImGui::Button("Disconnect")){
      client_.Disconnect();
    }
    ImGui::SameLine(); ImGui::ColorEdit4("##3",
       client_.server_ != nullptr ? (float*)&color_green : (float*)&color_red,
       ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
    
    ImGui::SameLine(); if (ImGui::Button("Send")){
      client_.SendPacket();
    }
    
    ImGui::End();
  }

  ComponentUpdate::ComponentUpdate() {
    type_ = kComponentUpdate;
    component_ = nullptr;
  }

  UserCameraUpdate::UserCameraUpdate() {
    type_ = kUserCameraUpdate;
    position_ = glm::vec3();
    rotation_ = glm::vec3();
  }
}
