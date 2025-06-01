#ifndef _XTEXTFIELD_H
#define _XTEXTFIELD_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xresource.h>

#include "xcolors.h"     // DefinedColors enum

#define DRAW_BORDER_ACTIVE(_tfield) DrawFieldBorder(_tfield, COLOR_BLACK)
#define DRAW_BORDER_NONE(_tfield) DrawFieldBorder(_tfield, COLOR_GRAY)
#define REDRAW_BORDER(tfield) DrawFieldBorder(tfield, tfield->currentBorder)

#define TEXT_BUFFER 255

#define TEXT_INNER_OFFSET 5
#define BORDER_THICKNESS 3

typedef void (*TextFieldCallback)(void*);  // pointer to void function - void (*)(void*)

///maah butto... text field now
typedef struct TextField TextField;
struct TextField {
    Window window;
    Display* display;

    char textBuffer[TEXT_BUFFER];
    int lastWrittenIndex;

    int text_width;
    int x, y, width, height;
    unsigned long background, fontColor;
    unsigned long currentBorder;

    XFontStruct* font;

    TextFieldCallback callback;
    void* callbackData;

    int isMouseInside; // click tracking flag
    int isActive;      // active state flag
};


TextField* createTextField(Display* display, Window parent, 
    int x, int y, int width, int height,
    unsigned long bg,
    const char* textFont, unsigned long fontColor,
    TextFieldCallback cb, 
    void* cbd);

void showWrittenText(TextField* tfield);

void fieldExpose(TextField* tfield);                              // redraw
void fieldUpdate(TextField* tfield, int newWidth, int newHeight); // update size and redraw (fieldExpose)
void fieldEnter(TextField* tfield);
void fieldLeave(TextField* tfield);

void processTextFieldCallback(TextField* tfield);

void activateTextField(TextField* tfield);
void deactivateTextField(TextField* tfield);

void inFieldKeyPressed(TextField* tfield, XEvent* event);
void inFieldClicked(TextField* tfield);
void handleTextFieldEvent(XEvent* event, TextField* tfield);

void DrawFieldBorder(TextField* tfield, unsigned long borderColor);

#endif