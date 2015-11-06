#include "glfwKeyCodes.h"

#include <GLFW/glfw3.h>

namespace df3d { namespace platform {

KeyboardEvent::KeyCode convertGlfwKeyCode(int keycode)
{
    switch (keycode)
    {
    case GLFW_KEY_SPACE:
        return KeyboardEvent::KEY_SPACE;
    case GLFW_KEY_MINUS:
        return KeyboardEvent::KEY_MINUS;
    case GLFW_KEY_SLASH:
        return KeyboardEvent::KEY_SLASH;
    case GLFW_KEY_EQUAL:
        return KeyboardEvent::KEY_EQUAL;
    case GLFW_KEY_0:
        return KeyboardEvent::KEY_0;
    case GLFW_KEY_1:
        return KeyboardEvent::KEY_1;
    case GLFW_KEY_2:
        return KeyboardEvent::KEY_2;
    case GLFW_KEY_3:
        return KeyboardEvent::KEY_3;
    case GLFW_KEY_4:
        return KeyboardEvent::KEY_4;
    case GLFW_KEY_5:
        return KeyboardEvent::KEY_5;
    case GLFW_KEY_6:
        return KeyboardEvent::KEY_6;
    case GLFW_KEY_7:
        return KeyboardEvent::KEY_7;
    case GLFW_KEY_8:
        return KeyboardEvent::KEY_8;
    case GLFW_KEY_9:
        return KeyboardEvent::KEY_9;
    case GLFW_KEY_A:
        return KeyboardEvent::KEY_A;
    case GLFW_KEY_B:
        return KeyboardEvent::KEY_B;
    case GLFW_KEY_C:
        return KeyboardEvent::KEY_C;
    case GLFW_KEY_D:
        return KeyboardEvent::KEY_D;
    case GLFW_KEY_E:
        return KeyboardEvent::KEY_E;
    case GLFW_KEY_F:
        return KeyboardEvent::KEY_F;
    case GLFW_KEY_G:
        return KeyboardEvent::KEY_G;
    case GLFW_KEY_H:
        return KeyboardEvent::KEY_H;
    case GLFW_KEY_I:
        return KeyboardEvent::KEY_I;
    case GLFW_KEY_J:
        return KeyboardEvent::KEY_J;
    case GLFW_KEY_K:
        return KeyboardEvent::KEY_K;
    case GLFW_KEY_L:
        return KeyboardEvent::KEY_L;
    case GLFW_KEY_M:
        return KeyboardEvent::KEY_M;
    case GLFW_KEY_N:
        return KeyboardEvent::KEY_N;
    case GLFW_KEY_O:
        return KeyboardEvent::KEY_O;
    case GLFW_KEY_P:
        return KeyboardEvent::KEY_P;
    case GLFW_KEY_Q:
        return KeyboardEvent::KEY_Q;
    case GLFW_KEY_R:
        return KeyboardEvent::KEY_R;
    case GLFW_KEY_S:
        return KeyboardEvent::KEY_S;
    case GLFW_KEY_T:
        return KeyboardEvent::KEY_T;
    case GLFW_KEY_U:
        return KeyboardEvent::KEY_U;
    case GLFW_KEY_V:
        return KeyboardEvent::KEY_V;
    case GLFW_KEY_W:
        return KeyboardEvent::KEY_W;
    case GLFW_KEY_X:
        return KeyboardEvent::KEY_X;
    case GLFW_KEY_Y:
        return KeyboardEvent::KEY_Y;
    case GLFW_KEY_Z:
        return KeyboardEvent::KEY_Z;
    case GLFW_KEY_LEFT_BRACKET:
        return KeyboardEvent::KEY_LEFT_BRACKET;
    case GLFW_KEY_RIGHT_BRACKET:
        return KeyboardEvent::KEY_RIGHT_BRACKET;
    case GLFW_KEY_BACKSLASH:
        return KeyboardEvent::KEY_BACKSLASH;
    case GLFW_KEY_GRAVE_ACCENT:
        return KeyboardEvent::KEY_GRAVE_ACCENT;
    case GLFW_KEY_ESCAPE:
        return KeyboardEvent::KEY_ESCAPE;
    case GLFW_KEY_ENTER:
        return KeyboardEvent::KEY_ENTER;
    case GLFW_KEY_TAB:
        return KeyboardEvent::KEY_TAB;
    case GLFW_KEY_BACKSPACE:
        return KeyboardEvent::KEY_BACKSPACE;
    case GLFW_KEY_INSERT:
        return KeyboardEvent::KEY_INSERT;
    case GLFW_KEY_DELETE:
        return KeyboardEvent::KEY_DELETE;
    case GLFW_KEY_LEFT:
        return KeyboardEvent::KEY_LEFT;
    case GLFW_KEY_RIGHT:
        return KeyboardEvent::KEY_RIGHT;
    case GLFW_KEY_UP:
        return KeyboardEvent::KEY_UP;
    case GLFW_KEY_DOWN:
        return KeyboardEvent::KEY_DOWN;
    case GLFW_KEY_PAGE_UP:
        return KeyboardEvent::KEY_PAGE_UP;
    case GLFW_KEY_PAGE_DOWN:
        return KeyboardEvent::KEY_PAGE_DOWN;
    case GLFW_KEY_HOME:
        return KeyboardEvent::KEY_HOME;
    case GLFW_KEY_END:
        return KeyboardEvent::KEY_END;
    case GLFW_KEY_CAPS_LOCK:
        return KeyboardEvent::KEY_CAPS_LOCK;
    case GLFW_KEY_SCROLL_LOCK:
        return KeyboardEvent::KEY_SCROLL_LOCK;
    case GLFW_KEY_NUM_LOCK:
        return KeyboardEvent::KEY_NUM_LOCK;
    case GLFW_KEY_PRINT_SCREEN:
        return KeyboardEvent::KEY_PRINT_SCREEN;
    case GLFW_KEY_F1:
        return KeyboardEvent::KEY_F1;
    case GLFW_KEY_F2:
        return KeyboardEvent::KEY_F2;
    case GLFW_KEY_F3:
        return KeyboardEvent::KEY_F3;
    case GLFW_KEY_F4:
        return KeyboardEvent::KEY_F4;
    case GLFW_KEY_F5:
        return KeyboardEvent::KEY_F5;
    case GLFW_KEY_F6:
        return KeyboardEvent::KEY_F6;
    case GLFW_KEY_F7:
        return KeyboardEvent::KEY_F7;
    case GLFW_KEY_F8:
        return KeyboardEvent::KEY_F8;
    case GLFW_KEY_F9:
        return KeyboardEvent::KEY_F9;
    case GLFW_KEY_F10:
        return KeyboardEvent::KEY_F10;
    case GLFW_KEY_F11:
        return KeyboardEvent::KEY_F11;
    case GLFW_KEY_F12:
        return KeyboardEvent::KEY_F12;
    case GLFW_KEY_LEFT_SHIFT:
        return KeyboardEvent::KEY_LEFT_SHIFT;
    case GLFW_KEY_LEFT_CONTROL:
        return KeyboardEvent::KEY_LEFT_CTRL;
    case GLFW_KEY_LEFT_ALT:
        return KeyboardEvent::KEY_LEFT_ALT;
    case GLFW_KEY_RIGHT_SHIFT:
        return KeyboardEvent::KEY_RIGHT_SHIFT;
    case GLFW_KEY_RIGHT_CONTROL:
        return KeyboardEvent::KEY_RIGHT_CTRL;
    case GLFW_KEY_RIGHT_ALT:
        return KeyboardEvent::KEY_RIGHT_ALT;
    default:
        break;
    }

    return KeyboardEvent::UNDEFINED;
}

} }
