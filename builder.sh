#!/bin/bash

# xbutton is here now
button="gui-lib/xbutton.c"
textfield="gui-lib/xtextfield.c"


# file names setup, need to change between client/server in this proj
filename=""

# compile
gcc "${filename}.c" -o "$filename" "$button" "$textfield" -lX11 -lXpm
