#include <cassert>

#include "engine/networking.h"

#include<string>
#include <iostream>
namespace al {
  Networking::Networking() {
    initialized_ = false;
    server_ = nullptr;
    max_clients_ = 4;
  }

  Networking::~Networking() {
    if (initialized_){
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

  void Networking::ServerManager() {
    ENetEvent event;
    /* Wait up to 1000 milliseconds for an event. */
    while (enet_host_service (client_, & event, 1000) > 0) {
      switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT:
          printf ("A new client connected from %x:%u.\n", 
                  event.peer->address.host,
                  event.peer->address.port);
        /* Store any relevant client information here. */
          event.peer->data = "Client information";
        break;
        case ENET_EVENT_TYPE_RECEIVE:
          printf ("A packet of length %llu containing %s was received from %s on channel %u.\n",
                  event.packet->dataLength,
                  event.packet->data,
                  event.peer->data,
                  event.channelID);
        /* Clean up the packet now that we're done using it. */
        enet_packet_destroy (event.packet);
        
        break;
       
        case ENET_EVENT_TYPE_DISCONNECT:
          printf ("%s disconnected.\n", event.peer->data);
        /* Reset the peer's client information. */
        event.peer -> data = nullptr;

        case ENET_EVENT_TYPE_NONE:
          break;
      }
    }
  }

  void Networking::SendPacket() {
    /* Create a reliable packet */
    ENetPacket * packet = enet_packet_create ("packet", 
                                              strlen ("packet") + 1,
                                              ENET_PACKET_FLAG_RELIABLE);
    
    enet_packet_resize (packet, strlen ("packetfoo") + 1);
    strcpy((char*)&packet->data, "packetfoo");
    
     // Send the packet to the peer over channel id 0. 
    enet_peer_send (peer_, 0, packet);

    // Send packets to the host
    enet_host_flush (server_);
  }

  void Networking::Connect() {
    ENetEvent event;
    ENetPeer *peer;
    /* Connect to some.server.net:1234. */
    enet_address_set_host (& address_, "some.server.net");
    address_.port = 1234;
    /* Initiate the connection, allocating the two channels 0 and 1. */
    peer = enet_host_connect (client_, & address_, 2, 0);    
    if (peer == NULL){
      printf ("No available peers for initiating an ENet connection.\n");
      return;
    }
    /* Wait up to 5 seconds for the connection attempt to succeed. */
    if (enet_host_service (client_, & event, 5000) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT) {
      printf("Connection to some.server.net:1234 succeeded.");
    } else {
      /* Either the 5 seconds are up or a disconnect event was */
      /* received. Reset the peer in the event the 5 seconds   */
      /* had run out without any significant event.            */
      enet_peer_reset (peer);
      printf("Connection to some.server.net:1234 failed.");
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
          puts ("Disconnection succeeded.");
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
    if (enet_initialize() < 0) {
      initialized_ = true;
      printf("An error occurred while initializing ENet.\n");
    } else{
      enet_deinitialize();
      initialized_ = false;
    }
  }

  void Networking::ImguiMenu() {
    
  }
}
