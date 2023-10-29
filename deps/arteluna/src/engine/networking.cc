#include <cassert>

#include "engine/networking.h"

#include<string>
#include <iostream>
namespace al {
  Networking::Networking() {
    initialized = false;
    server_ = nullptr;
    max_clients_ = 4;
  }

  Networking::~Networking() {
    if (initialized){
      enet_deinitialize();
      enet_host_destroy(server_);
      enet_host_destroy(client_);
    }
  }

  void Networking::CreateServer() {
    /* Bind the server to the default localhost. */
    address_.host = ENET_HOST_ANY;
    /* Bind the server to port 1234. */
    address_.port = 1234;
    server_ = enet_host_create (&address_     /* the address to bind the server host to */, 
                                max_clients_  /* allow up to 32 clients and/or outgoing connections */,
                                2             /* allow up to 2 channels to be used, 0 and 1 */,
                                0             /* assume any amount of incoming bandwidth */,
                                0             /* assume any amount of outgoing bandwidth */);
    if (server_ == nullptr) {
      printf ("An error occurred while trying to create an ENet server host.\n");
    }
  }

  void Networking::CreateClient() {
    client_ = enet_host_create(NULL  /* create a client host */,
                              1     /* only allow 1 outgoing connection */,
                              2     /* allow up 2 channels to be used, 0 and 1 */,
                              0     /* assume any amount of incoming bandwidth */,
                              0     /* assume any amount of outgoing bandwidth */);
    if (client_ == NULL) {
      printf ("An error occurred while trying to create an ENet client host.\n");
    }
  }

  void Networking::init() {
    if (enet_initialize() < 0) {
      initialized = true;
      printf("An error occurred while initializing ENet.\n");
    } else{
      enet_deinitialize();
      initialized = false;
    }
  }

  void Networking::ImguiMenu() {
    
  }
}
