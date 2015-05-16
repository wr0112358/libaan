#include "x11.hh"

#include "time.hh"
#include <X11/Xlib.h> 
#include <X11/Xatom.h>

// doc: http://www.x.org/archive/X11R7.5/doc/libX11/libX11.html
/**
 * Blocks until the user "empties the selection buffer".
 */
bool libaan::add_to_clipboard(const std::string &string,
                              std::chrono::milliseconds try_for)
{
    Display *display = XOpenDisplay(NULL);
    if(!display)
        return false;

    const auto TARGETS_ATOM = XInternAtom(display, "TARGETS", False);
    const auto UTF8STRING_ATOM = XInternAtom (display, "UTF8_STRING", False);

    Window window = XCreateWindow(display, DefaultRootWindow(display),
                                  0, 0, 1, 1, // x, y, w, h
                                  0, 0, // border_width, depth
                                  InputOnly,
                                  CopyFromParent, 0, nullptr);
    XSelectInput(display, window, StructureNotifyMask);
    XMapWindow(display, window);
    XIconifyWindow(display, window, DefaultScreen(display));

    XEvent event;
    do // search for MapNotify Event in event queue, may block
        XNextEvent(display, &event);
    while(event.type != MapNotify);

    XFlush(display);
    XSelectInput(display, window, StructureNotifyMask + ExposureMask);

    Bool event_status = false;
    bool loop_done = false;
    libaan::timer_ms timer;
    do {
        XSetSelectionOwner(display, XA_PRIMARY, window, CurrentTime);
        XFlush (display);

        // Get first SelectionRequest
        while(!(event_status = XCheckTypedEvent(display, SelectionRequest, &event))) {
            if(timer.duration() >= try_for.count()) {
                //std::cout << "TIMEOUT\n";
                return false;
            }
        }

        if(event_status) {
            if(event.xselectionrequest.target == XA_STRING
               || event.xselectionrequest.target == UTF8STRING_ATOM)
                loop_done = true;
            else if(event.xselectionrequest.target == TARGETS_ATOM) {

                // special setup necessary for kde applications.
                // send XEvent and wait for next SelectionRequest
                // see function CopyFromWindows in:
                // http://www.mail-archive.com/cygwin-xfree@cygwin.com/msg00897.html

                Atom target_atoms[2] = {UTF8STRING_ATOM, XA_STRING};
                XChangeProperty(display, event.xselectionrequest.requestor,
                                event.xselectionrequest.property,
                                event.xselectionrequest.target, 8,
                                PropModeReplace, (unsigned char *)(target_atoms),
                                sizeof(target_atoms));

                // Setup selection notify xevent
                XSelectionEvent respond;
                respond.type = SelectionNotify;
                respond.send_event = True;
                respond.display = display;
                respond.requestor = event.xselectionrequest.requestor;
                respond.selection = event.xselectionrequest.selection;
                respond.target = event.xselectionrequest.target;
                respond.property = event.xselectionrequest.property;
                respond.time = event.xselectionrequest.time;
                // Notify the requesting window that the operation has completed
                XSendEvent(display, respond.requestor, False, 0L,
                           (XEvent *)&respond);
                XFlush(display);
            }
        }
    } while(!loop_done);

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

    return true;
}
