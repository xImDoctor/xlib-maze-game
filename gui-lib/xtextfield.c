#include "xtextfield.h"

#include <stdlib.h>
#include <stdio.h>

TextField *createTextField(Display *display, Window parent,
                     int x, int y, int width, int height,
                     unsigned long bg,
                     const char *textFont, unsigned long fontColor,
                     TextFieldCallback cb,
                     void *cbd)
{

    TextField *tfield = (TextField *)malloc(sizeof(TextField));
    if (!tfield)
        return NULL;

    tfield->display = display;
    tfield->x = x;
    tfield->y = y;
    tfield->width = width;
    tfield->height = height;
    tfield->background = bg;

    // setup flags
    tfield->isMouseInside = 0;
    tfield->isActive = 0;

    tfield->currentBorder = COLOR_GRAY; // DRAW_BORDER_NONE(tfield) coz not active


    if ((tfield->font = XLoadQueryFont(display, textFont)) == NULL)
    {
        printf("Фунтик не загрузился\n");
        exit(1);
    }
    tfield->fontColor = fontColor;

    // clear text buffer when init
    memset(tfield->textBuffer, ' ', sizeof(tfield->textBuffer));
    tfield->lastWrittenIndex = 0;

// mb no need
    tfield->callback = cb;
    tfield->callbackData = cbd;

    tfield->window = XCreateSimpleWindow(display, parent, x, y, width, height, 0, 0, bg);

    XSelectInput(display, tfield->window, ButtonPressMask | ButtonReleaseMask | ExposureMask | EnterWindowMask | LeaveWindowMask | KeyPressMask);
    XMapWindow(display, tfield->window);

    return tfield;
}


// redrawing
void fieldExpose(TextField* tfield)
{
    
    XClearWindow(tfield->display, tfield->window);

    REDRAW_BORDER(tfield);      // redraw with last color
    showWrittenText(tfield);

}

void fieldUpdate(TextField *tfield, int newWidth, int newHeight)
{
    tfield->width = newWidth;
    tfield->height = newHeight;

    XResizeWindow(tfield->display, tfield->window, tfield->width, tfield->height);
    fieldExpose(tfield);
}


void fieldEnter(TextField *tfield)
{
    tfield->isMouseInside = 1;
}

void fieldLeave(TextField *tfield)
{
    tfield->isMouseInside = 0;
}

void activateTextField(TextField* tfield) {
    tfield->isActive = 1;
    tfield->currentBorder = COLOR_BLACK;
    DRAW_BORDER_ACTIVE(tfield);
}

void deactivateTextField(TextField* tfield) {
        tfield->isActive = 0;
        tfield->currentBorder = COLOR_GRAY;
        DRAW_BORDER_NONE(tfield);
}

void inFieldClicked(TextField *tfield)
{
    if (tfield->isMouseInside) 
        activateTextField(tfield);
    else 
        deactivateTextField(tfield);

}

void handleTextFieldEvent(XEvent *event, TextField *tfield)
{

    switch (event->type)
    {
    case Expose:
        fieldExpose(tfield);
        break;
    case EnterNotify:
        fieldEnter(tfield);
        break;
    case LeaveNotify:
        fieldLeave(tfield);
        break;
    // case ButtonPress:
        // DRAW_BORDER_ACTIVE(tfield);
        // break;
    case ButtonPress:
    case ButtonRelease:
        inFieldClicked(tfield);
        break;
    case KeyPress:
        inFieldKeyPressed(tfield, event);
        break;
    }
}

void processTextFieldCallback(TextField* tfield) {

    if (tfield->callback && tfield->isActive)
        tfield->callback(tfield->callbackData);

}


void inFieldKeyPressed(TextField *tfield, XEvent *event)
{
    // if not active, doesnt process
    if (!tfield->isActive) 
        return;

    // KeySym key = XLookupKeysym(&event->xkey, 0); // besides XKeycodeToKeysym

    // get string and push to buffer with its lenght (sizeof). Symbol writes into key using its pointer
    char keyBuffer[32];
    KeySym key;
    int lenght = XLookupString(&event->xkey, keyBuffer, sizeof(keyBuffer), &key, NULL);


    if (key == XK_BackSpace && tfield->lastWrittenIndex) {

        tfield->lastWrittenIndex -= 1; 
        tfield->textBuffer[tfield->lastWrittenIndex] = '\0'; 


        // printf("[inFieldKeyPressed]Последняя введённая буква удалена: %s\n", tfield->textBuffer);

        fieldExpose(tfield);
        return;

    } else if (key == XK_KP_Enter || key == XK_Return) {       // Enter == Return on some keyboards yeah

        printf("Enter pressed in the textfield. Callback sent\n");

        processTextFieldCallback(tfield);   // send callback

        return;
    }
    else if (key == XK_Escape) {                                // if field active and Esc pressed, turn off focus
        
        printf("Esc pressed, field unactive now\n");
        deactivateTextField(tfield);

        return;
    }


    if (lenght > 0 && tfield->lastWrittenIndex + lenght < TEXT_BUFFER && key != XK_BackSpace) {
        memcpy(&tfield->textBuffer[tfield->lastWrittenIndex], keyBuffer, lenght);
        tfield->lastWrittenIndex += lenght;

        tfield->textBuffer[tfield->lastWrittenIndex] = '\0';
        // printf("[inFieldKeyPressed]Текущий ввод в textfield: %s\n", tfield->textBuffer);

        fieldExpose(tfield);
    }
}


void showWrittenText(TextField* tfield) {

    GC gc = XCreateGC(tfield->display, tfield->window, 0, NULL);
    XSetForeground(tfield->display, gc, tfield->fontColor); // text color

        // from left side
    int textX = TEXT_INNER_OFFSET;
    int textY = (tfield->height + 10) / 2;

    // written in buffer text (user input)
    XDrawString(tfield->display, tfield->window, gc, textX, textY, tfield->textBuffer, tfield->lastWrittenIndex);

    XFreeGC(tfield->display, gc);
}


/* 
The same border logic as in the xbutton.c
*/
void DrawFieldBorder(TextField *tfield, unsigned long borderColor)
{
    GC gc = XCreateGC(tfield->display, tfield->window, 0, NULL);

    XSetLineAttributes(tfield->display, gc, BORDER_THICKNESS, LineSolid, CapButt, JoinMiter);

    XSetForeground(tfield->display, gc, borderColor);
    XDrawLine(tfield->display, tfield->window, gc, 0, 0, tfield->width - 1, 0);
    XDrawLine(tfield->display, tfield->window, gc, 0, 0, 0, tfield->height - 1);

    XDrawLine(tfield->display, tfield->window, gc, 0, tfield->height - 1, tfield->width - 1, tfield->height - 1);
    XDrawLine(tfield->display, tfield->window, gc, tfield->width - 1, 0, tfield->width - 1, tfield->height - 1);

    tfield->currentBorder = borderColor;

    XFlush(tfield->display);
    XFreeGC(tfield->display, gc);
}
