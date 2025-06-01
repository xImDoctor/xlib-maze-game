#include "game_commons.h"


game_state_t gameState;                 // current in-game state
int clientSockets[MAX_PLAYER_COUNT];    // sockets of clients


// procedure generation of maze aka labyrinth
void generateMaze() {


}

// make serialized state that contains in msg_t 
void serializeGameState(char* buffer) {


}

// send game state to connected clients
void sendGameState() {

}


// check move possibility
int isMovePossible(coords newCoords) {

}


// move client players with threads ...
void movePlayer(int playerID, direction_t moveDir) {


}


// snake case for threads to mark them above all funcs
// enemy's thread
void* enemy_thread(void* args) {



}


// thread for player/client
void* client_thread(void* args) {



}



int main() {




    return 0;
}
