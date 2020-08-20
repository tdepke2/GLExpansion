#ifndef EVENT_H_
#define EVENT_H_

using namespace std;

class Event {
    public:
    enum EventType {
        Close, Resize, KeyPress, KeyRelease, MouseButtonPress, MouseButtonRelease, MouseMove, MouseScroll
    };
    struct SizeEvent {
        int width;
        int height;
    };
    struct KeyEvent {
        int code;
        int scancode;
        int mods;
    };
    struct MouseButtonEvent {
        int code;
        int mods;
    };
    struct MouseMoveEvent {
        double xpos;
        double ypos;
    };
    struct MouseScrollEvent {
        double xoffset;
        double yoffset;
    };
    
    EventType type;
    union {
        SizeEvent size;
        KeyEvent key;
        MouseButtonEvent mouseButton;
        MouseMoveEvent mouseMove;
        MouseScrollEvent mouseScroll;
    };
};

#endif
