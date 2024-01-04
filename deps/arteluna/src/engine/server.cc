#include <cassert>

#include "engine/server.h"
#include <iostream>
#include <thread>

#include "entity.h"
#include "imgui.h"
#include "engine/entity_manager.h"
#include "engine/mesh.h"
#include "engine/material.h"
#include "engine/networking.h"

namespace al {
  Server::Server(ServiceManager& sm, std::shared_ptr<al::Material>& user_mat,
    std::shared_ptr<al::Mesh>& user_shape) {
    sm_ = &sm;
    user_mat_ = user_mat;
    
    user_shape_ = user_shape;
    
    initialized_ = false;
    server_ = nullptr;
    max_clients_ = 32;
    sprintf(host_name_,"172.0.0.1");
    connection_handle_thread_ = nullptr;
    
    /* Bind the server to the default localhost. */
    address_.host = ENET_HOST_ANY;
    /* Bind the server to port 1234. */
    address_.port = 1234;
    ticks_ = 0;
  }

  Server::~Server() {
    if (initialized_){
      enet_deinitialize();
      enet_host_destroy(server_);
    }
  }

  void Server::StartServer() {
    init();

    server_ = enet_host_create (&address_     /* the address to bind the server host to */, 
                                max_clients_  /* allow up to 32 clients and/or outgoing connections */,
                                2             /* allow up to 2 channels to be used, 0 and 1 */,
                                0             /* assume any amount of incoming bandwidth */,
                                0             /* assume any amount of outgoing bandwidth */
    );
    
    if (server_) {
      connection_handle_thread_ = new std::thread(
        &Server::ConnectionProcess,
        this
      );
      
    } else {
      printf ("An error occurred while trying to create an ENet server host.\n");fflush(stdout);
      return;
    }
    printf ("Server Info %d %d\n",address_.host,address_.port);fflush(stdout);
  }

  bool Server::ServerStarted() const {
    return initialized_;
  }

  void Server::EndServer() {
    enet_host_destroy(server_);
    enet_deinitialize();
    initialized_ = false;
    ticks_ = 0;
    server_ = nullptr;
  }

  void Server::ConnectionProcess() {
    ENetEvent event;
    printf ("Server :D\n");fflush(stdout);

    while (enet_host_service (server_, & event, 1000) >= 0) {

      switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT: {
          char user_name[30] = {'\0'};
          printf ("A new client connected from %x:%u.\n",
                  event.peer->address.host,
                  event.peer->address.port);fflush(stdout);
          
          sprintf(user_name,"User %d",next_user_id);
          event.peer->data = new int;
          *(int*)event.peer->data = next_user_id;
          auto* em = sm_->Get<EntityManager>();
          auto& entity = em->CreateNewEntity(user_name);
          auto* render_cmp = entity.AddComponent<RenderComponent>(*em);
          render_cmp->mesh_ = user_shape_;
          render_cmp->material_ = user_mat_;
          
          clients_.emplace_back(event.peer,entity.id());
          next_user_id++;
          break;
        }
        case ENET_EVENT_TYPE_RECEIVE: {
          Package* package = (Package*)event.packet->data;

          
          printf ("A packet of type %d was received from %s on channel %u.\n",
                  package->type(),
                  event.peer->data,
                  event.channelID);fflush(stdout);
          
          auto* em = sm_->Get<EntityManager>();
          std::pair<_ENetPeer*,size_t>* client = nullptr;
          for (auto& element : clients_) {
            if (element.first->connectID == event.peer->connectID) {
              client = &element;
              break;
            }
            
          }
          if (client != nullptr) {
            switch (package->type()) {
              case kNone:
                break;
              case kUserCameraUpdate: {
                const auto* camera = (UserCameraUpdate*)package;
                auto* transform = em->GetEntity(client->second)
                                    ->get_component<TransformComponent>(*em);

                transform->set_position(camera->position_);
                transform->set_rotation(camera->rotate_x_,camera->rotate_y_,0.f);
                
              }
                break;
              case kTransformComponentUpdate: {
                const auto* transform = (TransformComponentUpdate*)package;
                Entity* entity = em->GetEntity(transform->entity_id_); 
                auto* component = entity->get_component<TransformComponent>(*em);
                *component = transform->component_;
                
                break;
                
              }
              case kLightComponentUpdate: {
                const auto* light = (LightComponentUpdate*)package;
                Entity* entity = em->GetEntity(light->entity_id_); 
                auto* component = entity->get_component<LightComponent>(*em);
                *component = light->component_;
                break;
                
              }
              default: break;
            }
          }

          
          /* Clean up the packet now that we're done using it. */
          enet_packet_destroy (event.packet);
          
          break;
        }
        case ENET_EVENT_TYPE_DISCONNECT: {
          printf ("%s disconnected.\n", event.peer->data);fflush(stdout);
          /* Reset the peer's client information. */
          delete (int*)event.peer -> data;
          event.peer -> data = nullptr;
          
          break;
        }
        case ENET_EVENT_TYPE_NONE: break;
      }
      ticks_++;

    }
    printf ("No Server D:\n");fflush(stdout);

  }

  void Server::BroadcastChange(ENetPacket* package, ENetPeer* origin) {
    for (size_t i = 0; i < clients_.size(); ++i) {
      
      if (origin == nullptr || origin->connectID !=  clients_.at(i).first->connectID) {
        enet_peer_send(clients_.at(i).first, 0, package);
        // Send packets to the host
        enet_host_flush (clients_.at(i).first->host);
        
      }
    }
  }

  void Server::ImguiMenu() {
    if (!ServerStarted()) {
      ImGui::InputText("IP",host_name_,100);
      ImGui::InputScalar("Port",ImGuiDataType_S16,&address_.port,nullptr);

      if (ImGui::Button("Create Server")){
        StartServer();
      }
      
      ImVec4 color_green = ImVec4(0,1,0,1);
      ImVec4 color_red = ImVec4(1,0,0,1);
      ImGui::SameLine(); ImGui::ColorEdit4("##3",
        server_ != nullptr ? (float*)&color_green : (float*)&color_red,
        ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
    } else {
      ImGui::Text("Tick %d",ticks_);
      ImGui::SameLine(); if (ImGui::Button("Destroy Server")){
        EndServer();
        CloseServer();
      }
    }
  }

  void Server::CloseServer() {
    
  }

  void Server::init() {
    if(!initialized_){
      if (enet_initialize() < 0) {
        initialized_ = false;
        printf("An error occurred while initializing ENet.\n");fflush(stdout);
      } else{
        initialized_ = true;
      }
    }
  }

}
