#pragma once
#include "api.hpp"

namespace mse
{
    enum class Key : std::int32_t
    {
        A = GLFW_KEY_A,
        B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

        Num0 = GLFW_KEY_0,
        Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,

        F1 = GLFW_KEY_F1,
        F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

        Up = GLFW_KEY_UP,
        Down = GLFW_KEY_DOWN,
        Left = GLFW_KEY_LEFT,
        Right = GLFW_KEY_RIGHT,

        LShift = GLFW_KEY_LEFT_SHIFT,
        RShift = GLFW_KEY_RIGHT_SHIFT,
        Shift = GLFW_KEY_LEFT_SHIFT,

        LCtrl = GLFW_KEY_LEFT_CONTROL,
        RCtrl = GLFW_KEY_RIGHT_CONTROL,
        Ctrl = GLFW_KEY_LEFT_CONTROL,

        LAlt = GLFW_KEY_LEFT_ALT,
        RAlt = GLFW_KEY_RIGHT_ALT,
        Alt = GLFW_KEY_LEFT_ALT,

        LSuper = GLFW_KEY_LEFT_SUPER,
        RSuper = GLFW_KEY_RIGHT_SUPER,

        Esc = GLFW_KEY_ESCAPE,
        CapsLock = GLFW_KEY_CAPS_LOCK,
        Space = GLFW_KEY_SPACE,
        Enter = GLFW_KEY_ENTER,
        Backspace = GLFW_KEY_BACKSPACE,
        Tab = GLFW_KEY_TAB,

        Insert = GLFW_KEY_INSERT,
        Delete = GLFW_KEY_DELETE,
        Home = GLFW_KEY_HOME,
        End = GLFW_KEY_END,
        PageUp = GLFW_KEY_PAGE_UP,
        PageDown = GLFW_KEY_PAGE_DOWN,

        PrintScreen = GLFW_KEY_PRINT_SCREEN,
        ScrollLock = GLFW_KEY_SCROLL_LOCK,
        Pause = GLFW_KEY_PAUSE,

        Dot = GLFW_KEY_PERIOD,
        Comma = GLFW_KEY_COMMA,
        Slash = GLFW_KEY_SLASH,
        Backslash = GLFW_KEY_BACKSLASH,
        Apostrophe = GLFW_KEY_APOSTROPHE,
        Semicolon = GLFW_KEY_SEMICOLON,
        Equal = GLFW_KEY_EQUAL,
        Minus = GLFW_KEY_MINUS,
        LBracket = GLFW_KEY_LEFT_BRACKET,
        RBracket = GLFW_KEY_RIGHT_BRACKET,
        GraveAccent = GLFW_KEY_GRAVE_ACCENT,

        Count = RSuper + 1
    };

    class MSE_API Input final
    {
    public:
        static void init();
        static void update();

        static bool up(Key key);
        static bool down(Key key);

        static bool just_pressed(Key key);
        static bool just_released(Key key);

    private:
        Input() = default;

        static Input& instance()
        {
            static Input input;
            return input;
        }

        static constexpr size_t key_byte_count = (static_cast<size_t>(Key::Count) + 7) / 8;

        char up_down_[key_byte_count];
        char pressed_released_[key_byte_count];

        static void set(char* flags, Key key, bool value);
        static bool get(const char* flags, Key key);
    };
}