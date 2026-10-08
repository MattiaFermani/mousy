#include "UInputManager.h"
#include <QDebug>
#include <QStringList>
#include <QRegularExpression>
#include <thread>
#include <chrono>

#ifdef __linux__
#include <linux/uinput.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cstring>
#include <cerrno>
#endif

UInputManager& UInputManager::instance() {
    static UInputManager s_instance;
    return s_instance;
}

UInputManager::UInputManager() {
    initialize();
}

UInputManager::~UInputManager() {
    shutdown();
}

bool UInputManager::isAvailable() const {
    return initialized && (uinputFd >= 0);
}

QString UInputManager::statusMessage() const {
    if (isAvailable()) {
        return "Virtual Device Active (/dev/uinput)";
    }
    if (!lastError.isEmpty()) {
        return lastError;
    }
    return "Virtual device not initialized. Check /etc/udev/rules.d/99-mousy.rules";
}

bool UInputManager::initialize() {
#ifdef __linux__
    if (uinputFd >= 0) return true;

    uinputFd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (uinputFd < 0) {
        // Try fallback path
        uinputFd = open("/dev/input/uinput", O_WRONLY | O_NONBLOCK);
    }

    if (uinputFd < 0) {
        lastError = QString("Cannot open /dev/uinput (%1). Install scripts/99-mousy.rules or run with proper permissions.")
                        .arg(strerror(errno));
        qWarning() << "[UInputManager]" << lastError;
        initialized = false;
        return false;
    }

    // Enable relative events (mouse movement & scroll)
    ioctl(uinputFd, UI_SET_EVBIT, EV_REL);
    ioctl(uinputFd, UI_SET_RELBIT, REL_X);
    ioctl(uinputFd, UI_SET_RELBIT, REL_Y);
    ioctl(uinputFd, UI_SET_RELBIT, REL_WHEEL);
    ioctl(uinputFd, UI_SET_RELBIT, REL_HWHEEL);

    // Enable key events (keyboard keys & mouse buttons)
    ioctl(uinputFd, UI_SET_EVBIT, EV_KEY);
    ioctl(uinputFd, UI_SET_KEYBIT, BTN_LEFT);
    ioctl(uinputFd, UI_SET_KEYBIT, BTN_RIGHT);
    ioctl(uinputFd, UI_SET_KEYBIT, BTN_MIDDLE);
    ioctl(uinputFd, UI_SET_KEYBIT, BTN_SIDE);
    ioctl(uinputFd, UI_SET_KEYBIT, BTN_EXTRA);
    ioctl(uinputFd, UI_SET_KEYBIT, BTN_FORWARD);
    ioctl(uinputFd, UI_SET_KEYBIT, BTN_BACK);
    ioctl(uinputFd, UI_SET_KEYBIT, BTN_TASK);

    // Register standard keyboard keys (1 to 240)
    for (int k = 1; k < 240; ++k) {
        ioctl(uinputFd, UI_SET_KEYBIT, k);
    }

    // Device setup
    struct uinput_setup usetup;
    std::memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor  = 0x17ef; // Gaming USB vendor id
    usetup.id.product = 0x608d;
    std::strncpy(usetup.name, "Mousy Virtual Gaming Device", UINPUT_MAX_NAME_SIZE - 1);

    if (ioctl(uinputFd, UI_DEV_SETUP, &usetup) < 0) {
        // Fallback to legacy uinput_user_dev
        struct uinput_user_dev uidev;
        std::memset(&uidev, 0, sizeof(uidev));
        std::strncpy(uidev.name, "Mousy Virtual Gaming Device", UINPUT_MAX_NAME_SIZE - 1);
        uidev.id.bustype = BUS_USB;
        uidev.id.vendor  = 0x17ef;
        uidev.id.product = 0x608d;
        if (write(uinputFd, &uidev, sizeof(uidev)) < 0) {
            lastError = QString("Failed to setup uinput device: %1").arg(strerror(errno));
            close(uinputFd);
            uinputFd = -1;
            return false;
        }
    }

    if (ioctl(uinputFd, UI_DEV_CREATE) < 0) {
        lastError = QString("Failed to create uinput device: %1").arg(strerror(errno));
        close(uinputFd);
        uinputFd = -1;
        return false;
    }

    initialized = true;
    lastError.clear();
    qDebug() << "[UInputManager] Successfully registered Mousy Virtual Gaming Device.";
    return true;
#else
    lastError = "uinput is only supported on Linux.";
    return false;
#endif
}

void UInputManager::shutdown() {
#ifdef __linux__
    if (uinputFd >= 0) {
        ioctl(uinputFd, UI_DEV_DESTROY);
        close(uinputFd);
        uinputFd = -1;
    }
    initialized = false;
#endif
}

bool UInputManager::sendEvent(uint16_t type, uint16_t code, int32_t value) {
#ifdef __linux__
    if (!isAvailable()) return false;

    struct input_event ev;
    std::memset(&ev, 0, sizeof(ev));
    ev.type = type;
    ev.code = code;
    ev.value = value;

    return write(uinputFd, &ev, sizeof(ev)) == sizeof(ev);
#else
    Q_UNUSED(type); Q_UNUSED(code); Q_UNUSED(value);
    return false;
#endif
}

bool UInputManager::syn() {
#ifdef __linux__
    return sendEvent(EV_SYN, SYN_REPORT, 0);
#else
    return false;
#endif
}

bool UInputManager::emitMouseMove(int dx, int dy) {
#ifdef __linux__
    if (!isAvailable()) return false;
    bool ok = true;
    if (dx != 0) ok &= sendEvent(EV_REL, REL_X, dx);
    if (dy != 0) ok &= sendEvent(EV_REL, REL_Y, dy);
    return ok && syn();
#else
    Q_UNUSED(dx); Q_UNUSED(dy);
    return false;
#endif
}

bool UInputManager::emitMouseWheel(int steps) {
#ifdef __linux__
    if (!isAvailable()) return false;
    return sendEvent(EV_REL, REL_WHEEL, steps) && syn();
#else
    Q_UNUSED(steps);
    return false;
#endif
}

bool UInputManager::emitMouseButton(int buttonCode, bool pressed) {
#ifdef __linux__
    if (!isAvailable()) return false;
    return sendEvent(EV_KEY, static_cast<uint16_t>(buttonCode), pressed ? 1 : 0) && syn();
#else
    Q_UNUSED(buttonCode); Q_UNUSED(pressed);
    return false;
#endif
}

bool UInputManager::emitClick(int buttonCode, int holdTimeMs) {
#ifdef __linux__
    if (!emitMouseButton(buttonCode, true)) return false;
    std::this_thread::sleep_for(std::chrono::milliseconds(holdTimeMs));
    return emitMouseButton(buttonCode, false);
#else
    Q_UNUSED(buttonCode); Q_UNUSED(holdTimeMs);
    return false;
#endif
}

bool UInputManager::emitKey(int keyCode, bool pressed) {
#ifdef __linux__
    if (!isAvailable()) return false;
    return sendEvent(EV_KEY, static_cast<uint16_t>(keyCode), pressed ? 1 : 0) && syn();
#else
    Q_UNUSED(keyCode); Q_UNUSED(pressed);
    return false;
#endif
}

bool UInputManager::emitKeyPress(int keyCode, int holdTimeMs) {
#ifdef __linux__
    if (!emitKey(keyCode, true)) return false;
    std::this_thread::sleep_for(std::chrono::milliseconds(holdTimeMs));
    return emitKey(keyCode, false);
#else
    Q_UNUSED(keyCode); Q_UNUSED(holdTimeMs);
    return false;
#endif
}

bool UInputManager::emitKeyCombo(const QVector<int> &modifierKeys, int primaryKey) {
#ifdef __linux__
    if (!isAvailable()) return false;
    for (int mod : modifierKeys) {
        emitKey(mod, true);
    }
    if (primaryKey > 0) {
        emitKeyPress(primaryKey, 20);
    }
    for (int i = modifierKeys.size() - 1; i >= 0; --i) {
        emitKey(modifierKeys[i], false);
    }
    return true;
#else
    Q_UNUSED(modifierKeys); Q_UNUSED(primaryKey);
    return false;
#endif
}

bool UInputManager::emitRemappedAction(const QString &actionStr) {
    if (actionStr.trimmed().isEmpty()) return false;

    QString clean = actionStr.trimmed();

    // Check if it's a mouse button (e.g. BTN_LEFT, BTN_SIDE, Mouse 4, etc.)
    int btn = buttonCodeForName(clean);
    if (btn > 0) {
        return emitClick(btn);
    }

    // Check if it's a combo like "Ctrl+Shift+T" or "Alt+F4"
    if (clean.contains('+')) {
        QStringList parts = clean.split('+', Qt::SkipEmptyParts);
        QVector<int> mods;
        int primaryKey = 0;
        for (int i = 0; i < parts.size(); ++i) {
            QString token = parts[i].trimmed();
            int code = keyCodeForName(token);
            if (i == parts.size() - 1) {
                primaryKey = code;
            } else {
                if (code > 0) mods.append(code);
            }
        }
        return emitKeyCombo(mods, primaryKey);
    }

    // Single key press
    int code = keyCodeForName(clean);
    if (code > 0) {
        return emitKeyPress(code);
    }

    return false;
}

int UInputManager::buttonCodeForName(const QString &name) {
#ifdef __linux__
    QString upper = name.toUpper().trimmed();
    if (upper == "LMB" || upper == "BTN_LEFT" || upper == "LEFT") return BTN_LEFT;
    if (upper == "RMB" || upper == "BTN_RIGHT" || upper == "RIGHT") return BTN_RIGHT;
    if (upper == "MMB" || upper == "BTN_MIDDLE" || upper == "MIDDLE" || upper == "WHEEL") return BTN_MIDDLE;
    if (upper == "BACK" || upper == "MOUSE 4" || upper == "BTN_SIDE") return BTN_SIDE;
    if (upper == "FORWARD" || upper == "MOUSE 5" || upper == "BTN_EXTRA") return BTN_EXTRA;
#endif
    return 0;
}

int UInputManager::keyCodeForName(const QString &name) {
#ifdef __linux__
    QString u = name.toUpper().trimmed();
    if (u == "CTRL" || u == "LCTRL" || u == "CONTROL") return KEY_LEFTCTRL;
    if (u == "RCTRL") return KEY_RIGHTCTRL;
    if (u == "SHIFT" || u == "LSHIFT") return KEY_LEFTSHIFT;
    if (u == "RSHIFT") return KEY_RIGHTSHIFT;
    if (u == "ALT" || u == "LALT") return KEY_LEFTALT;
    if (u == "ALTGR" || u == "RALT") return KEY_RIGHTALT;
    if (u == "SUPER" || u == "WIN" || u == "META") return KEY_LEFTMETA;

    if (u == "ENTER" || u == "RETURN") return KEY_ENTER;
    if (u == "SPACE") return KEY_SPACE;
    if (u == "ESC" || u == "ESCAPE") return KEY_ESC;
    if (u == "TAB") return KEY_TAB;
    if (u == "BACKSPACE") return KEY_BACKSPACE;

    if (u == "F1") return KEY_F1;
    if (u == "F2") return KEY_F2;
    if (u == "F3") return KEY_F3;
    if (u == "F4") return KEY_F4;
    if (u == "F5") return KEY_F5;
    if (u == "F6") return KEY_F6;
    if (u == "F7") return KEY_F7;
    if (u == "F8") return KEY_F8;
    if (u == "F9") return KEY_F9;
    if (u == "F10") return KEY_F10;
    if (u == "F11") return KEY_F11;
    if (u == "F12") return KEY_F12;

    if (u.size() == 1) {
        char ch = u[0].toLatin1();
        if (ch >= 'A' && ch <= 'Z') {
            static const int letters[] = {
                KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I,
                KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R,
                KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z
            };
            return letters[ch - 'A'];
        }
        if (ch >= '0' && ch <= '9') {
            static const int digits[] = {
                KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9
            };
            return digits[ch - '0'];
        }
    }
#endif
    return 0;
}
