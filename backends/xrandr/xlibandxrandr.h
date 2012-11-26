#ifndef XLIBANDXRANDR_H
#define XLIBANDXRANDR_H

extern "C"
{
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#define INT8 _X11INT8
#define INT32 _X11INT32
#include <X11/Xproto.h>
#undef INT8
#undef INT32
#include <X11/extensions/Xrandr.h>
}

#include <fixx11h.h>

#endif // XLIBANDXRANDR