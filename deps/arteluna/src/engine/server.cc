#include <cassert>

#include "engine/server.h"
#include <iostream>
#include <thread>

#include "imgui.h"

namespace al {
  Server::Server() {
    initialized_ = false;
    server_ = nullptr;
    max_clients_ = 32;
    
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
                                0             /* assume any amount of outgoing bandwidth */);
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

  void Server::EndServer() {
    enet_host_destroy(server_);
    server_ = nullptr;
  }
  void Server::ConnectionProcess() {
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

  void Server::BroadCastChange() {
    
    /* Create a reliable packet */
    
    ENetPacket* packet = enet_packet_create(message, 
                                            sizeof(message) + 1,
                                            ENET_PACKET_FLAG_RELIABLE);

    for (int i = 0; i < clients_.size(); ++i){
       // Send the packet to the peer over channel id 0. 
      // enet_peer_send (clients_.at(i), 0, packet);

      // Send packets to the host
      enet_host_flush (clients_.at(i).host);
    }
  }

  void Server::ImguiMenu() {
    
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
