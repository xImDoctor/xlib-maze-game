#ifndef _XBUTTON_H
#define _XBUTTON_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xresource.h>

#include "xcolors.h"    // DefinedColors enum

#define DRAW_BORDER_PRESSED(_button) DrawBorder(_button, COLOR_BLACK, COLOR_GRAY)
#define DRAW_BORDER_RELEASED(_button) DrawBorder(_button, COLOR_GRAY, COLOR_BLACK)


typedef void (*ButtonCallback)(void*);  // pointer to void function - void (*)(void*)

///maah button
typedef struct Button Button;
struct Button {
    Window window;
    Display* display;

    const char* text;

    int text_width;
    int x, y, width, height;
    unsigned long border, background, foreground, fontColor; //hexadec num

    XFontStruct* font;

    ButtonCallback onClick;
    void* callbackData;

    int isMouseInside; // flag for click tracking
};

Button* createButton(Display* display, Window parent, 
    const char* text,
    int x, int y, int width, int height,
    unsigned long fg, unsigned long bg,
    const char* textFont, unsigned long fontColor,
    ButtonCallback cb, 
    void* cbd);

void buttonExpose(Button* button);                              // redraw
void buttonUpdate(Button* button, int newWidth, int newHeight); // update size and redraw - buttonExpose
void buttonEnter(Button* button);
void buttonLeave(Button* button);

// void buttonClicked(Button* button);
void buttonClicked(Button* button);

void handleButtonEvent(XEvent* event, Button* button);

void DrawBorder(Button* button, unsigned long topColor, unsigned long bottomColor);

// set callback function (or reset) any time after button init
void setButtonCallback(Button* button, ButtonCallback cb, void* cbd);

#endif