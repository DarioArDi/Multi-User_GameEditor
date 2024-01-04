#include <cassert>

#include "engine/client.h"
#include <iostream>
#include <thread>

#include "entity.h"
#include "imgui.h"
#include "components/render_component.h"
#include "engine/entity_manager.h"
#include "engine/networking.h"

namespace al {

  Client::Client(ServiceManager& sm,
                 const std::shared_ptr<al::Material>& user_mat,
                 const std::shared_ptr<al::Mesh>& user_shape) {
    user_mat_ = user_mat;
    user_shape_ = user_shape;
    sm_ = &sm;
    initialized_ = false;
    server_ = nullptr;
    client_ = nullptr;
    sprintf(host_name,"127.0.0.1\0");
    connection_handle_thread_ = nullptr;
    /* Bind the server to the default localhost. */
    address_.host = ENET_HOST_ANY;
    /* Bind the server to port 1234. */
    address_.port = 1234;
    ticks_ = 0;
    server_entity_id_ = 0;
  }

  Client::~Client() {
    if (initialized_){
      enet_deinitialize();
      enet_host_destroy(client_);
    }
  }

  void Client::CreateClient() {
    init();
    client_ = enet_host_create( nullptr  /* create a client host */,
                                1     /* only allow 1 outgoing connection */,
                                2     /* allow up 2 channels to be used, 0 and 1 */,
                                0     /* assume any amount of incoming bandwidth */,
                                0     /* assume any amount of outgoing bandwidth */);
    if (client_ == nullptr) {
      printf ("An error occurred while trying to create an ENet client host.\n");fflush(stdout);
    }
  }

  bool Client::ClientCreated() {
    return initialized_;
  }

  void Client::Update() {
    ENetEvent event;
    printf ("Client :D\n");fflush(stdout);

    while (enet_host_service (client_, & event, 1000) >= 0) {
      switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT: {
          printf ("Server connected from %x:%u.\n",
                  event.peer->address.host,
                  event.peer->address.port);fflush(stdout);
          /* Store any relevant client information here. */
          break;
        }
        case ENET_EVENT_TYPE_RECEIVE: {
          Package* package = (Package*)event.packet->data;

          
          printf ("A packet of type %d was received from %s on channel %u.\n",
                  package->type(),
                  event.peer->data,
                  event.channelID);fflush(stdout);
          
          auto* em = sm_->Get<EntityManager>();
          
          switch (package->type()) {
            case kNone:
              break;
            case kUserCameraUpdate: {
              const auto* camera = (UserCameraUpdate*)package;
              auto* transform = em->GetEntity(server_entity_id_)
                                  ->get_component<TransformComponent>(*em);

              transform->set_position(camera->position_);
              transform->set_rotation(camera->rotate_x_,camera->rotate_y_,0.f);
              break;
            }
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
            default: ;
          }
          break;
        }
        case ENET_EVENT_TYPE_DISCONNECT: {
          printf ("%s disconnected.\n", event.peer->data);fflush(stdout);
          /* Reset the peer's client information. */
          event.peer -> data = nullptr;
          
          break;
        }
        case ENET_EVENT_TYPE_NONE: break;
      }
      ticks_++;

    }
    printf ("No Client D:\n");fflush(stdout);
  }

  void Client::SendPacket(ENetPacket* package) {
    /* Create a reliable packet */
    
     // Send the packet to the peer over channel id 0. 
    enet_peer_send (server_, 0, package);

    // Send packets to the host
    enet_host_flush (server_->host);
  }

  void Client::Connect() {
    ENetEvent event;
    /* Connect to some.server.net:1234. */
    enet_address_set_host (& address_, host_name);
    printf ("Server Info %d %d\n",address_.host,address_.port);fflush(stdout);

    /* Initiate the connection, allocating the two channels 0 and 1. */
    server_ = enet_host_connect (client_, & address_, 1, 0);
    if (server_ == nullptr){
      printf ("No available peers for initiating an ENet connection.\n");fflush(stdout);
      return;
    }
    /* Wait up to 5 seconds for the connection attempt to succeed. */
    if (enet_host_service (client_, & event, 5000) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT) {
      printf("Connection to %s:%d succeeded.",host_name,address_.port);
      fflush(stdout);
      connection_handle_thread_ = new std::thread(
        &Client::Update,
        this
      );
      
      auto* em = sm_->Get<EntityManager>();
      auto& entity = em->CreateNewEntity("Server");
      auto* render_cmp = entity.AddComponent<RenderComponent>(*em);
      render_cmp->mesh_ = user_shape_;
      render_cmp->material_ = user_mat_;
      server_entity_id_ = entity.id();
      
    } else {
      /* Either the 5 seconds are up or a disconnect event was */
      /* received. Reset the peer in the event the 5 seconds   */
      /* had run out without any significant event.            */
      enet_peer_reset (server_);
      server_ = nullptr;
      printf("Connection to %s:1234 failed.",host_name);fflush(stdout);
    }
  }

  void Client::Disconnect() {
    ENetEvent event;
    enet_peer_disconnect (server_, 0);
    /* Allow up to 3 seconds for the disconnect to succeed
     * and drop any packets received packets.
     */
    while (enet_host_service (client_, & event, 3000) > 0) {
      switch (event.type) {
        case ENET_EVENT_TYPE_RECEIVE:
          enet_packet_destroy (event.packet);
        break;
        case ENET_EVENT_TYPE_DISCONNECT:
          printf("Disconnection succeeded.");fflush(stdout);
        return;
        case ENET_EVENT_TYPE_NONE:
        case ENET_EVENT_TYPE_CONNECT:
          break;
      }
    }
    /* We've arrived here, so the disconnect attempt didn't */
    /* succeed yet.  Force the connection down.             */
    enet_peer_reset (server_);
  }

  void Client::init() {
    if(!initialized_){
      if (enet_initialize() < 0) {
        initialized_ = false;
        printf("An error occurred while initializing ENet.\n");fflush(stdout);
      } else{
        initialized_ = true;
      }
    }
  }

  void Client::ImguiMenu() {
    if (!ClientCreated()) {
      ImGui::InputText("Host name",host_name,100);
      if (ImGui::Button("Connect")){
        CreateClient();
        Connect();
      }
    } else {
      ImGui::Text("Tick %d",ticks_);
      ImGui::SameLine(); if (ImGui::Button("Disconnect")){
        Disconnect();
        CloseClient();
      }
    }
    
    ImVec4 color_green = ImVec4(0,1,0,1);
    ImVec4 color_red = ImVec4(1,0,0,1);
    ImGui::Begin("Network");
    ImGui::ColorEdit4("##3",
      server_ != nullptr ? (float*)&color_green : (float*)&color_red,
      ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
    
    ImGui::End();
  }

  void Client::CloseClient() {
    
    if (initialized_){
      initialized_ = false;
      enet_deinitialize();
      enet_host_destroy(client_);
    }
  }
}
