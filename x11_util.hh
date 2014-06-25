/*
Copyright (C) 2014 Reiter Wolfgang wr0112358@gmail.com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef _LIBAAN_X11_UTIL_HH_
#define _LIBAAN_X11_UTIL_HH_

#include <chrono>
#include <iostream>
#include <unistd.h>   
#include <cstdlib>

namespace libaan {
namespace util {
namespace x11 {
bool add_to_clipboard(const std::string &string,
                      std::chrono::milliseconds try_for);
}
}
}

#include "chrono_util.hh"
#include <X11/Xlib.h> 
#include <X11/Xatom.h>

// doc: http://www.x.org/archive/X11R7.5/doc/libX11/libX11.html
/**
 * Blocks until the user "empties the selection buffer".
 */
bool libaan::util::x11::add_to_clipboard(const std::string &string,
                                         std::chrono::milliseconds try_for)
{
    Display *display = XOpenDisplay(NULL);
    if(!display)
        return false;
    Window window = XCreateSimpleWindow(display, DefaultRootWindow(display), 0,
                                        0, 200, 100, 0, 0, 0);
    XSelectInput(display, window, StructureNotifyMask);
    XMapWindow(display, window);

    XEvent event;
    do // search for MapNotify Event in event queue, may block
        XNextEvent(display, &event);
    while(event.type != MapNotify);

    XFlush(display);
    XSelectInput(display, window, StructureNotifyMask + ExposureMask);

    Bool event_status = false;
    libaan::util::time_me_ms timer;
    do {
        XSetSelectionOwner(display, XA_PRIMARY, window, CurrentTime);
        XFlush (display);

        // Get first SelectionRequest
        while(!(event_status = XCheckTypedEvent(display, SelectionRequest, &event))) {
            if(timer.duration() >= try_for.count()) {
                std::cout << "TIMEOUT\n";
                return false;
            }
        }
        if(event_status)
            break;
    } while(!event_status || (event.xselectionrequest.target != XA_STRING));

    std::cout << "Received Selection Request\n";

    const XSelectionRequestEvent &request = event.xselectionrequest;
    std::cout << "\twindow(" << (int)window << ")"
        << " -> received selection request from:\n\t\txselection.requester("
        << (int)event.xselection.requestor << ")->(property = " << request.property
        << ", target = " << request.target << ", selection = " << request.selection
        << ")\n";

    XChangeProperty(display, request.requestor, request.property, XA_STRING, 8,
                    PropModeReplace,
                    reinterpret_cast<const unsigned char *>(string.c_str()),
                    static_cast<int>(string.size()));
    XEvent respond;
    respond.xselection.property = request.property;
    respond.xselection.type = SelectionNotify;
    respond.xselection.display = request.display;
    respond.xselection.requestor = request.requestor;
    respond.xselection.selection = request.selection;
    respond.xselection.target = request.target;
    respond.xselection.time = request.time;
    XSendEvent(display, request.requestor, 0, 0, &respond);
    XFlush(display);

/* according to stackoverflow kde might need some special code.. can't test, only copied.
    const Atom XA_TARGETS = XInternAtom(display, "TARGETS", True);
    const Atom XA_CLIPBOARD = XInternAtom(display, "CLIPBOARD", True);
    if (request.target == XA_TARGETS &&
               request.selection == XA_CLIPBOARD) {
        std::cout << "\tKDE is used.(Untested code)\n";
        Atom supported[] = {XA_STRING};
        XChangeProperty(display, request.requestor,
                        request.property,
                        XA_TARGETS, 8,
                        PropModeReplace, (unsigned char *)(&supported),
                        sizeof(supported));
    } else {    // Strings only please
        std::cout << "\tTarget(" << (int)request.target << ") not a string\n";
        respond.xselection.property = None;
    }
*/

    return true;
}

#endif
