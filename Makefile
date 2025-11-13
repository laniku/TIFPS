NAME = TIFPS
ICON = 
DESCRIPTION = "3D FPS Demo and Game for the 84+CE"
COMPRESSED = NO

CFLAGS = -O3 -Wall -Wextra -std=c11 -fomit-frame-pointer -ffast-math -funroll-loops -finline-functions

include $(shell cedev-config --makefile)

