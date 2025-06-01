#include "game_commons.h"


game_state_t gameState;                 // current in-game state
int clientSockets[MAX_PLAYER_COUNT];    // sockets of clients


// procedure generation of maze aka labyrinth
void generateMaze() {


    for (int i = 0; i < LABYRITH_SIZE; ++i)
        for (int j=0; j < LABYRITH_SIZE; ++j)
            gameState.maze[i][j] = WALL;
    
    srand(time(NULL));

    int x = 0, y = 0;

    gameState.maze[y][x] = PATH;    // heights aka rows aka vert zxe - y, columns (horiz axe) - x


    // enlarge coords and make a path from 0,0 to maze final borders (exit)
    while (x < LABYRITH_SIZE-1 || y < LABYRITH_SIZE-1) {
        gameState.maze[y][x] = PATH;

        // if any of coords at field border - make path at other coord else just random way (not milky)
        if (x == LABYRITH_SIZE-1)
            ++y;
        else if (y == LABYRITH_SIZE - 1)
            ++x;
        else {
            if (rand()%2)
                ++x;
            else
                ++y;
        }

    }
    gameState.maze[LABYRITH_SIZE-1][LABYRITH_SIZE-1] = ESCAPE;

    // random pathes to make labyrinth looks like labyrinth yeaaah
    for (int i = 0; i < LABYRITH_SIZE*2; ++i) {
        
        int randomX = rand() % LABYRITH_SIZE;
        int randomY = rand() % LABYRITH_SIZE;

        if (gameState.maze[randomY][randomX] != ESCAPE)
            gameState.maze[randomY][randomX] = PATH;        // don't check if path coz it doesn't matter when we add pathes

    }

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
