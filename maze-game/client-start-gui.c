#include "game_commons.h"

// include my gui lib: button & field
#include "../gui-lib/xbutton.h"
#include "../gui-lib/xtextfield.h"
// alias for gui structs
typedef Button _bt;
typedef TextField _tf;

#include "client_ui.h"  // funcs and structs of app to use gui lib


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

    int gameTimeLeft; // game timer from server

    char gameField[LABYRITH_SIZE*LABYRITH_SIZE+1]; // + 1 to \0

    pthread_mutex_t fieldMutex;
    graphic_t graphics;

} client_state_t;

client_state_t clientState;     // current client state data


// init player (client) window, and set base client state and graphics fields
int init() {
    clientState.graphics.display = XOpenDisplay(NULL);
    if (!clientState.graphics.display) {
        fprintf(stderr, "Не удалось открыть дисплей\n");
        return 0;
    }
    
    int screen = DefaultScreen(clientState.graphics.display);
    clientState.graphics.ingameCellSize = 40;
    clientState.graphics.winWidth = LABYRITH_SIZE * clientState.graphics.ingameCellSize + CLIENT_WINDOW_OFFSET;
    clientState.graphics.winHeight = LABYRITH_SIZE * clientState.graphics.ingameCellSize + CLIENT_WINDOW_OFFSET;
    
    clientState.graphics.window = XCreateSimpleWindow(
        clientState.graphics.display,
        RootWindow(clientState.graphics.display, screen),
        100, 100,
        clientState.graphics.winWidth, clientState.graphics.winHeight,
        1,
        BlackPixel(clientState.graphics.display, screen),
        WhitePixel(clientState.graphics.display, screen)
    );
    
    XSelectInput(clientState.graphics.display, clientState.graphics.window, ExposureMask | KeyPressMask | StructureNotifyMask);
    XMapWindow(clientState.graphics.display, clientState.graphics.window);
    
    clientState.graphics.gc = XCreateGC(clientState.graphics.display, clientState.graphics.window, 0, NULL);
    
    XStoreName(clientState.graphics.display, clientState.graphics.window, "Maze Game Client");
    
    return 1;
}


// draw graphics on player window
int drawGame() {

    pthread_mutex_lock(&clientState.fieldMutex);

    XClearWindow(clientState.graphics.display, clientState.graphics.window);

    for (int i = 0; i < LABYRITH_SIZE; ++i)
        for (int j = 0; j < LABYRITH_SIZE; ++j) {

            coords cellStart;   // start top-left corner coords

            // current cell destination (start coords)
            cellStart.x = j * clientState.graphics.ingameCellSize;
            cellStart.y = i * clientState.graphics.ingameCellSize;

            char cell = clientState.gameField[i * LABYRITH_SIZE + j];
            
            switch (cell) {

                case WALL:  // draw wall as black rectangle

                    XSetForeground(clientState.graphics.display, clientState.graphics.gc, BlackPixel(clientState.graphics.display, 0));
                    XFillRectangle(clientState.graphics.display, clientState.graphics.window,
                                  clientState.graphics.gc, cellStart.x, cellStart.y, clientState.graphics.ingameCellSize, clientState.graphics.ingameCellSize);
                    break;
                    

                case PATH:  // white rectangle with borders (frame)

                    XSetForeground(clientState.graphics.display, clientState.graphics.gc, WhitePixel(clientState.graphics.display, 0));
                    XFillRectangle(clientState.graphics.display, clientState.graphics.window, clientState.graphics.gc, cellStart.x, cellStart.y, 
                                  clientState.graphics.ingameCellSize, clientState.graphics.ingameCellSize);

                    XSetForeground(clientState.graphics.display, clientState.graphics.gc, BlackPixel(clientState.graphics.display, 0));
                    XDrawRectangle(clientState.graphics.display, clientState.graphics.window, clientState.graphics.gc, cellStart.x, cellStart.y, 
                                  clientState.graphics.ingameCellSize, clientState.graphics.ingameCellSize);
                    break;
                    

                case ESCAPE: // escape - green rectangle

                    XSetForeground(clientState.graphics.display, clientState.graphics.gc, 0x00FF00);
                    XFillRectangle(clientState.graphics.display, clientState.graphics.window,
                                  clientState.graphics.gc, cellStart.x, cellStart.y,
                                  clientState.graphics.ingameCellSize, clientState.graphics.ingameCellSize);
                    break;
                    

                case ENEMY: // enemy - red circle

                    XSetForeground(clientState.graphics.display, clientState.graphics.gc, WhitePixel(clientState.graphics.display, 0));
                    XFillRectangle(clientState.graphics.display, clientState.graphics.window, clientState.graphics.gc, cellStart.x, cellStart.y,
                                  clientState.graphics.ingameCellSize, clientState.graphics.ingameCellSize);

                    XSetForeground(clientState.graphics.display, clientState.graphics.gc, 0xFF0000);
                    XFillArc(clientState.graphics.display, clientState.graphics.window, clientState.graphics.gc, cellStart.x + 5, cellStart.y + 5, 
                            clientState.graphics.ingameCellSize - 10, clientState.graphics.ingameCellSize - 10, 0, 360 * 64);
                    break;
                    

                default: // default means that this is player, make him like blue circle with its connection (ID) number

                    if (cell >= '0' && cell <= MAX_PLAYER_CHAR) {

                        // draw blue circle
                        XSetForeground(clientState.graphics.display, clientState.graphics.gc, WhitePixel(clientState.graphics.display, 0));
                        XFillRectangle(clientState.graphics.display, clientState.graphics.window, clientState.graphics.gc, cellStart.x, cellStart.y, 
                                      clientState.graphics.ingameCellSize, clientState.graphics.ingameCellSize);
                        
                        XSetForeground(clientState.graphics.display, clientState.graphics.gc, 0x0000FF);
                        XFillArc(clientState.graphics.display, clientState.graphics.window, clientState.graphics.gc, cellStart.x + 5, cellStart.y + 5, 
                                clientState.graphics.ingameCellSize - 10, clientState.graphics.ingameCellSize - 10, 0, 360 * 64);
                        
                        // draw number
                        XSetForeground(clientState.graphics.display, clientState.graphics.gc, WhitePixel(clientState.graphics.display, 0));

                        char player_str[2] = {cell, '\0'};
                        XDrawString(clientState.graphics.display, clientState.graphics.window, clientState.graphics.gc, cellStart.x + 15, cellStart.y + 25, 
                                    player_str, 1);
                    }
                    break;
            }
        }
    
    
    // show player status
    XSetForeground(clientState.graphics.display, clientState.graphics.gc, BlackPixel(clientState.graphics.display, 0));


    char statusMsg[200];    // buffer to contain and show msg

    if (clientState.isActive)
        snprintf(statusMsg, sizeof(statusMsg), "Time left: %d | Player %d is still alive", clientState.gameTimeLeft, clientState.playerID);
    else
        snprintf(statusMsg, sizeof(statusMsg), "Time left: %d | Player %d is dead", clientState.gameTimeLeft, clientState.playerID);
    

    XDrawString(clientState.graphics.display, clientState.graphics.window, clientState.graphics.gc, 10, clientState.graphics.winHeight - 100, statusMsg, strlen(statusMsg));
    

    pthread_mutex_unlock(&clientState.fieldMutex);
    XFlush(clientState.graphics.display);
}

// draw clean window
int drawClean() {


    XClearWindow(clientState.graphics.display, clientState.graphics.window);

    // // show player status
    // XSetForeground(clientState.graphics.display, clientState.graphics.gc, BlackPixel(clientState.graphics.display, 0));


    // char statusMsg[200];    // buffer to contain and show msg

    // if (clientState.isActive)
    //     snprintf(statusMsg, sizeof(statusMsg), "Time left: %d | Player %d is still alive", clientState.gameTimeLeft, clientState.playerID);
    // else
    //     snprintf(statusMsg, sizeof(statusMsg), "Time left: %d | Player %d is dead", clientState.gameTimeLeft, clientState.playerID);
    

    // XDrawString(clientState.graphics.display, clientState.graphics.window, clientState.graphics.gc, 10, clientState.graphics.winHeight - 100, statusMsg, strlen(statusMsg));
    

    XFlush(clientState.graphics.display);
}

// parse game state with timer now
void parseGameState(const char* gameData) {

    pthread_mutex_lock(&clientState.fieldMutex);
    
    // timer put after '|' from server
    char* timePos = strchr(gameData, '|');
    
    if (timePos) {

        // copy game field data
        int fieldSize = timePos - gameData;
        strncpy(clientState.gameField, gameData, fieldSize);
        clientState.gameField[fieldSize] = '\0';
        
        // copy time and convert to int
        clientState.gameTimeLeft = atoi(timePos + 1);

    } else {        // if timer not found (old server version)

        strcpy(clientState.gameField, gameData);
        clientState.gameTimeLeft = 0;
    }
    
    pthread_mutex_unlock(&clientState.fieldMutex);
}

// method to change window title and add player's number to it when connected successfully
void changeWindowTitle(const char* newFormattedTitle) {

    char windowTitle[100];

    snprintf(windowTitle, sizeof(windowTitle), newFormattedTitle, clientState.playerID);

    XStoreName(clientState.graphics.display, clientState.graphics.window, windowTitle);
    XFlush(clientState.graphics.display); // flush to show changes
}

// thread to loot data from our pretty server)
void* network_thread(void* args) {

    msg_t msg;

    while (1) {

        int bytes = recv(clientState.socket, &msg, sizeof(msg), 0);
        if (bytes <= 0) {
            perror("[Error]Соединение с сервером потеряно\n");
            break;
        }

        switch(msg.type) {
            
            case MSG_CONNECT:
                clientState.playerID = msg.playerID;
                clientState.isActive = 1; // restore activity when connected
                printf("[MSG_CONNECT]Успешно подключились к серверу как игрок %d\n", clientState.playerID);

                // change window title with my custom
                changeWindowTitle("Maze Game Client | Player %d");
                break;

            // now parse server game state data at parseGameState()
            case MSG_GAME_STATE:
                // bring mutex from drawGame and get new game data from server
                //pthread_mutex_lock(&clientState.fieldMutex);
                //strcpy(clientState.gameField, msg.gameData);    // copy sent by server game data (field string) to client one to visualize
                //pthread_mutex_unlock(&clientState.fieldMutex);

                parseGameState(msg.gameData);   // locks mutex by itself now
                drawGame();     // redraw game screen with actucal game data
                break;


            // check if lose msg for this player exactly
            case MSG_PLAYER_LOSE:
                if (msg.playerID == clientState.playerID) {
                    clientState.isActive = 0;
                    drawGame(); // redraw game to change status field at time

                    printf("Вы (игрок %d) проиграли!\n", clientState.playerID);
                }
                break;

                case MSG_RESTART_GAME:
                    if (msg.playerID == clientState.playerID) {

                        clientState.isActive = 1;   //reset player after game restart
                        printf("Игра перезапущена. А ваш игрок воскрес!\n");
                        drawGame();
                    }
                    break;
        }
    }

    return NULL;
}

// send player move issue (msg) to server
void sendMove(direction_t moveDir) {

    if (!clientState.isActive)
        return;
    
    msg_t msg;

    msg.type = MSG_MOVE;
    msg.playerID = clientState.playerID;
    msg.direction = moveDir;

    send(clientState.socket, &msg, sizeof(msg), 0);
}


void closeClient();
// process move keys using (WASD and just arrows)
// idk for now press or release so its just using
void processKeyEvent(XKeyEvent* event) {
    
    KeySym key = XLookupKeysym(event, 0);

    switch (key) {
        case XK_Up: case XK_w:
            sendMove(DIR_UP);
            break;

        case XK_Down: case XK_s:
            sendMove(DIR_DOWN);
            break;

        case XK_Left: case XK_a:
            sendMove(DIR_LEFT);
            break;

        case XK_Right: case XK_d:
            sendMove(DIR_RIGHT);
            break;

        case XK_Escape: // if esc used - close game
            closeClient();
            break;
    }
}


// handler to close app (now used in exitButton, so all other uses changed)
void closeClient() {
    puts("Closing app.. Goodbye!\n");
    exit(0);
}

// get server ip from argv, parse just ip argument as well
int main(int argc, char* argv[]) {

    // check arg count
    if (argc != 2) {    // first arg - './thisapp', second one - ip addr
        printf("Для запуска: %s <ip-сервера>\n", argv[0]);
        printf("Или введите ip через GUI\n");
        //return 1;
    }

    // nullify client state data and init its mutex
    // memset(&clientState, 0, sizeof(clientState));
    // pthread_mutex_init(&clientState.fieldMutex, NULL);
    // clientState.isActive = 1;


    // connection to server
    // clientState.socket = socket(AF_INET, SOCK_STREAM, 0);
    
    // if (clientState.socket < 0) {
    //     perror("[Error]Ошибка создания сокета\n");
    //     return 1;
    // }

    // struct sockaddr_in serverAddress;

    // serverAddress.sin_family = AF_INET;
    // serverAddress.sin_port = htons(SERVER_PORT);
    // inet_pton(AF_INET, argv[1], &serverAddress.sin_addr);
    
    // if (connect(clientState.socket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
    //     perror("[Error]Ошибка подключения к серверу\n");
    //     perror("[Error]Возможно, сервер недоступен или нет свободных слотов для подключения\n");
    //     return 1;
    // }
    
    // printf("Подключение к серверу %s:%d выполнено успешно\n", argv[1], SERVER_PORT);
    

    // init graphics, returns 1 if success
    if (!init()) {
        fprintf(stderr, "Client init error\n");
        return 1;
    }


    // game keys (to console)
    rulesWinClb rulesWindow = {
        clientState.graphics.display, 
        createAnotherParentWindow(clientState.graphics.display, RULES_WIDTH, RULES_HEIGHT, "Game Rules", "rules", "GameRules"),
        0,  // rules displayed
        1   // rules exist (created above)
        }; 
    // rules now in the window
    // puts("[Rules]Управление: WASD или стрелки, ESC - выход из игры\nЦель - выбраться из лабиринта, не попавшись врагу\n");


    // make client buttons with graphics initialized in clientState
    Button *connectButton, *rulesButton, *exitButton;

    // y offset between buttons - 20px
    connectButton = createButton(clientState.graphics.display, clientState.graphics.window, "Connect", 
    10, 10, 200, 50, COLOR_ORANGE, COLOR_WHITE, "fixed", COLOR_BLACK, NULL, NULL);

    rulesButton = createButton(clientState.graphics.display, clientState.graphics.window, "Rules", 
    10, 80, 200, 50, COLOR_CYAN, COLOR_WHITE, "fixed", COLOR_BLACK, &toggleRulesWindow, &rulesWindow);

    exitButton = createButton(clientState.graphics.display, clientState.graphics.window, "Exit", 
    10, 150, 200, 50, COLOR_PINK, COLOR_WHITE, "fixed", COLOR_BLACK, &closeClient, NULL);

    

    // pthread_t network_t_id;
    // pthread_create(&network_t_id, NULL, network_thread, NULL);


    XEvent event;
    while (1)
    {
        XNextEvent(clientState.graphics.display, &event);

            // button prior
            if (event.xbutton.window == connectButton->window)
                handleButtonEvent(&event, connectButton);
            else if (event.xbutton.window == rulesButton->window)
                handleButtonEvent(&event, rulesButton);
            else if (event.xbutton.window == exitButton->window)
                handleButtonEvent(&event, exitButton);

            // base + ingame event loop
            else if (event.xany.window == clientState.graphics.window) {
            switch (event.type)
            {

            case Expose:
                drawClean();
                break;

            case KeyPress:
                processKeyEvent(&event.xkey);
                break;
            case DestroyNotify:
                if (event.xdestroywindow.window == clientState.graphics.window)
                    closeClient();
                break;
            }
        
    }
        else if (event.xany.window == rulesWindow.window && rulesWindow.isRulesDisplayed) {
            switch (event.type) {
                case Expose:
                    drawRulesWindow(rulesWindow.display, rulesWindow.window);
                    break;
                case KeyPress: case ButtonPress:
                    toggleRulesWindow(&rulesWindow);
                    break;
                case ClientMessage:
                    // Handle window manager close request for rules window
                    // instead of direct DestroyNotify that destroys both windows
                    if (event.xclient.data.l[0] == wmDeleteMessage)
                    {

                        puts("[Debug]Rules window close requested - hiding\n");
                        toggleRulesWindow(&rulesWindow); // turn it off
                    }
                break;
            }
        }
    }


    // close(clientState.socket);
    // XCloseDisplay(clientState.graphics.display);
    return 0;
}