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
#include <QInputDialog>
#include <QGroupBox>

MacroWidget::MacroWidget(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Header with status & duration stats
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *title = new QLabel("<h2 style='color: #00d2ff; margin: 0;'>⏺ Macro Studio</h2>");
    headerLayout->addWidget(title);
    headerLayout->addStretch();

    statsLabel = new QLabel("Events: 0  |  Duration: 0ms");
    statsLabel->setStyleSheet("color: #888; font-size: 13px; font-weight: bold;");
    headerLayout->addWidget(statsLabel);
    mainLayout->addLayout(headerLayout);

    // Status banner
    statusLabel = new QLabel("Status: Idle — Ready to record or play");
    statusLabel->setStyleSheet("color: #888; font-size: 13px; padding: 8px 12px; background: #181820; border-radius: 6px; border: 1px solid #2a2a35;");
    mainLayout->addWidget(statusLabel);

    // Main content: Events list on left, Controls on right
    QHBoxLayout *bodyLayout = new QHBoxLayout();

    // Event list
    eventList = new QListWidget();
    eventList->setStyleSheet(R"(
        QListWidget {
            background-color: #121216;
            border: 1px solid #282832;
            border-radius: 8px;
            color: #ddd;
            font-family: monospace;
            font-size: 13px;
            padding: 6px;
        }
        QListWidget::item {
            padding: 8px 12px;
            border-bottom: 1px solid #1a1a20;
            border-radius: 4px;
        }
        QListWidget::item:selected {
            background-color: #183858;
            color: #00d2ff;
        }
        QListWidget::item:hover {
            background-color: #1c1c24;
        }
    )");
    bodyLayout->addWidget(eventList, 3);

    // Right side panel: Quick Presets & Playback Config
    QWidget *rightPanel = new QWidget();
    QVBoxLayout *panelLayout = new QVBoxLayout(rightPanel);
    panelLayout->setSpacing(10);
    panelLayout->setContentsMargins(0, 0, 0, 0);

    // Playback Settings Group
    QGroupBox *configBox = new QGroupBox("Playback Settings");
    configBox->setStyleSheet(R"(
        QGroupBox {
            color: #00d2ff; font-weight: bold; font-size: 13px;
            border: 1px solid #2a2a35; border-radius: 8px; padding-top: 18px;
            margin-top: 8px; background: #16161c;
        }
        QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 4px; }
    )");
    QVBoxLayout *configLayout = new QVBoxLayout(configBox);

    QHBoxLayout *repeatRow = new QHBoxLayout();
    QLabel *repeatLbl = new QLabel("Repeat Count:");
    repeatLbl->setStyleSheet("color: #bbb;");
    repeatSpinBox = new QSpinBox();
    repeatSpinBox->setRange(1, 100);
    repeatSpinBox->setValue(1);
    repeatSpinBox->setSuffix("x");
    repeatSpinBox->setStyleSheet("background: #202028; color: #fff; padding: 4px; border-radius: 4px;");
    repeatRow->addWidget(repeatLbl);
    repeatRow->addWidget(repeatSpinBox);
    configLayout->addLayout(repeatRow);

    QHBoxLayout *speedRow = new QHBoxLayout();
    QLabel *speedLbl = new QLabel("Speed Multiplier:");
    speedLbl->setStyleSheet("color: #bbb;");
    speedComboBox = new QComboBox();
    speedComboBox->addItems({"0.5x (Slow)", "1.0x (Normal)", "2.0x (Fast)", "4.0x (Instant)"});
    speedComboBox->setCurrentIndex(1);
    speedComboBox->setStyleSheet("background: #202028; color: #fff; padding: 4px; border-radius: 4px;");
    speedRow->addWidget(speedLbl);
    speedRow->addWidget(speedComboBox);
    configLayout->addLayout(speedRow);

    panelLayout->addWidget(configBox);

    // Quick Presets Group
    QGroupBox *presetBox = new QGroupBox("Quick Presets");
    presetBox->setStyleSheet(configBox->styleSheet());
    QVBoxLayout *presetLayout = new QVBoxLayout(presetBox);

    QPushButton *presetDoubleBtn = new QPushButton("⚡ Double Click");
    QPushButton *presetRapidBtn = new QPushButton("🔥 Rapid Fire (5x)");
    QPushButton *presetDelayBtn = new QPushButton("⏱ Add Delay (+100ms)");
    QPushButton *presetTextBtn = new QPushButton("⌨ Type Text...");

    QString presetStyle = R"(
        QPushButton {
            background-color: #242430; color: #eee; border: 1px solid #333344;
            padding: 8px 12px; border-radius: 6px; font-weight: normal; font-size: 13px;
        }
        QPushButton:hover { background-color: #303042; border-color: #00d2ff; color: #00d2ff; }
    )";
    presetDoubleBtn->setStyleSheet(presetStyle);
    presetRapidBtn->setStyleSheet(presetStyle);
    presetDelayBtn->setStyleSheet(presetStyle);
    presetTextBtn->setStyleSheet(presetStyle);

    presetLayout->addWidget(presetDoubleBtn);
    presetLayout->addWidget(presetRapidBtn);
    presetLayout->addWidget(presetDelayBtn);
    presetLayout->addWidget(presetTextBtn);

    panelLayout->addWidget(presetBox);
    panelLayout->addStretch();
    bodyLayout->addWidget(rightPanel, 2);

    mainLayout->addLayout(bodyLayout, 1);

    // Bottom Action Buttons
    QHBoxLayout *btnRow = new QHBoxLayout();

    recordBtn = new QPushButton("⏺  Record");
    recordBtn->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #e03030, stop:1 #b02020);
            color: white; border: none; padding: 12px 20px; border-radius: 8px;
            font-weight: bold; font-size: 14px;
        }
        QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #ff4545, stop:1 #d02525); }
        QPushButton:disabled { background: #2a2a2a; color: #555; }
    )");

    playBtn = new QPushButton("▶  Play");
    playBtn->setEnabled(false);

    stopBtn = new QPushButton("⏹  Stop");
    stopBtn->setEnabled(false);
    stopBtn->setStyleSheet(R"(
        QPushButton {
            background: #444; color: white; border: none; padding: 12px 20px; border-radius: 8px;
            font-weight: bold; font-size: 14px;
        }
        QPushButton:hover { background: #555; }
        QPushButton:disabled { background: #222; color: #555; }
    )");

    deleteBtn = new QPushButton("✕  Delete Item");
    clearBtn = new QPushButton("🗑  Clear All");
    saveBtn = new QPushButton("💾  Save Macro");
    loadBtn = new QPushButton("📂  Load Macro");

    btnRow->addWidget(recordBtn);
    btnRow->addWidget(playBtn);
    btnRow->addWidget(stopBtn);
    btnRow->addWidget(deleteBtn);
    btnRow->addWidget(clearBtn);
    btnRow->addWidget(saveBtn);
    btnRow->addWidget(loadBtn);
    mainLayout->addLayout(btnRow);

    // Connections
    connect(recordBtn, &QPushButton::clicked, this, [this]() {
        if (recording) stopRecording(); else startRecording();
    });
    connect(playBtn, &QPushButton::clicked, this, &MacroWidget::playMacro);
    connect(stopBtn, &QPushButton::clicked, this, &MacroWidget::stopMacro);
    connect(clearBtn, &QPushButton::clicked, this, &MacroWidget::clearMacro);
    connect(deleteBtn, &QPushButton::clicked, this, &MacroWidget::deleteMacroItem);
    connect(saveBtn, &QPushButton::clicked, this, &MacroWidget::saveMacro);
    connect(loadBtn, &QPushButton::clicked, this, &MacroWidget::loadMacro);

    // Presets
    connect(presetDoubleBtn, &QPushButton::clicked, this, &MacroWidget::addPresetDoubleClick);
    connect(presetRapidBtn, &QPushButton::clicked, this, &MacroWidget::addPresetRapidFire);
    connect(presetDelayBtn, &QPushButton::clicked, this, &MacroWidget::addPresetDelay);
    connect(presetTextBtn, &QPushButton::clicked, this, &MacroWidget::addPresetText);

    // Playback timer
    playTimer = new QTimer(this);
    playTimer->setSingleShot(true);
    connect(playTimer, &QTimer::timeout, this, &MacroWidget::playNextEvent);
}

void MacroWidget::startRecording() {
    recording = true;
    events.clear();
    eventList->clear();
    recordTimer.start();
    recordBtn->setText("⏹  Stop Recording");
    statusLabel->setText("Status: <span style='color: #ff4040; font-weight: bold;'>● RECORDING</span> — Press keys or click mouse to capture events...");
    statusLabel->setStyleSheet("color: #ff4040; font-size: 13px; padding: 8px 12px; background: #2a1418; border-radius: 6px; border: 1px solid #ff4040;");
    playBtn->setEnabled(false);
    stopBtn->setEnabled(false);

    qApp->installEventFilter(this);
}

void MacroWidget::stopRecording() {
    recording = false;
    qApp->removeEventFilter(this);
    recordBtn->setText("⏺  Record");
    statusLabel->setText(QString("Status: Recorded %1 events successfully").arg(events.size()));
    statusLabel->setStyleSheet("color: #00d2ff; font-size: 13px; padding: 8px 12px; background: #141c24; border-radius: 6px; border: 1px solid #00d2ff;");
    playBtn->setEnabled(!events.isEmpty());
    updateStats();
}

bool MacroWidget::eventFilter(QObject *obj, QEvent *event) {
    Q_UNUSED(obj);
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
    updateStats();
}

void MacroWidget::updateStats() {
    qint64 totalMs = 0;
    for (const auto &ev : events) totalMs += ev.delayMs;
    statsLabel->setText(QString("Events: %1  |  Total Duration: %2ms").arg(events.size()).arg(totalMs));
    playBtn->setEnabled(!events.isEmpty() && !playing && !recording);
}

void MacroWidget::playMacro() {
    if (events.isEmpty() || playing) return;
    playing = true;
    playIndex = 0;
    currentLoop = 1;
    targetLoops = repeatSpinBox->value();

    playBtn->setEnabled(false);
    recordBtn->setEnabled(false);
    stopBtn->setEnabled(true);

    statusLabel->setText(QString("Status: <span style='color: #00ff90; font-weight: bold;'>▶ PLAYING</span> (Loop %1 of %2)")
        .arg(currentLoop).arg(targetLoops));
    statusLabel->setStyleSheet("color: #00ff90; font-size: 13px; padding: 8px 12px; background: #14281c; border-radius: 6px; border: 1px solid #00ff90;");
    playNextEvent();
}

void MacroWidget::stopMacro() {
    if (!playing) return;
    playing = false;
    playTimer->stop();
    playBtn->setEnabled(true);
    recordBtn->setEnabled(true);
    stopBtn->setEnabled(false);
    statusLabel->setText("Status: Playback stopped by user");
    statusLabel->setStyleSheet("color: #888; font-size: 13px; padding: 8px 12px; background: #181820; border-radius: 6px;");
}

void MacroWidget::playNextEvent() {
    if (!playing) return;

    if (playIndex >= events.size()) {
        if (currentLoop < targetLoops) {
            currentLoop++;
            playIndex = 0;
            statusLabel->setText(QString("Status: <span style='color: #00ff90; font-weight: bold;'>▶ PLAYING</span> (Loop %1 of %2)")
                .arg(currentLoop).arg(targetLoops));
        } else {
            playing = false;
            playBtn->setEnabled(true);
            recordBtn->setEnabled(true);
            stopBtn->setEnabled(false);
            statusLabel->setText(QString("Status: Playback complete (%1 loops finished)").arg(targetLoops));
            statusLabel->setStyleSheet("color: #888; font-size: 13px; padding: 8px 12px; background: #181820; border-radius: 6px;");
            return;
        }
    }

    eventList->setCurrentRow(playIndex);
    const MacroEvent &ev = events[playIndex];
    playIndex++;

    // Calculate speed factor
    float speedMultiplier = 1.0f;
    switch (speedComboBox->currentIndex()) {
        case 0: speedMultiplier = 0.5f; break;
        case 1: speedMultiplier = 1.0f; break;
        case 2: speedMultiplier = 2.0f; break;
        case 3: speedMultiplier = 4.0f; break;
    }

    int nextDelay = (playIndex < events.size()) ? static_cast<int>(events[playIndex].delayMs / speedMultiplier) : 50;
    playTimer->start(std::max(1, nextDelay));
}

void MacroWidget::clearMacro() {
    events.clear();
    eventList->clear();
    updateStats();
    statusLabel->setText("Status: Macro cleared");
    statusLabel->setStyleSheet("color: #888; font-size: 13px; padding: 8px 12px; background: #181820; border-radius: 6px;");
}

void MacroWidget::deleteMacroItem() {
    int row = eventList->currentRow();
    if (row >= 0 && row < events.size()) {
        events.removeAt(row);
        delete eventList->takeItem(row);
        updateStats();
    }
}

void MacroWidget::saveMacro() {
    if (events.isEmpty()) return;

    QString path = QFileDialog::getSaveFileName(this, "Save Macro", "", "Mousy Macro (*.mousy)");
    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Error", "Could not save macro file.");
        return;
    }

    QDataStream out(&file);
    out << static_cast<int>(events.size());
    for (const auto &ev : events) {
        out << static_cast<int>(ev.type) << ev.x << ev.y << ev.button << ev.key << ev.delayMs << ev.description;
    }
    file.close();
    statusLabel->setText(QString("Status: Saved %1 events to %2").arg(events.size()).arg(QFileInfo(path).fileName()));
}

void MacroWidget::loadMacro() {
    QString path = QFileDialog::getOpenFileName(this, "Load Macro", "", "Mousy Macro (*.mousy)");
    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Error", "Could not open macro file.");
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
    updateStats();
    statusLabel->setText(QString("Status: Loaded %1 events from %2").arg(events.size()).arg(QFileInfo(path).fileName()));
}

// Presets
void MacroWidget::addPresetDoubleClick() {
    MacroEvent e1;
    e1.type = MacroEvent::MouseClick;
    e1.button = Qt::LeftButton;
    e1.delayMs = 0;
    e1.description = "🖱 Left Click (+0ms)";
    addEvent(e1);

    MacroEvent e2;
    e2.type = MacroEvent::MouseClick;
    e2.button = Qt::LeftButton;
    e2.delayMs = 45;
    e2.description = "🖱 Left Click (+45ms)";
    addEvent(e2);
}

void MacroWidget::addPresetRapidFire() {
    for (int i = 0; i < 5; ++i) {
        MacroEvent e;
        e.type = MacroEvent::MouseClick;
        e.button = Qt::LeftButton;
        e.delayMs = (i == 0) ? 0 : 35;
        e.description = QString("🖱 Left Click (+%1ms)").arg(e.delayMs);
        addEvent(e);
    }
}

void MacroWidget::addPresetDelay() {
    MacroEvent e;
    e.type = MacroEvent::Delay;
    e.delayMs = 100;
    e.description = "⏱ Wait 100ms (+100ms)";
    addEvent(e);
}

void MacroWidget::addPresetText() {
    bool ok;
    QString text = QInputDialog::getText(this, "Type Text Preset", "Enter text sequence to convert to keypresses:", QLineEdit::Normal, "GG WP", &ok);
    if (!ok || text.isEmpty()) return;

    for (int i = 0; i < text.length(); ++i) {
        QChar ch = text[i];
        int key = ch.toUpper().unicode();

        MacroEvent kp;
        kp.type = MacroEvent::KeyPress;
        kp.key = key;
        kp.delayMs = (i == 0) ? 0 : 40;
        kp.description = QString("⌨ Key Press: %1 (+%2ms)").arg(ch).arg(kp.delayMs);
        addEvent(kp);

        MacroEvent kr;
        kr.type = MacroEvent::KeyRelease;
        kr.key = key;
        kr.delayMs = 25;
        kr.description = QString("⌨ Key Release: %1 (+25ms)").arg(ch);
        addEvent(kr);
    }
}
