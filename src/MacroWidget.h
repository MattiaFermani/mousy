#ifndef MACROWIDGET_H
#define MACROWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QElapsedTimer>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QVector>
#include <QSpinBox>
#include <QComboBox>

struct MacroEvent {
    enum Type { MouseMove, MouseClick, KeyPress, KeyRelease, Delay };
    Type type;
    int x = 0, y = 0;
    int button = 0;  // Qt::MouseButton
    int key = 0;     // Qt::Key
    qint64 delayMs = 0;
    QString description;
};

class MacroWidget : public QWidget {
    Q_OBJECT

public:
    explicit MacroWidget(QWidget *parent = nullptr);

private slots:
    void startRecording();
    void stopRecording();
    void playMacro();
    void stopMacro();
    void clearMacro();
    void deleteMacroItem();
    void saveMacro();
    void loadMacro();

    // Presets
    void addPresetDoubleClick();
    void addPresetRapidFire();
    void addPresetDelay();
    void addPresetText();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QListWidget *eventList;
    QPushButton *recordBtn;
    QPushButton *playBtn;
    QPushButton *stopBtn;
    QPushButton *clearBtn;
    QPushButton *deleteBtn;
    QPushButton *saveBtn;
    QPushButton *loadBtn;
    QLabel *statusLabel;
    QLabel *statsLabel;

    QSpinBox *repeatSpinBox;
    QComboBox *speedComboBox;

    bool recording = false;
    bool playing = false;
    QElapsedTimer recordTimer;
    QVector<MacroEvent> events;

    QTimer *playTimer;
    int playIndex = 0;
    int currentLoop = 0;
    int targetLoops = 1;

    void addEvent(const MacroEvent &event);
    void updateStats();
    void playNextEvent();
};

#endif // MACROWIDGET_H
