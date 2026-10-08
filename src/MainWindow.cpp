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
#include <QGroupBox>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setWindowTitle("Mousy — Advanced Mouse Editor");
    resize(1000, 700);
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
    MouseViewWidget *mouseView = new MouseViewWidget();
    mouseView->setMinimumWidth(420);
    mainLayout->addWidget(mouseView, 2);

    // Right: Config panel
    QWidget *configPanel = new QWidget();
    configPanel->setObjectName("ConfigPanel");
    QVBoxLayout *panelLayout = new QVBoxLayout(configPanel);
    panelLayout->setSpacing(14);

    QLabel *titleLabel = new QLabel("<h2 style='color: #00d2ff;'>Button Configuration</h2>");
    actionLabel = new QLabel("Click a button on the mouse to configure it.");
    actionLabel->setWordWrap(true);
    actionLabel->setStyleSheet("color: #999; font-size: 14px; padding: 12px; background: #1a1a1a; border-radius: 8px;");

    bindingLabel = new QLabel("Current Binding: <span style='color: #888;'>None</span>");
    bindingLabel->setWordWrap(true);
    bindingLabel->setStyleSheet("color: #ccc; font-size: 13px; padding: 10px; background: #141418; border-radius: 6px; border: 1px solid #2a2a2a;");

    QPushButton *remapKeyBtn = new QPushButton("⌨  Remap to Key");
    QPushButton *resetBtn = new QPushButton("↺  Reset to Default");
    resetBtn->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #555, stop:1 #444);
            color: white; border: none; padding: 12px 24px; border-radius: 8px;
            font-weight: bold; font-size: 14px;
        }
        QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #777, stop:1 #666); }
        QPushButton:disabled { background: #2a2a2a; color: #555; }
    )");

    remapKeyBtn->setEnabled(false);
    resetBtn->setEnabled(false);

    panelLayout->addWidget(titleLabel);
    panelLayout->addWidget(actionLabel);
    panelLayout->addWidget(bindingLabel);
    panelLayout->addSpacing(10);
    panelLayout->addWidget(remapKeyBtn);
    panelLayout->addWidget(resetBtn);
    panelLayout->addStretch();

    mainLayout->addWidget(configPanel, 1);

    // Connections
    connect(mouseView, &MouseViewWidget::mouseButtonClicked, this, &MainWindow::onMouseButtonClicked);
    connect(mouseView, &MouseViewWidget::mouseButtonClicked, [=](int, const QString&) {
        remapKeyBtn->setEnabled(true);
        resetBtn->setEnabled(true);
    });
    connect(remapKeyBtn, &QPushButton::clicked, this, &MainWindow::onRemapKey);
    connect(resetBtn, &QPushButton::clicked, [this]() {
        if (selectedButtonId >= 0) {
            buttonBindings.remove(selectedButtonId);
            bindingLabel->setText("Current Binding: <span style='color: #888;'>Default</span>");
        }
    });

    return widget;
}

void MainWindow::onMouseButtonClicked(int buttonId, const QString& buttonName) {
    selectedButtonId = buttonId;
    selectedButtonName = buttonName;
    actionLabel->setText(QString("<b style='color: #00d2ff;'>Selected:</b> %1<br><br>"
                                 "Choose an action below to configure this button.").arg(buttonName));
    actionLabel->setStyleSheet("color: #ddd; font-size: 14px; padding: 12px; background: #1a1a1a; border-radius: 8px; border: 1px solid #00d2ff;");

    if (buttonBindings.contains(buttonId)) {
        bindingLabel->setText(QString("Current Binding: <span style='color: #00d2ff;'>%1</span>").arg(buttonBindings[buttonId]));
    } else {
        bindingLabel->setText("Current Binding: <span style='color: #888;'>Default</span>");
    }
}

void MainWindow::onRemapKey() {
    if (selectedButtonId < 0) return;

    QString key = QInputDialog::getText(this, "Remap Key",
        QString("Enter the key or shortcut to bind to \"%1\":").arg(selectedButtonName));

    if (!key.isEmpty()) {
        buttonBindings[selectedButtonId] = key;
        bindingLabel->setText(QString("Current Binding: <span style='color: #00ff90;'>%1</span>  ✓").arg(key));
    }
}

// ─── HEATMAP TAB ────────────────────────────────────────────────

QWidget* MainWindow::createHeatmapTab() {
    QWidget *widget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setSpacing(12);
    layout->setContentsMargins(16, 16, 16, 16);

    // Control bar
    QHBoxLayout *controls = new QHBoxLayout();

    QLabel *titleLabel = new QLabel("<b style='color: #00d2ff; font-size: 16px;'>🔥 Live Heatmap</b>");
    controls->addWidget(titleLabel);
    controls->addStretch();

    QPushButton *clearBtn = new QPushButton("Clear");
    clearBtn->setFixedWidth(100);
    controls->addWidget(clearBtn);

    layout->addLayout(controls);

    // Heatmap canvas
    HeatmapWidget *heatmap = new HeatmapWidget();
    heatmap->setStyleSheet("border: 1px solid #2a2a2a; border-radius: 8px;");
    layout->addWidget(heatmap, 1);

    // Connections
    connect(clearBtn, &QPushButton::clicked, heatmap, &HeatmapWidget::clearHeatmap);

    return widget;
}

// ─── AIM TRAINER TAB ────────────────────────────────────────────

QWidget* MainWindow::createAimTrainerTab() {
    QWidget *widget = new QWidget();
    QHBoxLayout *mainLayout = new QHBoxLayout(widget);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // Left: Game area
    AimTrainerWidget *aimWidget = new AimTrainerWidget();
    aimWidget->setStyleSheet("border: 1px solid #2a2a2a; border-radius: 8px;");
    mainLayout->addWidget(aimWidget, 3);

    // Right: Controls & Stats
    QWidget *sidePanel = new QWidget();
    QVBoxLayout *sideLayout = new QVBoxLayout(sidePanel);
    sideLayout->setSpacing(14);

    QLabel *title = new QLabel("<h2 style='color: #00d2ff;'>🎯 Aim Trainer</h2>");
    sideLayout->addWidget(title);

    // Stats group
    QGroupBox *statsBox = new QGroupBox("Live Stats");
    statsBox->setStyleSheet(R"(
        QGroupBox {
            color: #00d2ff; font-weight: bold; font-size: 14px;
            border: 1px solid #2a2a2a; border-radius: 8px; padding-top: 20px;
            margin-top: 10px; background: #141418;
        }
        QGroupBox::title { subcontrol-origin: margin; left: 14px; padding: 0 6px; }
    )");
    QVBoxLayout *statsLayout = new QVBoxLayout(statsBox);
    statsLayout->setSpacing(8);

    aimHitsLabel = new QLabel("Hits: 0");
    aimMissLabel = new QLabel("Misses: 0");
    aimReactionLabel = new QLabel("Avg Reaction: — ms");
    aimAccuracyLabel = new QLabel("Accuracy: — %");

    QString statStyle = "color: #ccc; font-size: 15px; padding: 4px;";
    aimHitsLabel->setStyleSheet(statStyle);
    aimMissLabel->setStyleSheet(statStyle);
    aimReactionLabel->setStyleSheet(statStyle);
    aimAccuracyLabel->setStyleSheet(statStyle);

    statsLayout->addWidget(aimHitsLabel);
    statsLayout->addWidget(aimMissLabel);
    statsLayout->addWidget(aimReactionLabel);
    statsLayout->addWidget(aimAccuracyLabel);

    sideLayout->addWidget(statsBox);

    // Settings group
    QGroupBox *settingsBox = new QGroupBox("Settings");
    settingsBox->setStyleSheet(R"(
        QGroupBox {
            color: #aaa; font-weight: bold; font-size: 13px;
            border: 1px solid #2a2a2a; border-radius: 8px; padding-top: 20px;
            margin-top: 10px; background: #141418;
        }
        QGroupBox::title { subcontrol-origin: margin; left: 14px; padding: 0 6px; }
    )");
    QVBoxLayout *settingsLayout = new QVBoxLayout(settingsBox);

    // Target size
    QHBoxLayout *sizeRow = new QHBoxLayout();
    QLabel *sizeLabel = new QLabel("Target Size:");
    sizeLabel->setStyleSheet("color: #aaa;");
    QSlider *sizeSlider = new QSlider(Qt::Horizontal);
    sizeSlider->setRange(15, 60);
    sizeSlider->setValue(30);
    sizeSlider->setStyleSheet(R"(
        QSlider::groove:horizontal { background: #2a2a2a; height: 6px; border-radius: 3px; }
        QSlider::handle:horizontal {
            background: #00d2ff; width: 16px; height: 16px; margin: -5px 0;
            border-radius: 8px;
        }
    )");
    QLabel *sizeValue = new QLabel("30");
    sizeValue->setStyleSheet("color: #00d2ff; font-weight: bold; min-width: 30px;");
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
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #e03030, stop:1 #c02020);
            color: white; border: none; padding: 14px; border-radius: 8px;
            font-weight: bold; font-size: 16px;
        }
        QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #ff4040, stop:1 #e03030); }
    )");
    stopBtn->setEnabled(false);

    sideLayout->addWidget(startBtn);
    sideLayout->addWidget(stopBtn);
    sideLayout->addStretch();

    mainLayout->addWidget(sidePanel, 1);

    // Connections
    connect(aimWidget, &AimTrainerWidget::statsUpdated, this, &MainWindow::onAimStatsUpdated);

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

    connect(sizeSlider, &QSlider::valueChanged, [=](int val) {
        sizeValue->setText(QString::number(val));
        aimWidget->setTargetSize(val);
    });

    connect(speedSlider, &QSlider::valueChanged, [=](int val) {
        speedValue->setText(QString("%1ms").arg(val));
        aimWidget->setSpawnInterval(val);
    });

    return widget;
}

void MainWindow::onAimStatsUpdated(int hits, int misses, double avgReaction, double accuracy) {
    aimHitsLabel->setText(QString("Hits: <span style='color: #00ff90;'>%1</span>").arg(hits));
    aimMissLabel->setText(QString("Misses: <span style='color: #ff4040;'>%1</span>").arg(misses));
    aimReactionLabel->setText(QString("Avg Reaction: <span style='color: #00d2ff;'>%1 ms</span>").arg(static_cast<int>(avgReaction)));
    aimAccuracyLabel->setText(QString("Accuracy: <span style='color: %1;'>%2%</span>")
        .arg(accuracy > 70 ? "#00ff90" : (accuracy > 40 ? "#ffaa00" : "#ff4040"))
        .arg(accuracy, 0, 'f', 1));
}

// ─── MACRO TAB ──────────────────────────────────────────────────

QWidget* MainWindow::createMacroTab() {
    return new MacroWidget();
}
