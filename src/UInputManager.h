#ifndef UINPUTMANAGER_H
#define UINPUTMANAGER_H

#include <QString>
#include <QVector>
#include <QObject>

class UInputManager : public QObject {
    Q_OBJECT

public:
    static UInputManager& instance();

    bool initialize();
    void shutdown();

    bool isAvailable() const;
    QString statusMessage() const;

    // Virtual mouse actions
    bool emitMouseMove(int dx, int dy);
    bool emitMouseWheel(int steps);
    bool emitMouseButton(int buttonCode, bool pressed);
    bool emitClick(int buttonCode, int holdTimeMs = 15);

    // Virtual keyboard actions
    bool emitKey(int keyCode, bool pressed);
    bool emitKeyPress(int keyCode, int holdTimeMs = 15);
    bool emitKeyCombo(const QVector<int> &modifierKeys, int primaryKey);

    // Human-readable remapping helper (e.g., "Ctrl+Shift+T" or "F5" or "BTN_EXTRA")
    bool emitRemappedAction(const QString &actionStr);

    // Standard Linux button constants for UI reference
    static int buttonCodeForName(const QString &name);
    static int keyCodeForName(const QString &name);

private:
    UInputManager();
    ~UInputManager();
    UInputManager(const UInputManager&) = delete;
    UInputManager& operator=(const UInputManager&) = delete;

    int uinputFd = -1;
    bool initialized = false;
    QString lastError;

    bool sendEvent(uint16_t type, uint16_t code, int32_t value);
    bool syn();
};

#endif // UINPUTMANAGER_H
