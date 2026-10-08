#include "MacroWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QApplication>
#include <QFileDialog>
#include <QFile>
#include <QDataStream>
#include <QMessageBox>

MacroWidget::MacroWidget(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Title
    QLabel *title = new QLabel("<h2 style='color: #00d2ff;'>⏺ Macro Recorder</h2>");
    mainLayout->addWidget(title);

    // Status
    statusLabel = new QLabel("Status: Idle");
    statusLabel->setStyleSheet("color: #888; font-size: 13px; padding: 8px; background: #1a1a1a; border-radius: 6px;");
    mainLayout->addWidget(statusLabel);

    // Event list
    eventList = new QListWidget();
    eventList->setStyleSheet(R"(
        QListWidget {
            background-color: #141418;
            border: 1px solid #2a2a2a;
            border-radius: 8px;
            color: #ccc;
            font-family: monospace;
            font-size: 13px;
            padding: 6px;
        }
        QListWidget::item {
            padding: 6px 10px;
            border-bottom: 1px solid #1e1e22;
            border-radius: 4px;
        }
        QListWidget::item:selected {
            background-color: #1a3a5c;
            color: #00d2ff;
        }
        QListWidget::item:hover {
            background-color: #1e1e28;
        }
    )");
    mainLayout->addWidget(eventList, 1);

    // Buttons row 1
    QHBoxLayout *btnRow1 = new QHBoxLayout();

    recordBtn = new QPushButton("⏺  Record");
    recordBtn->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #e03030, stop:1 #c02020);
            color: white; border: none; padding: 12px 20px; border-radius: 8px;
            font-weight: bold; font-size: 14px;
        }
        QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #ff4040, stop:1 #e03030); }
        QPushButton:disabled { background: #2a2a2a; color: #555; }
    )");

    playBtn = new QPushButton("▶  Play");
    playBtn->setEnabled(false);

    clearBtn = new QPushButton("🗑  Clear");

    deleteBtn = new QPushButton("✕  Delete");

    btnRow1->addWidget(recordBtn);
    btnRow1->addWidget(playBtn);
    btnRow1->addWidget(deleteBtn);
    btnRow1->addWidget(clearBtn);
    mainLayout->addLayout(btnRow1);

    // Buttons row 2
    QHBoxLayout *btnRow2 = new QHBoxLayout();

    saveBtn = new QPushButton("💾  Save Macro");
    loadBtn = new QPushButton("📂  Load Macro");

    btnRow2->addWidget(saveBtn);
    btnRow2->addWidget(loadBtn);
    mainLayout->addLayout(btnRow2);

    // Connections
    connect(recordBtn, &QPushButton::clicked, this, [this]() {
        if (recording) stopRecording(); else startRecording();
    });
    connect(playBtn, &QPushButton::clicked, this, &MacroWidget::playMacro);
    connect(clearBtn, &QPushButton::clicked, this, &MacroWidget::clearMacro);
    connect(deleteBtn, &QPushButton::clicked, this, &MacroWidget::deleteMacroItem);
    connect(saveBtn, &QPushButton::clicked, this, &MacroWidget::saveMacro);
    connect(loadBtn, &QPushButton::clicked, this, &MacroWidget::loadMacro);

    // Play timer
    playTimer = new QTimer(this);
    playTimer->setSingleShot(true);
    connect(playTimer, &QTimer::timeout, this, &MacroWidget::playNextEvent);
}

void MacroWidget::startRecording() {
    recording = true;
    events.clear();
    eventList->clear();
    recordTimer.start();
    recordBtn->setText("⏹  Stop");
    statusLabel->setText("Status: <span style='color: #ff4040;'>● RECORDING</span> — Press keys or move mouse...");
    statusLabel->setStyleSheet("color: #ff4040; font-size: 13px; padding: 8px; background: #1a1a1a; border-radius: 6px; border: 1px solid #ff4040;");
    playBtn->setEnabled(false);

    // Install global event filter
    qApp->installEventFilter(this);
}

void MacroWidget::stopRecording() {
    recording = false;
    qApp->removeEventFilter(this);
    recordBtn->setText("⏺  Record");
    statusLabel->setText(QString("Status: Recorded %1 events").arg(events.size()));
    statusLabel->setStyleSheet("color: #00d2ff; font-size: 13px; padding: 8px; background: #1a1a1a; border-radius: 6px;");
    playBtn->setEnabled(!events.isEmpty());
}

bool MacroWidget::eventFilter(QObject *obj, QEvent *event) {
    if (!recording) return false;

    qint64 elapsed = recordTimer.elapsed();

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent*>(event);
        if (ke->isAutoRepeat()) return false;

        MacroEvent me;
        me.type = MacroEvent::KeyPress;
        me.key = ke->key();
        me.delayMs = elapsed;
        me.description = QString("⌨ Key Press: %1  (+%2ms)").arg(QKeySequence(ke->key()).toString()).arg(elapsed);
        addEvent(me);
        recordTimer.restart();
        return false;
    }

    if (event->type() == QEvent::KeyRelease) {
        QKeyEvent *ke = static_cast<QKeyEvent*>(event);
        if (ke->isAutoRepeat()) return false;

        MacroEvent me;
        me.type = MacroEvent::KeyRelease;
        me.key = ke->key();
        me.delayMs = elapsed;
        me.description = QString("⌨ Key Release: %1  (+%2ms)").arg(QKeySequence(ke->key()).toString()).arg(elapsed);
        addEvent(me);
        recordTimer.restart();
        return false;
    }

    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *me_event = static_cast<QMouseEvent*>(event);
        MacroEvent me;
        me.type = MacroEvent::MouseClick;
        me.x = static_cast<int>(me_event->globalPosition().x());
        me.y = static_cast<int>(me_event->globalPosition().y());
        me.button = static_cast<int>(me_event->button());
        me.delayMs = elapsed;

        QString btnName = (me_event->button() == Qt::LeftButton) ? "Left" :
                          (me_event->button() == Qt::RightButton) ? "Right" : "Middle";
        me.description = QString("🖱 %1 Click at (%2, %3)  (+%4ms)").arg(btnName).arg(me.x).arg(me.y).arg(elapsed);
        addEvent(me);
        recordTimer.restart();
        return false;
    }

    return false;
}

void MacroWidget::addEvent(const MacroEvent &event) {
    events.append(event);
    eventList->addItem(event.description);
    eventList->scrollToBottom();
}

void MacroWidget::playMacro() {
    if (events.isEmpty() || playing) return;
    playing = true;
    playIndex = 0;
    playBtn->setEnabled(false);
    recordBtn->setEnabled(false);
    statusLabel->setText("Status: <span style='color: #00ff90;'>▶ PLAYING</span>");
    statusLabel->setStyleSheet("color: #00ff90; font-size: 13px; padding: 8px; background: #1a1a1a; border-radius: 6px; border: 1px solid #00ff90;");
    playNextEvent();
}

void MacroWidget::playNextEvent() {
    if (playIndex >= events.size()) {
        playing = false;
        playBtn->setEnabled(true);
        recordBtn->setEnabled(true);
        statusLabel->setText("Status: Playback complete");
        statusLabel->setStyleSheet("color: #888; font-size: 13px; padding: 8px; background: #1a1a1a; border-radius: 6px;");
        return;
    }

    // Highlight current event
    eventList->setCurrentRow(playIndex);

    const MacroEvent &ev = events[playIndex];
    playIndex++;

    // Schedule next event with appropriate delay
    int nextDelay = (playIndex < events.size()) ? static_cast<int>(events[playIndex].delayMs) : 100;
    playTimer->start(std::max(1, nextDelay));
}

void MacroWidget::clearMacro() {
    events.clear();
    eventList->clear();
    playBtn->setEnabled(false);
    statusLabel->setText("Status: Cleared");
    statusLabel->setStyleSheet("color: #888; font-size: 13px; padding: 8px; background: #1a1a1a; border-radius: 6px;");
}

void MacroWidget::deleteMacroItem() {
    int row = eventList->currentRow();
    if (row >= 0 && row < events.size()) {
        events.removeAt(row);
        delete eventList->takeItem(row);
        playBtn->setEnabled(!events.isEmpty());
    }
}

void MacroWidget::saveMacro() {
    if (events.isEmpty()) return;

    QString path = QFileDialog::getSaveFileName(this, "Save Macro", "", "Mousy Macro (*.mousy)");
    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Error", "Could not save file.");
        return;
    }

    QDataStream out(&file);
    out << static_cast<int>(events.size());
    for (const auto &ev : events) {
        out << static_cast<int>(ev.type) << ev.x << ev.y << ev.button << ev.key << ev.delayMs << ev.description;
    }
    file.close();
    statusLabel->setText(QString("Status: Saved %1 events").arg(events.size()));
}

void MacroWidget::loadMacro() {
    QString path = QFileDialog::getOpenFileName(this, "Load Macro", "", "Mousy Macro (*.mousy)");
    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Error", "Could not open file.");
        return;
    }

    QDataStream in(&file);
    int count;
    in >> count;

    events.clear();
    eventList->clear();

    for (int i = 0; i < count; ++i) {
        MacroEvent ev;
        int type;
        in >> type >> ev.x >> ev.y >> ev.button >> ev.key >> ev.delayMs >> ev.description;
        ev.type = static_cast<MacroEvent::Type>(type);
        events.append(ev);
        eventList->addItem(ev.description);
    }

    file.close();
    playBtn->setEnabled(!events.isEmpty());
    statusLabel->setText(QString("Status: Loaded %1 events").arg(events.size()));
}

void MacroWidget::updateUI() {
    playBtn->setEnabled(!events.isEmpty() && !playing && !recording);
    recordBtn->setEnabled(!playing);
}
