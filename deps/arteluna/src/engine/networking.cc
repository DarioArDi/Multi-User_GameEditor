#include <cassert>

#include "engine/networking.h"
#include <iostream>
#include <thread>

#include "imgui.h"

namespace al {
  Networking::Networking() {
    initialized_ = false;
    peer_ = nullptr;
    server_ = nullptr;
    client_ = nullptr;
    max_clients_ = 32;
    sprintf(message,"Hola\0");fflush(stdout);
    
    /* Bind the server to the default localhost. */
    address_.host = ENET_HOST_ANY;
    /* Bind the server to port 1234. */
    address_.port = 1234;
    ticks_ = 0;
  }

  Networking::~Networking() {
    if (initialized_){
      enet_deinitialize();
      enet_host_destroy(server_);
      enet_host_destroy(client_);
    }
  }

  void Networking::StartServer() {
    init();

    server_ = enet_host_create (&address_     /* the address to bind the server host to */, 
                                max_clients_  /* allow up to 32 clients and/or outgoing connections */,
                                2             /* allow up to 2 channels to be used, 0 and 1 */,
                                0             /* assume any amount of incoming bandwidth */,
                                0             /* assume any amount of outgoing bandwidth */);
    if (server_) {
      connection_handle_thread_ = new std::thread(
        &Networking::ConnectionProcess,
        this
      );
    } else {
      printf ("An error occurred while trying to create an ENet server host.\n");fflush(stdout);
      return;
    }
    printf ("Server Info %d %d\n",address_.host,address_.port);fflush(stdout);
  }

  void Networking::EndServer() {
    enet_host_destroy(server_);
    server_ = nullptr;
  }

  void Networking::CreateClient() {
    init();
    client_ = enet_host_create( NULL  /* create a client host */,
                                1     /* only allow 1 outgoing connection */,
                                2     /* allow up 2 channels to be used, 0 and 1 */,
                                0     /* assume any amount of incoming bandwidth */,
                                0     /* assume any amount of outgoing bandwidth */);
    if (client_ == NULL) {
      printf ("An error occurred while trying to create an ENet client host.\n");fflush(stdout);
    }
  }

  void Networking::ConnectionProcess() {
    ENetEvent event;
    printf ("Server :D\n");fflush(stdout);

    while (enet_host_service (server_, & event, 1000) >= 0) {
      switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT: {
          printf ("A new client connected from %x:%u.\n",
                  event.peer->address.host,
                  event.peer->address.port);fflush(stdout);
          /* Store any relevant client information here. */
          event.peer->data = "Client information";
          break;
        }
        case ENET_EVENT_TYPE_RECEIVE: {
          printf ("A packet of length %llu containing %s was received from %s on channel %u.\n",
                  event.packet->dataLength,
                  event.packet->data,
                  event.peer->data,
                  event.channelID);fflush(stdout);
          /* Clean up the packet now that we're done using it. */
          enet_packet_destroy (event.packet);
          
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
    printf ("No Server D:\n");fflush(stdout);

  }

  void Networking::SendPacket() {
    /* Create a reliable packet */
    ENetPacket* packet = enet_packet_create(message, 
                                            sizeof(message) + 1,
                                            ENET_PACKET_FLAG_RELIABLE);
    
     // Send the packet to the peer over channel id 0. 
    enet_peer_send (peer_, 0, packet);

    // Send packets to the host
    enet_host_flush (peer_->host);
  }

  void Networking::Connect() {
    ENetEvent event;
    /* Connect to some.server.net:1234. */
    enet_address_set_host (& address_, host_name);
    printf ("Server Info %d %d\n",address_.host,address_.port);fflush(stdout);

    /* Initiate the connection, allocating the two channels 0 and 1. */
    peer_ = enet_host_connect (client_, & address_, 1, 0);
    if (peer_ == nullptr){
      printf ("No available peers for initiating an ENet connection.\n");fflush(stdout);
      return;
    }
    /* Wait up to 5 seconds for the connection attempt to succeed. */
    if (enet_host_service (client_, & event, 5000) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT) {
      printf("Connection to %s:%d succeeded.",host_name,address_.port);fflush
      (stdout);
    } else {
      /* Either the 5 seconds are up or a disconnect event was */
      /* received. Reset the peer in the event the 5 seconds   */
      /* had run out without any significant event.            */
      enet_peer_reset (peer_);
      printf("Connection to %s:1234 failed.",host_name);fflush(stdout);
    }
  }

  void Networking::Disconnect() {
    ENetEvent event;
    enet_peer_disconnect (peer_, 0);
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
    enet_peer_reset (peer_);
  }

  void Networking::init() {
    if(!initialized_){
      if (enet_initialize() < 0) {
        initialized_ = false;
        printf("An error occurred while initializing ENet.\n");fflush(stdout);
      } else{
        initialized_ = true;
      }
    }
  }

  void Networking::ImguiMenu() {
    ImVec4 color_green = ImVec4(0,1,0,1);
    ImVec4 color_red = ImVec4(1,0,0,1);
    ImGui::Begin("Network");
    ImGui::Text("Tick %d",ticks_);
    if (ImGui::Button("Create Server")){
      StartServer();
    }
    ImGui::SameLine();
    if (ImGui::Button("Destroy Server")){
      EndServer();
    }
    ImGui::ColorEdit4("##3",
      server_ != nullptr ? (float*)&color_green : (float*)&color_red,
      ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
      
    
    if (ImGui::Button("Connect")){
      CreateClient();
      Connect();
    }
    
    ImGui::SameLine(); if (ImGui::Button("Disconnect")){
      Disconnect();
    }
    ImGui::InputText("Message",message,255);

    ImGui::SameLine(); if (ImGui::Button("Send")){
      SendPacket();
    }
    
    ImGui::End();
  }
}
