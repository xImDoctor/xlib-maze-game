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


// serialize game state to send it then
void serializeGameState(char* buffer) {

    int offset = 0;

    for (int i = 0; i < LABYRITH_SIZE; ++i)
        for (int j = 0; j < LABYRITH_SIZE; ++j) {
            
            int playerHere = -1;  // check if player in this pos and return his ID
            
            // parse in search of player pos
            for (int p = 0; p < gameState.inGamePlayerCount; ++p) 
                if (gameState.players[p].isConnected && gameState.players[p].pos.y == i  && gameState.players[p].pos.x == j) 
                    playerHere = gameState.players[p].id;
                    break;
                
            if (gameState.enemyPos.y == i && gameState.enemyPos.x == j) 
                buffer[offset++] = ENEMY;
            else if (playerHere >= 0)
                buffer[offset++] = '0' + playerHere;
            else
                buffer[offset++] = gameState.maze[i][j];
        }
    buffer[offset] = '\0';

}



// send game state to connected clients
void sendGameState() {
    msg_t msg;

    msg.type = MSG_GAME_STATE;
    serializeGameState(msg.gameData);

    for (int i = 0; i < gameState.inGamePlayerCount; ++i)
        if (gameState.players[i].isConnected)
            send(clientSockets[i], &msg, sizeof(msg), 0);   // send using network
}


// check move possibility
int isMovePossible(coords newCoords) {

    // check borders
    if (newCoords.x < 0 || newCoords.x >= LABYRITH_SIZE || newCoords.y < 0 || newCoords.y > LABYRITH_SIZE)
        return 0;

    // check path/not
    return gameState.maze[newCoords.y][newCoords.x] != WALL;
}


// move client players with threads ...
void movePlayer(int playerID, direction_t moveDir) {

    pthread_mutex_lock(&gameState.gameMutex);       // try to bring mutex, if its free - do moving, else - freeze thread and wait

    player_t* player = &gameState.players[playerID];    // player address in ingame players


    if (!player->isActive) {    // if player not active - free mutex
        pthread_mutex_unlock(&gameState.gameMutex);
        return;
    }

    coords newPos = {player->pos.x, player->pos.y}; // save current pos to change it

    switch (moveDir)
    {
        case DIR_UP:    --newPos.y; break;
        case DIR_DOWN:  ++newPos.y; break;
        case DIR_LEFT:  --newPos.x; break;
        case DIR_RIGHT: ++newPos.x; break;
    }

    if (isMovePossible(newPos)) {

        player->pos = newPos;

        if (gameState.maze[newPos.y][newPos.x] == ESCAPE)
            printf("Игрок %d добрался до выхода!\n", player->id);
    }

    pthread_mutex_unlock(&gameState.gameMutex);
}


// snake case for threads to mark them above all funcs
// enemy's thread
void* enemy_thread(void* args) {

    while(1) {
        sleep(ENEMY_INACTIVE_TIME); //s

        pthread_mutex_lock(&gameState.gameMutex);

        direction_t dir = rand() % 4;
        
        coords oldPos = gameState.enemyPos;
        coords newPos = oldPos;

        switch (dir)
        {
            case DIR_UP:    --newPos.y; break;
            case DIR_DOWN:  ++newPos.y; break;
            case DIR_LEFT:  --newPos.x; break;
            case DIR_RIGHT: ++newPos.x; break;
        }

        // when enemy in-bounds invert pathes and move him
        if (newPos.x >= 0 && newPos.x < LABYRITH_SIZE && newPos.y >= 0 && newPos.y < LABYRITH_SIZE) {

            // invert logic
            if (gameState.maze[oldPos.y][oldPos.x] == WALL)
                gameState.maze[oldPos.y][oldPos.x] = PATH;
            else if (gameState.maze[oldPos.y][oldPos.x] == PATH)
                gameState.maze[oldPos.y][oldPos.x] = WALL;
            
            gameState.enemyPos = newPos;
            
            // check collision with player
            for (int i = 0; i < gameState.inGamePlayerCount; ++i) 
                if (gameState.players[i].isActive && gameState.players[i].pos.x == newPos.x && gameState.players[i].pos.y == newPos.y) {
                    
                    gameState.players[i].isActive = 0;          // make him inactive (game over)
                    printf("Игрок %d пойман вражиной басурманской!\n", i);
                    
                    // send player that he looser
                    msg_t msg;
                    msg.type = MSG_PLAYER_LOSE;
                    msg.playerID = i;
                    send(clientSockets[i], &msg, sizeof(msg), 0);
                } 
        }
        
        pthread_mutex_unlock(&gameState.gameMutex);
        sendGameState();    // send actual game state (pathes, enemy pos, player dead/alive)
    }

    return NULL;
}


// thread for player/client
void* client_thread(void* args) {

    client_thread_data_t* data = (client_thread_data_t*)args;
    msg_t msg;

    // loop to recive msgs
    while(1) {
        int bytes = recv(data->socket, &msg, sizeof(msg), 0);

        if (bytes <= 0)
            break;
        
        switch(msg.type) {

            case MSG_MOVE:
                movePlayer(data->playerID, msg.direction);
                sendGameState();
                break;
            case MSG_DISCONNECT:
                gameState.players[data->playerID].isActive = 0;
                gameState.players[data->playerID].isConnected = 0;
                break;
        }
    }

    // if client disconnected
    pthread_mutex_lock(&gameState.gameMutex);
    gameState.players[data->playerID].isConnected = 0;
    pthread_mutex_unlock(&gameState.gameMutex);

    close(data->socket);
    free(data);
    return NULL;
}



int main() {




    return 0;
}
