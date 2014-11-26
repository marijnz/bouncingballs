#pragma once

namespace gep
{
    /// \brief Provides windows virtual key values via enum values.
    /// Taken from http://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx
    struct WindowsVirtualKeyCodes
    {
        enum Enum
        {
            LButton  = VK_LBUTTON,  ///< Left mouse button
            RButton  = VK_RBUTTON,  ///< Right mouse button
            Cancel   = VK_CANCEL,   ///< Control-break processing
            MButton  = VK_MBUTTON,  ///< Middle mouse button (three-button mouse)
            ButtonX1 = VK_XBUTTON1, ///< X1 mouse button
            ButtonX2 = VK_XBUTTON2, ///< X2 mouse button

            Back = VK_BACK, ///< BACKSPACE key
            Tab  = VK_TAB,  ///< TAB key

            Clear  = VK_CLEAR,  ///< CLEAR key
            Return = VK_RETURN, ///< ENTER key

            Shift   = VK_SHIFT,   ///< SHIFT key
            Control = VK_CONTROL, ///< CTRL key
            Menu    = VK_MENU,    ///< ALT key
            Pause   = VK_PAUSE,   ///< PAUSE key
            Capital = VK_CAPITAL, ///< CAPS LOCK key
            Kana    = VK_KANA,    ///< IME Kana mode
            Hangul  = VK_HANGUL,  ///< IME Hangul mode

            Junja = VK_JUNJA, ///< IME Junja mode
            Final = VK_FINAL, ///< IME final mode
            Hanja = VK_HANJA, ///< IME Hanja mode
            Kanji = VK_KANJI, ///< IME Kanji mode

            Escape     = VK_ESCAPE,     ///< ESC key
            Convert    = VK_CONVERT,    ///< IME convert
            NonConvert = VK_NONCONVERT, ///< IME nonconvert
            Accept     = VK_ACCEPT,     ///< IME accept
            ModeChange = VK_MODECHANGE, ///< IME mode change request
            Space      = VK_SPACE,      ///< SPACEBAR
            Prior      = VK_PRIOR,      ///< PAGE UP key
            Next       = VK_NEXT,       ///< PAGE DOWN key
            End        = VK_END,        ///< END key
            Home       = VK_HOME,       ///< HOME key
            Left       = VK_LEFT,       ///< LEFT ARROW key
            Up         = VK_UP,         ///< UP ARROW key
            Right      = VK_RIGHT,      ///< RIGHT ARROW key
            Down       = VK_DOWN,       ///< DOWN ARROW key
            Select     = VK_SELECT,     ///< SELECT key
            Print      = VK_PRINT,      ///< PRINT key
            Execute    = VK_EXECUTE,    ///< EXECUTE key
            Snapshot   = VK_SNAPSHOT,   ///< PRINT SCREEN key
            Insert     = VK_INSERT,     ///< INS key
            Del        = VK_DELETE,     ///< DEL key
            Help       = VK_HELP,       ///< HELP key

            _0 = 0x30, ///< 0 key
            _1 = 0x31, ///< 1 key
            _2 = 0x32, ///< 2 key
            _3 = 0x33, ///< 3 key
            _4 = 0x34, ///< 4 key
            _5 = 0x35, ///< 5 key
            _6 = 0x36, ///< 6 key
            _7 = 0x37, ///< 7 key
            _8 = 0x38, ///< 8 key
            _9 = 0x39, ///< 9 key

            A = 0x41, ///< A key
            B = 0x42, ///< B key
            C = 0x43, ///< C key
            D = 0x44, ///< D key
            E = 0x45, ///< E key
            F = 0x46, ///< F key
            G = 0x47, ///< G key
            H = 0x48, ///< H key
            I = 0x49, ///< I key
            J = 0x4A, ///< J key
            K = 0x4B, ///< K key
            L = 0x4C, ///< L key
            M = 0x4D, ///< M key
            N = 0x4E, ///< N key
            O = 0x4F, ///< O key
            P = 0x50, ///< P key
            Q = 0x51, ///< Q key
            R = 0x52, ///< R key
            S = 0x53, ///< S key
            T = 0x54, ///< T key
            U = 0x55, ///< U key
            V = 0x56, ///< V key
            W = 0x57, ///< W key
            X = 0x58, ///< X key
            Y = 0x59, ///< Y key
            Z = 0x5A, ///< Z key

            LWin = VK_LWIN, ///< Left Windows key (Natural keyboard)
            RWin = VK_RWIN, ///< Right Windows key (Natural keyboard)
            Apps = VK_APPS, ///< Applications key (Natural keyboard)

            Sleep = VK_SLEEP,         ///< Computer Sleep key
            Numpad0 = VK_NUMPAD0,     ///< Numeric keypad 0 key
            Numpad1 = VK_NUMPAD1,     ///< Numeric keypad 1 key
            Numpad2 = VK_NUMPAD2,     ///< Numeric keypad 2 key
            Numpad3 = VK_NUMPAD3,     ///< Numeric keypad 3 key
            Numpad4 = VK_NUMPAD4,     ///< Numeric keypad 4 key
            Numpad5 = VK_NUMPAD5,     ///< Numeric keypad 5 key
            Numpad6 = VK_NUMPAD6,     ///< Numeric keypad 6 key
            Numpad7 = VK_NUMPAD7,     ///< Numeric keypad 7 key
            Numpad8 = VK_NUMPAD8,     ///< Numeric keypad 8 key
            Numpad9 = VK_NUMPAD9,     ///< Numeric keypad 9 key
            Multiply = VK_MULTIPLY,   ///< Multiply key
            Add = VK_ADD,             ///< Add key
            Separator = VK_SEPARATOR, ///< Separator key
            Subtract = VK_SUBTRACT,   ///< Subtract key
            Decimal = VK_DECIMAL,     ///< Decimal key
            Divide = VK_DIVIDE,       ///< Divide key

            F1  = VK_F1,  ///< F1 key
            F2  = VK_F2,  ///< F2 key
            F3  = VK_F3,  ///< F3 key
            F4  = VK_F4,  ///< F4 key
            F5  = VK_F5,  ///< F5 key
            F6  = VK_F6,  ///< F6 key
            F7  = VK_F7,  ///< F7 key
            F8  = VK_F8,  ///< F8 key
            F9  = VK_F9,  ///< F9 key
            F10 = VK_F10, ///< F10 key
            F11 = VK_F11, ///< F11 key
            F12 = VK_F12, ///< F12 key
            F13 = VK_F13, ///< F13 key
            F14 = VK_F14, ///< F14 key
            F15 = VK_F15, ///< F15 key
            F16 = VK_F16, ///< F16 key
            F17 = VK_F17, ///< F17 key
            F18 = VK_F18, ///< F18 key
            F19 = VK_F19, ///< F19 key
            F20 = VK_F20, ///< F20 key
            F21 = VK_F21, ///< F21 key
            F22 = VK_F22, ///< F22 key
            F23 = VK_F23, ///< F23 key
            F24 = VK_F24, ///< F24 key

            NumLock    = VK_NUMLOCK, ///< NUM LOCK key
            ScrollLock = VK_SCROLL,  ///< SCROLL LOCK key

            LShift              = VK_LSHIFT,              ///< Left SHIFT key
            RShift              = VK_RSHIFT,              ///< Right SHIFT key
            LControl            = VK_LCONTROL,            ///< Left CONTROL key
            RControl            = VK_RCONTROL,            ///< Right CONTROL key
            LMenu               = VK_LMENU,               ///< Left MENU key
            RMenu               = VK_RMENU,               ///< Right MENU key
            Browser_Back        = VK_BROWSER_BACK,        ///< Browser Back key
            Browser_ForwarD     = VK_BROWSER_FORWARD,     ///< Browser Forward key
            Browser_RefresH     = VK_BROWSER_REFRESH,     ///< Browser Refresh key
            Browser_Stop        = VK_BROWSER_STOP,        ///< Browser Stop key
            Browser_Search      = VK_BROWSER_SEARCH,      ///< Browser Search key
            Browser_Favorites   = VK_BROWSER_FAVORITES,   ///< Browser Favorites key
            Browser_Home        = VK_BROWSER_HOME,        ///< Browser Start and Home key
            Volume_Mute         = VK_VOLUME_MUTE,         ///< Volume Mute key
            Volume_Down         = VK_VOLUME_DOWN,         ///< Volume Down key
            Volume_Up           = VK_VOLUME_UP,           ///< Volume Up key
            Media_NextTrack     = VK_MEDIA_NEXT_TRACK,    ///< Next Track key
            Media_PrevTrack     = VK_MEDIA_PREV_TRACK,    ///< Previous Track key
            Media_Stop          = VK_MEDIA_STOP,          ///< Stop Media key
            Media_Play_Pause    = VK_MEDIA_PLAY_PAUSE,    ///< Play/Pause Media key
            Launch_Mail         = VK_LAUNCH_MAIL,         ///< Start Mail key
            Launch_Media_select = VK_LAUNCH_MEDIA_SELECT, ///< Select Media key
            Launch_App1         = VK_LAUNCH_APP1,         ///< Start Application 1 key
            Launch_App2         = VK_LAUNCH_APP2,         ///< Start Application 2 key

            Oem_1      = VK_OEM_1,      ///< Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the ';:' key
            Oem_Plus   = VK_OEM_PLUS,   ///< For any country/region, the '+' key
            Oem_Comma  = VK_OEM_COMMA,  ///< For any country/region, the ',' key
            Oem_Minus  = VK_OEM_MINUS,  ///< For any country/region, the '-' key
            Oem_Period = VK_OEM_PERIOD, ///< For any country/region, the '.' key
            Oem_2      = VK_OEM_2,      ///< Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '/?' key
            Oem_3      = VK_OEM_3,      ///< Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '`~' key

            Oem_4 = VK_OEM_4, ///< Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '[{' key
            Oem_5 = VK_OEM_5, ///< Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '\|' key
            Oem_6 = VK_OEM_6, ///< Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the ']}' key
            Oem_7 = VK_OEM_7, ///< Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the 'single-quote/double-quote' key
            Oem_8 = VK_OEM_8, ///< Used for miscellaneous characters; it can vary by keyboard.

            Oem_102 = VK_OEM_102, ///< Either the angle bracket key or the backslash key on the RT 102-key keyboard

            Processkey = VK_PROCESSKEY, ///< IME PROCESS key

            Packet = VK_PACKET, ///< Used to pass Unicode characters as if they were keystrokes. The VK_PACKET key is the low word of a 32-bit Virtual Key value used for non-keyboard input methods. For more information, see Remark in KEYBDINPUT, SendInput, WM_KEYDOWN, and WM_KEYUP

            Attn      = VK_ATTN,     ///< Attn key
            CrSel     = VK_CRSEL,    ///< CrSel key
            ExSel     = VK_EXSEL,    ///< ExSel key
            EraseEof  = VK_EREOF,    ///< Erase EOF key
            Play      = VK_PLAY,     ///< Play key
            Zoom      = VK_ZOOM,     ///< Zoom key
            NoName    = VK_NONAME,   ///< Reserved
            Pa1       = VK_PA1,      ///< PA1 key
            Oem_Clear = VK_OEM_CLEAR ///< Clear key
        };

        GEP_DISALLOW_CONSTRUCTION(WindowsVirtualKeyCodes);
    };

    // For convenience
    typedef WindowsVirtualKeyCodes Key;
}
