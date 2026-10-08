#include "MainWindow.h"
#include "MouseViewWidget.h"
#include "HeatmapWidget.h"
#include "AimTrainerWidget.h"
#include "MacroWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>
#include <QSpinBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setWindowTitle("Mousy — Advanced Mouse Editor");
    resize(1050, 720);
    setupUi();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUi() {
    tabWidget = new QTabWidget(this);
    setCentralWidget(tabWidget);

    tabWidget->addTab(createButtonMapperTab(), "🖱  Button Mapper");
    tabWidget->addTab(createHeatmapTab(), "🔥  Heatmap");
    tabWidget->addTab(createAimTrainerTab(), "🎯  Aim Trainer");
    tabWidget->addTab(createMacroTab(), "⏺  Macros");
}

// ─── BUTTON MAPPER TAB ──────────────────────────────────────────

QWidget* MainWindow::createButtonMapperTab() {
    QWidget *widget = new QWidget();
    QHBoxLayout *mainLayout = new QHBoxLayout(widget);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Left: Interactive mouse view
    mouseView = new MouseViewWidget();
    mouseView->setMinimumWidth(440);
    mainLayout->addWidget(mouseView, 3);

    // Right: Config panel
    QWidget *configPanel = new QWidget();
    configPanel->setObjectName("ConfigPanel");
    QVBoxLayout *panelLayout = new QVBoxLayout(configPanel);
    panelLayout->setSpacing(14);
    panelLayout->setContentsMargins(10, 10, 10, 10);

    QLabel *titleLabel = new QLabel("<h2 style='color: #00d2ff; margin: 0;'>Button Mapping</h2>");
    actionLabel = new QLabel("Click a button on the mouse diagram to configure it.");
    actionLabel->setWordWrap(true);
    actionLabel->setStyleSheet("color: #999; font-size: 14px; padding: 12px; background: #181820; border-radius: 8px; border: 1px solid #282835;");

    bindingLabel = new QLabel("Current Binding: <span style='color: #888;'>None</span>");
    bindingLabel->setWordWrap(true);
    bindingLabel->setStyleSheet("color: #ccc; font-size: 13px; padding: 10px; background: #141418; border-radius: 6px; border: 1px solid #282835;");

    QPushButton *remapKeyBtn = new QPushButton("⌨  Remap to Key / Macro");
    QPushButton *resetBtn = new QPushButton("↺  Reset to Default");
    resetBtn->setStyleSheet(R"(
        QPushButton {
            background: #333340;
            color: white; border: none; padding: 10px 20px; border-radius: 8px;
            font-weight: bold; font-size: 13px;
        }
        QPushButton:hover { background: #444455; }
        QPushButton:disabled { background: #222228; color: #555; }
    )");

    remapKeyBtn->setEnabled(false);
    resetBtn->setEnabled(false);

    panelLayout->addWidget(titleLabel);
    panelLayout->addWidget(actionLabel);
    panelLayout->addWidget(bindingLabel);
    panelLayout->addWidget(remapKeyBtn);
    panelLayout->addWidget(resetBtn);

    // Hardware Profiles Group (RGB & DPI)
    QGroupBox *hardwareBox = new QGroupBox("Hardware & Lighting");
    hardwareBox->setStyleSheet(R"(
        QGroupBox {
            color: #00d2ff; font-weight: bold; font-size: 13px;
            border: 1px solid #282835; border-radius: 8px; padding-top: 18px;
            margin-top: 10px; background: #141418;
        }
        QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 4px; }
    )");
    QVBoxLayout *hwLayout = new QVBoxLayout(hardwareBox);
    hwLayout->setSpacing(10);

    // RGB Mode
    QHBoxLayout *rgbRow = new QHBoxLayout();
    QLabel *rgbLbl = new QLabel("RGB Effect:");
    rgbLbl->setStyleSheet("color: #aaa;");
    QComboBox *rgbCombo = new QComboBox();
    rgbCombo->addItems({"Neon Cyan", "Rainbow Cycle", "Cyberpunk Pink", "Matrix Green", "Crimson Fire"});
    rgbRow->addWidget(rgbLbl);
    rgbRow->addWidget(rgbCombo);
    hwLayout->addLayout(rgbRow);

    // DPI Selector
    QHBoxLayout *dpiRow = new QHBoxLayout();
    QLabel *dpiLbl = new QLabel("DPI Stage:");
    dpiLbl->setStyleSheet("color: #aaa;");
    QComboBox *dpiCombo = new QComboBox();
    dpiCombo->addItems({"400 DPI", "800 DPI", "1200 DPI", "1600 DPI", "2400 DPI", "3200 DPI", "6400 DPI"});
    dpiCombo->setCurrentIndex(3); // 1600
    dpiRow->addWidget(dpiLbl);
    dpiRow->addWidget(dpiCombo);
    hwLayout->addLayout(dpiRow);

    // Polling rate
    QHBoxLayout *pollRow = new QHBoxLayout();
    QLabel *pollLbl = new QLabel("Polling Rate:");
    pollLbl->setStyleSheet("color: #aaa;");
    QComboBox *pollCombo = new QComboBox();
    pollCombo->addItems({"125 Hz (8ms)", "500 Hz (2ms)", "1000 Hz (1ms)"});
    pollCombo->setCurrentIndex(2);
    pollRow->addWidget(pollLbl);
    pollRow->addWidget(pollCombo);
    hwLayout->addLayout(pollRow);

    panelLayout->addWidget(hardwareBox);
    panelLayout->addStretch();

    mainLayout->addWidget(configPanel, 2);

    // Connections
    connect(mouseView, &MouseViewWidget::mouseButtonClicked, this, &MainWindow::onMouseButtonClicked);
    connect(mouseView, &MouseViewWidget::mouseButtonClicked, [=](int, const QString&) {
        remapKeyBtn->setEnabled(true);
        resetBtn->setEnabled(true);
    });
    connect(remapKeyBtn, &QPushButton::clicked, this, &MainWindow::onRemapKey);
    connect(resetBtn, &QPushButton::clicked, this, &MainWindow::onResetBinding);

    connect(rgbCombo, &QComboBox::currentIndexChanged, [this](int idx) {
        if (mouseView) mouseView->setRgbEffect(static_cast<MouseViewWidget::RgbEffect>(idx));
    });

    connect(dpiCombo, &QComboBox::currentIndexChanged, [this, dpiCombo](int) {
        int dpi = dpiCombo->currentText().split(' ')[0].toInt();
        if (mouseView) mouseView->setDpi(dpi);
    });

    return widget;
}

void MainWindow::onMouseButtonClicked(int buttonId, const QString& buttonName) {
    selectedButtonId = buttonId;
    selectedButtonName = buttonName;
    actionLabel->setText(QString("<b style='color: #00d2ff;'>Selected:</b> %1<br><br>"
                                 "Click 'Remap to Key' to bind an action.").arg(buttonName));
    actionLabel->setStyleSheet("color: #ddd; font-size: 14px; padding: 12px; background: #1a1a24; border-radius: 8px; border: 1px solid #00d2ff;");

    if (buttonBindings.contains(buttonId)) {
        bindingLabel->setText(QString("Current Binding: <span style='color: #00d2ff; font-weight: bold;'>%1</span>").arg(buttonBindings[buttonId]));
    } else {
        bindingLabel->setText("Current Binding: <span style='color: #888;'>Default</span>");
    }
}

void MainWindow::onRemapKey() {
    if (selectedButtonId < 0) return;

    bool ok;
    QString key = QInputDialog::getText(this, "Remap Mouse Button",
        QString("Enter keyboard key, shortcut, or macro name for \"%1\":").arg(selectedButtonName),
        QLineEdit::Normal, buttonBindings.value(selectedButtonId, ""), &ok);

    if (ok && !key.isEmpty()) {
        buttonBindings[selectedButtonId] = key;
        bindingLabel->setText(QString("Current Binding: <span style='color: #00ff90; font-weight: bold;'>%1</span>  ✓").arg(key));
        if (mouseView) mouseView->setBindings(buttonBindings);
    }
}

void MainWindow::onResetBinding() {
    if (selectedButtonId >= 0) {
        buttonBindings.remove(selectedButtonId);
        bindingLabel->setText("Current Binding: <span style='color: #888;'>Default</span>");
        if (mouseView) mouseView->setBindings(buttonBindings);
    }
}

// ─── HEATMAP TAB ────────────────────────────────────────────────

QWidget* MainWindow::createHeatmapTab() {
    QWidget *widget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setSpacing(12);
    layout->setContentsMargins(16, 16, 16, 16);

    // Top Control Bar
    QHBoxLayout *controls = new QHBoxLayout();

    QLabel *titleLabel = new QLabel("<b style='color: #00d2ff; font-size: 16px;'>🔥 Live Movement & Click Heatmap</b>");
    controls->addWidget(titleLabel);

    controls->addSpacing(20);

    // Mode dropdown
    QLabel *modeLbl = new QLabel("View:");
    modeLbl->setStyleSheet("color: #aaa;");
    QComboBox *modeCombo = new QComboBox();
    modeCombo->addItems({"Combined (Moves + Clicks)", "Movement Heat Only", "Clicks Heat Only"});
    controls->addWidget(modeLbl);
    controls->addWidget(modeCombo);

    // Trail checkbox
    QCheckBox *trailCheck = new QCheckBox("Cursor Trail");
    trailCheck->setChecked(true);
    trailCheck->setStyleSheet("color: #ccc;");
    controls->addWidget(trailCheck);

    controls->addStretch();

    // Stats
    heatmapStatsLabel = new QLabel("Moves: 0  |  Clicks: 0");
    heatmapStatsLabel->setStyleSheet("color: #00d2ff; font-weight: bold; font-size: 13px; padding-right: 15px;");
    controls->addWidget(heatmapStatsLabel);

    // Export & Clear
    QPushButton *exportBtn = new QPushButton("📸 Export PNG");
    exportBtn->setStyleSheet(R"(
        QPushButton {
            background: #252835; color: #00d2ff; border: 1px solid #354050;
            padding: 8px 16px; border-radius: 6px; font-weight: bold;
        }
        QPushButton:hover { background: #303848; border-color: #00d2ff; }
    )");
    controls->addWidget(exportBtn);

    QPushButton *clearBtn = new QPushButton("🗑 Clear");
    clearBtn->setFixedWidth(90);
    controls->addWidget(clearBtn);

    layout->addLayout(controls);

    // Heatmap canvas
    heatmapWidget = new HeatmapWidget();
    heatmapWidget->setStyleSheet("border: 1px solid #282835; border-radius: 8px;");
    layout->addWidget(heatmapWidget, 1);

    // Connections
    connect(clearBtn, &QPushButton::clicked, heatmapWidget, &HeatmapWidget::clearHeatmap);
    connect(exportBtn, &QPushButton::clicked, this, &MainWindow::onExportHeatmap);

    connect(modeCombo, &QComboBox::currentIndexChanged, [this](int idx) {
        if (heatmapWidget) {
            heatmapWidget->setDisplayMode(static_cast<HeatmapWidget::DisplayMode>(idx));
        }
    });

    connect(trailCheck, &QCheckBox::toggled, [this](bool checked) {
        if (heatmapWidget) heatmapWidget->setTrailEnabled(checked);
    });

    connect(heatmapWidget, &HeatmapWidget::statsChanged, this, &MainWindow::onHeatmapStatsChanged);

    return widget;
}

void MainWindow::onHeatmapStatsChanged(int moves, int clicks) {
    if (heatmapStatsLabel) {
        heatmapStatsLabel->setText(QString("Moves: %1  |  Clicks: %2").arg(moves).arg(clicks));
    }
}

void MainWindow::onExportHeatmap() {
    if (!heatmapWidget) return;

    QString path = QFileDialog::getSaveFileName(this, "Export Heatmap Image", "heatmap.png", "PNG Image (*.png)");
    if (!path.isEmpty()) {
        if (heatmapWidget->exportImage(path)) {
            QMessageBox::information(this, "Export Success", "Heatmap successfully saved to:\n" + path);
        } else {
            QMessageBox::warning(this, "Export Error", "Failed to save heatmap image.");
        }
    }
}

// ─── AIM TRAINER TAB ────────────────────────────────────────────

QWidget* MainWindow::createAimTrainerTab() {
    QWidget *widget = new QWidget();
    QHBoxLayout *mainLayout = new QHBoxLayout(widget);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // Left: Game area
    aimWidget = new AimTrainerWidget();
    aimWidget->setStyleSheet("border: 1px solid #282835; border-radius: 8px;");
    mainLayout->addWidget(aimWidget, 3);

    // Right: Controls & Stats
    QWidget *sidePanel = new QWidget();
    QVBoxLayout *sideLayout = new QVBoxLayout(sidePanel);
    sideLayout->setSpacing(12);

    QLabel *title = new QLabel("<h2 style='color: #00d2ff; margin: 0;'>🎯 Aim Trainer</h2>");
    sideLayout->addWidget(title);

    // Stats group
    QGroupBox *statsBox = new QGroupBox("Live Stats");
    statsBox->setStyleSheet(R"(
        QGroupBox {
            color: #00d2ff; font-weight: bold; font-size: 13px;
            border: 1px solid #282835; border-radius: 8px; padding-top: 18px;
            margin-top: 6px; background: #141418;
        }
        QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 4px; }
    )");
    QVBoxLayout *statsLayout = new QVBoxLayout(statsBox);
    statsLayout->setSpacing(6);

    aimHitsLabel = new QLabel("Hits: 0");
    aimMissLabel = new QLabel("Misses: 0");
    aimReactionLabel = new QLabel("Avg Reaction: — ms");
    aimAccuracyLabel = new QLabel("Accuracy: — %");
    aimStreakLabel = new QLabel("Streak: 0");

    QString statStyle = "color: #ccc; font-size: 14px; padding: 3px;";
    aimHitsLabel->setStyleSheet(statStyle);
    aimMissLabel->setStyleSheet(statStyle);
    aimReactionLabel->setStyleSheet(statStyle);
    aimAccuracyLabel->setStyleSheet(statStyle);
    aimStreakLabel->setStyleSheet(statStyle);

    statsLayout->addWidget(aimHitsLabel);
    statsLayout->addWidget(aimMissLabel);
    statsLayout->addWidget(aimReactionLabel);
    statsLayout->addWidget(aimAccuracyLabel);
    statsLayout->addWidget(aimStreakLabel);

    sideLayout->addWidget(statsBox);

    // Settings group
    QGroupBox *settingsBox = new QGroupBox("Game Settings");
    settingsBox->setStyleSheet(statsBox->styleSheet());
    QVBoxLayout *settingsLayout = new QVBoxLayout(settingsBox);
    settingsLayout->setSpacing(8);

    // Game Mode
    QHBoxLayout *modeRow = new QHBoxLayout();
    QLabel *modeLbl = new QLabel("Mode:");
    modeLbl->setStyleSheet("color: #aaa;");
    QComboBox *gameModeCombo = new QComboBox();
    gameModeCombo->addItems({"Classic (Random)", "Flick (Edges)", "GridShot (Tactical 3x3)"});
    modeRow->addWidget(modeLbl);
    modeRow->addWidget(gameModeCombo);
    settingsLayout->addLayout(modeRow);

    // Target size
    QHBoxLayout *sizeRow = new QHBoxLayout();
    QLabel *sizeLabel = new QLabel("Target Size:");
    sizeLabel->setStyleSheet("color: #aaa;");
    QSlider *sizeSlider = new QSlider(Qt::Horizontal);
    sizeSlider->setRange(15, 60);
    sizeSlider->setValue(30);
    sizeSlider->setStyleSheet(R"(
        QSlider::groove:horizontal { background: #282835; height: 6px; border-radius: 3px; }
        QSlider::handle:horizontal {
            background: #00d2ff; width: 16px; height: 16px; margin: -5px 0;
            border-radius: 8px;
        }
    )");
    QLabel *sizeValue = new QLabel("30px");
    sizeValue->setStyleSheet("color: #00d2ff; font-weight: bold; min-width: 45px;");
    sizeRow->addWidget(sizeLabel);
    sizeRow->addWidget(sizeSlider);
    sizeRow->addWidget(sizeValue);
    settingsLayout->addLayout(sizeRow);

    // Spawn speed
    QHBoxLayout *speedRow = new QHBoxLayout();
    QLabel *speedLabel = new QLabel("Spawn Speed:");
    speedLabel->setStyleSheet("color: #aaa;");
    QSlider *speedSlider = new QSlider(Qt::Horizontal);
    speedSlider->setRange(400, 2500);
    speedSlider->setValue(1200);
    speedSlider->setStyleSheet(sizeSlider->styleSheet());
    QLabel *speedValue = new QLabel("1200ms");
    speedValue->setStyleSheet("color: #00d2ff; font-weight: bold; min-width: 55px;");
    speedRow->addWidget(speedLabel);
    speedRow->addWidget(speedSlider);
    speedRow->addWidget(speedValue);
    settingsLayout->addLayout(speedRow);

    sideLayout->addWidget(settingsBox);

    // Buttons
    QPushButton *startBtn = new QPushButton("▶  START");
    startBtn->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #00c853, stop:1 #009624);
            color: white; border: none; padding: 14px; border-radius: 8px;
            font-weight: bold; font-size: 16px;
        }
        QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #00e676, stop:1 #00c853); }
    )");

    QPushButton *stopBtn = new QPushButton("⏹  STOP");
    stopBtn->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #e03030, stop:1 #b02020);
            color: white; border: none; padding: 14px; border-radius: 8px;
            font-weight: bold; font-size: 16px;
        }
        QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #ff4545, stop:1 #d02525); }
    )");
    stopBtn->setEnabled(false);

    sideLayout->addWidget(startBtn);
    sideLayout->addWidget(stopBtn);
    sideLayout->addStretch();

    mainLayout->addWidget(sidePanel, 1);

    // Connections
    connect(aimWidget, &AimTrainerWidget::statsUpdated, this, &MainWindow::onAimStatsUpdated);
    connect(aimWidget, &AimTrainerWidget::gameFinished, this, &MainWindow::onAimGameFinished);

    connect(startBtn, &QPushButton::clicked, [=]() {
        aimWidget->startGame();
        startBtn->setEnabled(false);
        stopBtn->setEnabled(true);
    });

    connect(stopBtn, &QPushButton::clicked, [=]() {
        aimWidget->stopGame();
        startBtn->setEnabled(true);
        stopBtn->setEnabled(false);
    });

    connect(gameModeCombo, &QComboBox::currentIndexChanged, [this](int idx) {
        if (aimWidget) aimWidget->setGameMode(static_cast<AimTrainerWidget::GameMode>(idx));
    });

    connect(sizeSlider, &QSlider::valueChanged, [=](int val) {
        sizeValue->setText(QString("%1px").arg(val));
        aimWidget->setTargetSize(val);
    });

    connect(speedSlider, &QSlider::valueChanged, [=](int val) {
        speedValue->setText(QString("%1ms").arg(val));
        aimWidget->setSpawnInterval(val);
    });

    return widget;
}

void MainWindow::onAimStatsUpdated(int hits, int misses, double avgReaction, double accuracy) {
    if (!aimHitsLabel) return;
    aimHitsLabel->setText(QString("Hits: <span style='color: #00ff90;'>%1</span>").arg(hits));
    aimMissLabel->setText(QString("Misses: <span style='color: #ff4040;'>%1</span>").arg(misses));
    aimReactionLabel->setText(QString("Avg Reaction: <span style='color: #00d2ff;'>%1 ms</span>").arg(static_cast<int>(avgReaction)));
    aimAccuracyLabel->setText(QString("Accuracy: <span style='color: %1;'>%2%</span>")
        .arg(accuracy > 70 ? "#00ff90" : (accuracy > 40 ? "#ffaa00" : "#ff4040"))
        .arg(accuracy, 0, 'f', 1));
}

void MainWindow::onAimGameFinished(int hits, int misses, double avgReaction, double accuracy) {
    if (hits + misses == 0) return;
    QMessageBox::information(this, "Session Summary",
        QString("<h3>🎯 Session Complete!</h3>"
                "<b>Hits:</b> %1<br>"
                "<b>Misses:</b> %2<br>"
                "<b>Accuracy:</b> %3%<br>"
                "<b>Avg Reaction Time:</b> %4 ms")
            .arg(hits).arg(misses).arg(accuracy, 0, 'f', 1).arg(static_cast<int>(avgReaction)));
}

// ─── MACRO TAB ──────────────────────────────────────────────────

QWidget* MainWindow::createMacroTab() {
    return new MacroWidget();
}
