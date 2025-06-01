#include "xbutton.h"
#include <stdlib.h>
#include <stdio.h>

Button *createButton(Display *display, Window parent,
                     const char *text,
                     int x, int y, int width, int height,
                     unsigned long fg, unsigned long bg,
                     const char *textFont, unsigned long fontColor,
                     ButtonCallback cb,
                     void *cbd)
{

    Button *button = (Button *)malloc(sizeof(Button));
    if (!button)
        return NULL;

    button->display = display;
    button->text = text;
    button->x = x;
    button->y = y;
    button->width = width;
    button->height = height;
    button->foreground = fg;
    button->background = bg;

    // mouse flag
    button->isMouseInside = 0;

    if ((button->font = XLoadQueryFont(display, textFont)) == NULL)
    {
        printf("Фунтик не загрузился\n");
        exit(1);
    }
    button->fontColor = fontColor;

    button->onClick = cb;
    button->callbackData = cbd;

    // button->window = XCreateSimpleWindow(display, parent, x, y, width, height, 1, border, bg);
    button->window = XCreateSimpleWindow(display, parent, x, y, width, height, 0, 0, bg);

    XSelectInput(display, button->window, ButtonPressMask | ButtonReleaseMask | ExposureMask | EnterWindowMask | LeaveWindowMask);
    XMapWindow(display, button->window);

    return button;
}

// redrawing
void buttonExpose(Button *button)
{
    GC gc = XCreateGC(button->display, button->window, 0, NULL);
    XSetForeground(button->display, gc, button->fontColor); // text color
    DRAW_BORDER_RELEASED(button);

    // int textX = (button->width - XTextWidth(DefaultFont(button->display), button->text, strlen(button->text))) / 2;
    int textX = (button->width - XTextWidth(button->font, button->text, strlen(button->text))) / 2;
    int textY = (button->height + 10) / 2;

    XDrawString(button->display, button->window, gc, textX, textY, button->text, strlen(button->text));

    XFreeGC(button->display, gc);
}

void buttonUpdate(Button *button, int newWidth, int newHeight)
{
    button->width = newWidth;
    button->height = newHeight;

    XResizeWindow(button->display, button->window, button->width, button->height);
    buttonExpose(button);
}

void clearRedrawing(Button *);
void buttonEnter(Button *button)
{
    button->isMouseInside = 1;
    XSetWindowBackground(button->display, button->window, button->foreground);
    clearRedrawing(button);
}

void buttonLeave(Button *button)
{
    button->isMouseInside = 0;
    XSetWindowBackground(button->display, button->window, button->background);
    clearRedrawing(button);
}


void buttonClicked(Button *button)
{
    DRAW_BORDER_RELEASED(button);
    if (button->onClick && button->isMouseInside)
        button->onClick(button->callbackData);
}

void handleButtonEvent(XEvent *event, Button *button)
{

    switch (event->type)
    {
    case Expose:
        buttonExpose(button);
        break;
    case EnterNotify:
        buttonEnter(button);
        break;
    case LeaveNotify:
        buttonLeave(button);
        break;
    case ButtonPress:
        DRAW_BORDER_PRESSED(button);
        break;
    case ButtonRelease:
        buttonClicked(button);
        break;
    }
}

// working func for clearly redrawing of button window
void clearRedrawing(Button *button)
{
    XClearWindow(button->display, button->window);
    buttonExpose(button);
}

/* Some draw border logic comment:
// verticle
// XDrawLine(button->display, button->window, gc, 0, 0, button->width - 1, 0);
// XDrawLine(button->display, button->window, gc, 0, button->height - 1, button->width - 1, button->height - 1);

// horizontal
// XDrawLine(button->display, button->window, gc, 0, 0, 0, button->height - 1);
// XDrawLine(button->display, button->window, gc, button->width - 1, 0, button->width - 1, button->height - 1);
*/
void DrawBorder(Button *button, unsigned long topColor, unsigned long bottomColor)
{
    GC gc = XCreateGC(button->display, button->window, 0, NULL);

    // line thickness
    XSetLineAttributes(button->display, gc, 3, LineSolid, CapButt, JoinMiter);

    XSetForeground(button->display, gc, topColor);
    XDrawLine(button->display, button->window, gc, 0, 0, button->width - 1, 0);
    XDrawLine(button->display, button->window, gc, 0, 0, 0, button->height - 1);

    XSetForeground(button->display, gc, bottomColor);
    XDrawLine(button->display, button->window, gc, 0, button->height - 1, button->width - 1, button->height - 1);
    XDrawLine(button->display, button->window, gc, button->width - 1, 0, button->width - 1, button->height - 1);

    XFlush(button->display);
    XFreeGC(button->display, gc);
}


void setButtonCallback(Button* button, ButtonCallback cb, void* cbd) {

    button->onClick = cb;
    button->callbackData = cbd;
}
