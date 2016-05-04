//
// Created by martin on 4/21/16.
//

#ifndef CLIENT_TEMPSERVER_H
#define CLIENT_TEMPSERVER_H
#include <SDL.h>
#include <SDL_net.h>
#include <stdbool.h>
#include <stdio.h>
#include "tools/linked_list.h"

#define MAX_PLAYER 8

typedef struct uniqueID{
    int id;
    bool used;
}uID;

void init_server();
void add_clients(TCPsocket tmpsocket, SDLNet_SocketSet *sockets,Dlist *socketList, int *playernum, uID *ID, char *tmp);
void check_data(TCPsocket tmpsocket, SDLNet_SocketSet *sockets,Dlist *socketList, int *playernum, uID *ID, char *tmp);
void check_DC(TCPsocket tmpsocket, SDLNet_SocketSet *sockets,Dlist *socketList, int *playernum, uID *ID, char *tmp);
static void on_exit(SDLNet_SocketSet *sockets, Dlist *socketList, TCPsocket *server, int *playernum);
static void remove_client(SDLNet_SocketSet *sockets,Dlist *socketList, uID *ID, int id, int *playernum);
#endif //CLIENT_TEMPSERVER_H

