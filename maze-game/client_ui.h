#ifndef CLIENT_UI_H
#define CLIENT_UI_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

Atom wmDeleteMessage;


Window createAnotherParentWindow(Display* display, int width, int height, char* windowName, char* hintName, char* hintClass) {
    Window root = DefaultRootWindow(display);
    int screen = DefaultScreen(display);
    
    XSetWindowAttributes attrs;
    attrs.background_pixel = WhitePixel(display, screen);
    attrs.border_pixel = BlackPixel(display, screen);
    attrs.event_mask = ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask;
    
    Window newWindow = XCreateWindow(
        display,
        root,                                       // parent
        200, 150,                                   // x, y position
        width, height,                              // width, height
        2,                                          // border width
        CopyFromParent,                             // depth
        InputOutput,                                // class
        CopyFromParent,                             // visual
        CWBackPixel | CWBorderPixel | CWEventMask,  // value mask
        &attrs                                      // attributes
    );
    
    // win title
    XStoreName(display, newWindow, windowName);

    // window closing protocol to fix all window closing when exit from rules window for example
    wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, newWindow, &wmDeleteMessage, 1);
    
    // manager class (for win manager identification)
    XClassHint* classHint = XAllocClassHint();
    if (classHint) {

        classHint->res_name = hintName;
        classHint->res_class = hintClass;

        XSetClassHint(display, newWindow, classHint);
        XFree(classHint);
    }
    
    return newWindow;
}


// rules subwindow (but parent lvl)
#define RULES_WIDTH 300
#define RULES_HEIGHT 250

typedef struct rules_window_t rulesWinClb;
struct rules_window_t{      // rules window opt

    Display* display;
    Window window;
    int isRulesDisplayed;   // flag to toggle
    int isWindowExists;
};


// on/off rules window (window must be created already)
void toggleRulesWindow(void* rulesWinInfo) {
    rulesWinClb* rulesInfo = (rulesWinClb*)rulesWinInfo;

    if (!rulesInfo->isWindowExists)
            rulesInfo->window = createAnotherParentWindow(rulesInfo->display, RULES_WIDTH, RULES_HEIGHT, 
                                                            "Game Rules", "rules", "GameRules");

    if (!rulesInfo->isRulesDisplayed) {
        XMapWindow(rulesInfo->display, rulesInfo->window);

        rulesInfo->isRulesDisplayed = 1;
        puts("[Debug]Rules window shown\n");
    }
    else {
        XUnmapWindow(rulesInfo->display, rulesInfo->window);

        rulesInfo->isRulesDisplayed = 0;
        puts("[Debug]Rules window hidden\n");
    }
    
    // draw changes
    XFlush(rulesInfo->display);
}

void drawRulesWindow(Display* display, Window window) {
    GC gc = XCreateGC(display, window, 0, NULL);
    XSetForeground(display, gc, BlackPixel(display, DefaultScreen(display)));
    
    // Draw rules
    const char* rules[] = {
        "Game Rules:",
        "",
        "Controls: WASD or Arrow Keys",
        "ESC - Exit game",
        "",
        "Goal: Escape the maze without",
        "getting caught by the enemy",
        "",
        "Press any key to close this window"
    };
    
    int y = 30;
    int rulesArraySize = sizeof(rules)/sizeof(rules[0]); 
    for (int i = 0; i < rulesArraySize; i++) {
        XDrawString(display, window, gc, 20, y, rules[i], strlen(rules[i]));
        y += 20;
    }
    
    XFreeGC(display, gc);
}



#endif 