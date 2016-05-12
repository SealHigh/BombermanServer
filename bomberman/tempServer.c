//
// Created by martin on 4/21/16.
//
#include "tempServer.h"



void init_server()
{

    SDL_Init(SDL_INIT_EVERYTHING);
    SDLNet_Init();

    uID ID[8];
    for(int i=0; i<MAX_PLAYER; i++)
    {
        ID[i].id = i;
        ID[i].used = false;
    }

    int map=0;
    int playernum = 0; // Amount of players on server
    SDL_Event event; //Used to exit server

    // Set IP address and that this is server
    IPaddress ip;
    if(SDLNet_ResolveHost(&ip,NULL,22222)==-1) //Resolve servers ip, "null" marks this as the server
    {
        printf("SDLNet_ResolveHost: %s\n",SDLNet_GetError());
        exit(3);
    }

    Dlist socketList;
    dlist_init(&socketList);

    int gameInProgress = 0;
    char tmp[1400];
    bool running = true;
    SDLNet_SocketSet sockets = SDLNet_AllocSocketSet(MAX_PLAYER); // So we can see if any data is sen from any client
    TCPsocket server = SDLNet_TCP_Open(&ip);

    while(running)
    {

        while(SDL_PollEvent(&event))
            if(event.type == SDL_QUIT)
                running = false;
        TCPsocket  tmpsocket = SDLNet_TCP_Accept(server);
        add_clients(tmpsocket, &sockets,&socketList,&playernum,ID, tmp, &map, gameInProgress);
        check_DC(tmpsocket, &sockets,&socketList,&playernum,ID, tmp);
        check_data(tmpsocket, &sockets,&socketList,&playernum,ID, tmp, &gameInProgress);

  }
    on_exit(&sockets, &socketList, &server, &playernum);

}
void add_clients(TCPsocket tmpsocket, SDLNet_SocketSet *sockets,Dlist *socketList, int *playernum, uID *ID, char *tmp, int *map, int gameInProgress)
{
    int curID=0;

    //Check if new connection
    if(tmpsocket)
    {
        if (*playernum < MAX_PLAYER && gameInProgress == 0)
        {

            SDLNet_TCP_AddSocket(*sockets, tmpsocket); //Adds player to list of connections

            for(int i = 0; i<MAX_PLAYER; i++)
            {
                if(ID[i].used == false){
                    printf("%d\n", ID[i].id);
                    ID[i].used = true;
                    curID = ID[i].id;
                    break;
                }

            }
            if(curID == 0){
                *map = rand() % 2;
            }
            dlist_insert_last(socketList,dlist_createElement(curID, tmpsocket, SDL_GetTicks())); // Adds new connection to our connection list

            printf("New connection: %d \n", curID);

            sprintf(tmp, "1 %d 1 1 %d \n", curID, *map);
            SDLNet_TCP_Send(tmpsocket,tmp,(int) strlen(tmp)+1); //Sends to the client that connected
            SDL_Delay(50); //Needed so the client that connected can init all other functions before it add all other players

            //Sends to all connected players except the player that sent the data
            //on client side this fills up the list of players
            for(int k=0; k<dlist_size(socketList); k++)
            {
                for (int i=0; i<dlist_size(socketList); i++)
                {
                    if(get_list_postition(socketList,i)->id == curID && get_list_postition(socketList,k)->socket == tmpsocket)
                    {
                    }
                    else{
                        sprintf(tmp, "1 %d 1 1 \n", get_list_postition(socketList, i)->id);
                        SDLNet_TCP_Send(get_list_postition(socketList, k)->socket, tmp, (int) strlen(tmp) + 1);
                        SDL_Delay(20);
                    }
                }

            }

            *playernum+=1;
        }else{
            sprintf(tmp, "4\n");

        }

    }
}

void check_data(TCPsocket tmpsocket, SDLNet_SocketSet *sockets,Dlist *socketList, int *playernum, uID *ID, char *tmp, int *gameInProgress)
{
    //check for incoming data
    while(SDLNet_CheckSockets(*sockets,0) > 0)
    {

        for(int i=0; i<dlist_size(socketList); i++ )
        {

            if(SDLNet_SocketReady(get_list_postition(socketList,i)->socket))
            {
                int type, id;
                get_list_postition(socketList,i)->timeout = SDL_GetTicks(); // Client is still connected
                SDLNet_TCP_Recv(get_list_postition(socketList,i)->socket, tmp, 1400);
                sscanf(tmp, "%d %d",&type, &id);

                //Check what kind of message client sent
                if(type==3) //Postition
                {
                    remove_client(sockets, socketList, ID, id, playernum);
                }

                if(type==2) //Postition
                {

                    for(int k=0; k<dlist_size(socketList); k++) //Sends to all connected players except the player that sent the data
                    {
                        if(k==i)
                            continue;

                        SDLNet_TCP_Send(get_list_postition(socketList,k)->socket,tmp,(int) strlen(tmp)+1);
                    }
                }

                if(type==4) //Bombs
                {
                    printf("bomb ahoy\n");
                    for(int k=0; k<dlist_size(socketList); k++) //Sends to all connected players except the player that sent the data
                    {
                            SDLNet_TCP_Send(get_list_postition(socketList, k)->socket, tmp, (int) strlen(tmp) + 1);
                    }
                }
                if(type==8) //Bombs
                {
                            printf("game going\n");
                    *gameInProgress = 1;
                }
            }
        }
    }
}







void check_DC(TCPsocket tmpsocket, SDLNet_SocketSet *sockets,Dlist *socketList, int *playernum, uID *ID, char *tmp){

    for(int j=0; j<*playernum; j++) // Handles dc'ed player, disconnect after 10 sec
    {
        int timeoutLimit;
        if (SDL_GetTicks() < 60000)
            timeoutLimit = SDL_GetTicks() - SDL_GetTicks() +100;
        else
            timeoutLimit = SDL_GetTicks() - 60000;
        if (socketList->element != NULL) {
            if (get_list_postition(socketList, j)->timeout < timeoutLimit) {
                int id = get_list_postition(socketList, j)->id;
                printf("DC: id %d ", id);
                printf("time %d \n", get_list_postition(socketList, j)->timeout);
                sprintf(tmp, "3 %d \n", get_list_postition(socketList, j)->id);

                remove_client(sockets, socketList, ID,id , playernum);

            }
        }
    }
}




static void remove_client(SDLNet_SocketSet *sockets,Dlist *socketList, uID *ID, int id, int *playernum){

    printf("removing player\n");
    char tmp[1400];

    int posInList = 0;
    for(int i= 0; i<*playernum; i++){
        if(get_list_postition(socketList, i)->id == id){
            posInList=i;
            break;
        }
    }

    // Send disconnect message to clients
    sprintf(tmp, "9 %d\n", id);
    for (int k = 0; k < dlist_size(socketList); k++){
        if (get_list_postition(socketList, k)->id ==id) // dont send to the client that left
            continue;
        printf("sending to %d",get_list_postition(socketList, k)->id);
        for(int i = 0; i<10; i++) {
            SDLNet_TCP_Send(get_list_postition(socketList, k)->socket, tmp, (int) strlen(tmp) + 1);
        }
    }

    //Delete all client info from server
    printf("pos in list to delete: %d\n", posInList);
    SDLNet_TCP_DelSocket(*sockets, get_list_postition(socketList, posInList)->socket);
    for(int i =  0; i<MAX_PLAYER; i++)
    {
        if(get_list_postition(socketList, posInList)->id == ID[i].id)
        {
            ID[i].used = false;
        }
    }
    dlist_removeElement(socketList, posInList);



    //Make data array linked list and delete socket from it here
    *playernum -= 1;
}




static void on_exit(SDLNet_SocketSet *sockets, Dlist *socketList, TCPsocket *server, int *playernum){
    for(int i=0; i<*playernum; i++)
        SDLNet_TCP_Close(get_list_postition(socketList,i)->socket);

    dlist_removeAllElements(socketList);
    SDLNet_FreeSocketSet(*sockets);
    SDLNet_TCP_Close(*server);
    SDLNet_Quit();
}

















































