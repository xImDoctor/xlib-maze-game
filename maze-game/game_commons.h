#ifndef GAME_COMMONS_H
#define GAME_COMMONS_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// timers
#include <unistd.h> 
#include <time.h>

// threads and company xD
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>

#define CLIENT_WINDOW_OFFSET 200

#define LABYRITH_SIZE 10
#define MAX_PLAYER_COUNT 9
#define MAX_PLAYER_CHAR '0' + MAX_PLAYER_COUNT

#define SERVER_PORT 8080
#define MSG_BUFFER_SIZE 1024

// in seconds - 1 min now
#define GAME_RESTART_TIME 60


// map symbls
#define WALL '#'
#define PATH ' '
#define ENEMY 'E'
#define ESCAPE 'X'

// pauses in seconds between enemy move
#define ENEMY_INACTIVE_TIME 1


// working enums and structures

// enum for msgs between serv and client
typedef enum {
    MSG_CONNECT,
    MSG_MOVE,
    MSG_GAME_STATE,
    MSG_PLAYER_LOSE,
    MSG_DISCONNECT,
    MSG_RESTART_GAME    // when timer left
} msg_type_t;

typedef enum {DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT} direction_t;   // move dirs

typedef struct {int x, y; } position_t;                             // entities coords
typedef position_t coords;

//player struct
typedef struct {
    int id;
    coords pos;
    int isActive;       // 1 - active , 0 - lost
    int isConnected;
} player_t;


// struct that contains game logic and variables: maze and players info + mutex for threads
typedef struct {
    char maze[LABYRITH_SIZE][LABYRITH_SIZE];

    player_t players[MAX_PLAYER_COUNT];
    int inGamePlayerCount;

    int gameTimeLeft;

    position_t enemyPos;

    pthread_mutex_t gameMutex;     // mutex for ingame threads

} game_state_t;


// client-serv msg struct
typedef struct {

    msg_type_t type; 

    int playerID;
    direction_t direction;

    char gameData[LABYRITH_SIZE*LABYRITH_SIZE + MAX_PLAYER_COUNT * 20]; // serialized game state to msg

} msg_t;


// client thread for server
typedef struct {
    int socket;
    int playerID;

    game_state_t* game;

}  client_thread_data_t;


#endif