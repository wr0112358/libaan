#include "x11.hh"

#include "time.hh"

#include <memory>
#include <X11/Xlib.h> 
#include <X11/Xatom.h>

#include <cassert>
#include <iostream>
#include <thread>
// doc: http://www.x.org/archive/X11R7.5/doc/libX11/libX11.html

bool libaan::add_to_clipboard(const std::string &string,
                              std::chrono::milliseconds try_for)
{
    using display_ptr = std::unique_ptr<Display, decltype(&::XCloseDisplay)>;
    display_ptr display(XOpenDisplay(NULL), ::XCloseDisplay);

    if(!display)
        return false;

    const auto TARGETS_ATOM = XInternAtom(display.get(), "TARGETS", False);
    const auto UTF8STRING_ATOM = XInternAtom (display.get(), "UTF8_STRING", False);

    Window window = XCreateWindow(display.get(), DefaultRootWindow(display.get()),
                                  // x, y, w, h
                                  0, 0, 1, 1,
                                  // border_width, depth
                                  0, 0,
                                  InputOnly,
                                  CopyFromParent, 0, nullptr);
    XSelectInput(display.get(), window, StructureNotifyMask);
    XMapWindow(display.get(), window);
    XIconifyWindow(display.get(), window, DefaultScreen(display.get()));

    XEvent event;
    do // search for MapNotify Event in event queue, may block
        XNextEvent(display.get(), &event);
    while(event.type != MapNotify);

    XFlush(display.get());
    XSelectInput(display.get(), window, StructureNotifyMask + ExposureMask);

    Bool event_status = False;
    libaan::timer_ms timer;
    do {
        XSetSelectionOwner(display.get(), XA_PRIMARY, window, CurrentTime);
        XFlush (display.get());

        // Get first SelectionRequest
        while((event_status = XCheckTypedEvent(display.get(), SelectionRequest, &event))
              == False) {
            if(timer.duration() >= try_for.count()) {
                //std::cout << "TIMEOUT\n";
                XDestroyWindow(display.get(), window);
                return false;
            }
            //std::cout << "." << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        assert(event_status == True);

        if(event.xselectionrequest.target == XA_STRING
           || event.xselectionrequest.target == UTF8STRING_ATOM)
            break;
        else if(event.xselectionrequest.target != TARGETS_ATOM)
            continue;

        // special setup necessary for kde applications.(target == TARGETS_ATOM)
        // send XEvent and wait for next SelectionRequest
        // see function CopyFromWindows in:
        // http://www.mail-archive.com/cygwin-xfree@cygwin.com/msg00897.html

        Atom target_atoms[2] = {UTF8STRING_ATOM, XA_STRING};
        XChangeProperty(display.get(), event.xselectionrequest.requestor,
                        event.xselectionrequest.property,
                        event.xselectionrequest.target, 8,
                        PropModeReplace, (unsigned char *)(target_atoms),
                        sizeof(target_atoms));

        // Setup selection notify xevent
        XSelectionEvent respond;
        respond.type = SelectionNotify;
        respond.send_event = True;
        respond.display = display.get();
        respond.requestor = event.xselectionrequest.requestor;
        respond.selection = event.xselectionrequest.selection;
        respond.target = event.xselectionrequest.target;
        respond.property = event.xselectionrequest.property;
        respond.time = event.xselectionrequest.time;
        // Notify the requesting window that the operation has completed
        XSendEvent(display.get(), respond.requestor, False, 0L,
                   (XEvent *)&respond);
        XFlush(display.get());
    } while(true);

    const XSelectionRequestEvent &request = event.xselectionrequest;

/*
    std::cout << "Received Selection Request\n"
              << "\twindow(" << (int)window << ")"
              << " -> received selection request from:"
              << "\n\t\txselection.owner(" << (int)request.owner << ")"
              << "\n\t\txselection.requestor("
              << (int)request.requestor << ")->(property = " << request.property
              << ", target = " << request.target << ", selection = " << request.selection
              << ")\n";
*/

    // Setup the receiving window and send the string:
    XChangeProperty(display.get(), request.requestor, request.property, XA_STRING, 8,
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
    XSendEvent(display.get(), request.requestor, 0, 0, &respond);
    XFlush(display.get());

    XDestroyWindow(display.get(), window);
    return true;
}
