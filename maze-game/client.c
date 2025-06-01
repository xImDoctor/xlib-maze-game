#include "game_commons.h"


#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

// data for ingame graphics making: display, window and context + base sizes
typedef struct {
    Display* display;
    Window window;
    GC gc;

    int winWidth, winHeight;
    int ingameCellSize;

} ingame_graphics_t;
typedef ingame_graphics_t graphic_t;


typedef struct {
    int socket;
    int playerID;
    int isActive;

    char gameField[LABYRITH_SIZE*LABYRITH_SIZE+1]; // + 1 to \0

    pthread_mutex_t fieldMutex;
    graphic_t graphics;

} client_state_t;

client_state_t clientState;     // current client state data


int init() {

}

int drawGame() {


}


// thread to loot our pretty server)
void* network_thread(void* args) {


}

// send player move to server
void sendMove(direction_t moveDir) {

}

// process move keys using (WASD and just arrows)
// idk for now press or release so its just using
void processKeyEvent(XKeyEvent* event) {


}


int main(int argc, char* argv[]) {




    return 0;
}