#include "MainWindow.h"
#include "MouseViewWidget.h"
#include "HeatmapWidget.h"
#include "AimTrainerWidget.h"
#include "MacroWidget.h"
#include "DiagnosticsWidget.h"
#include "ProfileManager.h"
#include "UInputManager.h"
#include "AudioFx.h"
#include "Version.h"

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
#include <QColorDialog>
#include <QDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setWindowTitle(QString("Mousy %1 — Ultra-Modern Mouse Suite").arg(MousyVersion::VersionString));
    resize(1180, 780);

    // Initialize configuration
    ProfileManager::instance().load();

    setupUi();

    // Load active profile into UI
    applyProfileToUi(ProfileManager::instance().activeProfileName());
}

MainWindow::~MainWindow() {
    onSaveProfile();
}

void MainWindow::setupUi() {
    QWidget *centralContainer = new QWidget(this);
    QVBoxLayout *rootLayout = new QVBoxLayout(centralContainer);
    rootLayout->setContentsMargins(12, 12, 12, 12);
    rootLayout->setSpacing(8);

    // Top Profile & System Toolbar
    rootLayout->addWidget(createTopProfileBar());

    // Main Tab Widget
    tabWidget = new QTabWidget(this);
    rootLayout->addWidget(tabWidget, 1);
    setCentralWidget(centralContainer);

    tabWidget->addTab(createButtonMapperTab(), "🖱  Button Mapper");
    tabWidget->addTab(createHeatmapTab(), "🔥  Heatmap");
    tabWidget->addTab(createAimTrainerTab(), "🎯  Aim Trainer");
    tabWidget->addTab(createMacroTab(), "⏺  Macros");
    tabWidget->addTab(createDiagnosticsTab(), "📊  Diagnostics");
}

QWidget* MainWindow::createTopProfileBar() {
    QWidget *bar = new QWidget(this);
    bar->setStyleSheet(R"(
        QWidget {
            background: #141720;
            border: 1px solid #232c3d;
            border-radius: 8px;
        }
    )");

    QHBoxLayout *layout = new QHBoxLayout(bar);
    layout->setContentsMargins(14, 8, 14, 8);
    layout->setSpacing(12);

    // Logo & Brand
    QLabel *brand = new QLabel("<span style='font-size: 16px; font-weight: 900; color: #00d2ff;'>MOUSY</span> "
                               "<span style='font-size: 11px; color: #667799; font-weight: bold;'>STUDIO</span>");
    layout->addWidget(brand);

    layout->addSpacing(16);

    // Profile selector
    QLabel *profLbl = new QLabel("Profile:");
    profLbl->setStyleSheet("color: #8899aa; font-weight: bold; font-size: 12px;");
    layout->addWidget(profLbl);

    profileCombo = new QComboBox();
    profileCombo->setMinimumWidth(160);
    profileCombo->setStyleSheet(R"(
        QComboBox {
            background: #1a2232; color: #00d2ff; border: 1px solid #2a3c54;
            padding: 5px 10px; border-radius: 6px; font-weight: bold; font-size: 12px;
        }
        QComboBox:hover { border-color: #00d2ff; }
        QComboBox::drop-down { border: none; }
    )");
    layout->addWidget(profileCombo);

    QPushButton *newProfBtn = new QPushButton("＋ New");
    newProfBtn->setStyleSheet(R"(
        QPushButton {
            background: #1f2d42; color: #00ffaa; border: 1px solid #2f4565;
            padding: 5px 12px; border-radius: 6px; font-weight: bold; font-size: 12px;
        }
        QPushButton:hover { background: #283a54; border-color: #00ffaa; }
    )");
    connect(newProfBtn, &QPushButton::clicked, this, &MainWindow::onCreateProfile);
    layout->addWidget(newProfBtn);

    QPushButton *saveProfBtn = new QPushButton("💾 Save");
    saveProfBtn->setStyleSheet(R"(
        QPushButton {
            background: #1f2d42; color: #00d2ff; border: 1px solid #2f4565;
            padding: 5px 12px; border-radius: 6px; font-weight: bold; font-size: 12px;
        }
        QPushButton:hover { background: #283a54; border-color: #00d2ff; }
    )");
    connect(saveProfBtn, &QPushButton::clicked, this, &MainWindow::onSaveProfile);
    layout->addWidget(saveProfBtn);

    QPushButton *delProfBtn = new QPushButton("🗑 Delete");
    delProfBtn->setStyleSheet(R"(
        QPushButton {
            background: #2d1a22; color: #ff5577; border: 1px solid #4a2835;
            padding: 5px 12px; border-radius: 6px; font-weight: bold; font-size: 12px;
        }
        QPushButton:hover { background: #3d202d; border-color: #ff5577; }
    )");
    connect(delProfBtn, &QPushButton::clicked, this, &MainWindow::onDeleteProfile);
    layout->addWidget(delProfBtn);

    layout->addStretch();

    // Linux uinput status badge
    uinputStatusLabel = new QLabel();
    if (UInputManager::instance().isAvailable()) {
        uinputStatusLabel->setText("⚡ Virtual uinput: Active");
        uinputStatusLabel->setStyleSheet("color: #00ff90; font-weight: bold; font-size: 11px; padding: 4px 8px; background: #0f281e; border: 1px solid #1a5c3d; border-radius: 5px;");
    } else {
        uinputStatusLabel->setText("⚠ Simulated Mode (No uinput)");
        uinputStatusLabel->setToolTip("Install scripts/99-mousy.rules to enable system-wide hardware remapping without sudo.");
        uinputStatusLabel->setStyleSheet("color: #ffaa00; font-weight: bold; font-size: 11px; padding: 4px 8px; background: #2b220d; border: 1px solid #5a4415; border-radius: 5px;");
    }
    layout->addWidget(uinputStatusLabel);

    // Version & About button
    QPushButton *versionBtn = new QPushButton(QString("ℹ %1").arg(MousyVersion::VersionString));
    versionBtn->setCursor(Qt::PointingHandCursor);
    versionBtn->setToolTip("View Semantic Version breakdown & release details");
    versionBtn->setStyleSheet(R"(
        QPushButton {
            background: #182230; color: #00d2ff; border: 1px solid #283848;
            padding: 5px 12px; border-radius: 6px; font-size: 12px; font-weight: bold;
        }
        QPushButton:hover { background: #223245; border-color: #00d2ff; }
    )");
    connect(versionBtn, &QPushButton::clicked, this, &MainWindow::showAboutDialog);
    layout->addWidget(versionBtn);

    updateProfileList();
    connect(profileCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onProfileChanged);

    return bar;
}

void MainWindow::updateProfileList() {
    if (!profileCombo) return;
    profileCombo->blockSignals(true);
    profileCombo->clear();
    QStringList names = ProfileManager::instance().profileNames();
    profileCombo->addItems(names);
    int idx = names.indexOf(ProfileManager::instance().activeProfileName());
    if (idx >= 0) profileCombo->setCurrentIndex(idx);
    profileCombo->blockSignals(false);
}

void MainWindow::applyProfileToUi(const QString &profileName) {
    ProfileManager::instance().setActiveProfile(profileName);
    const MouseProfile &prof = ProfileManager::instance().activeProfile();

    buttonBindings = prof.buttonBindings;
    if (mouseView) {
        mouseView->setBindings(buttonBindings);
        mouseView->setDpi(prof.dpi);
        mouseView->setRgbEffect(static_cast<MouseViewWidget::RgbEffect>(prof.rgbEffect));
        for (auto it = prof.zoneColors.begin(); it != prof.zoneColors.end(); ++it) {
            mouseView->setCustomZoneColor(it.key(), it.value());
        }
    }

    if (rgbCombo) {
        rgbCombo->blockSignals(true);
        rgbCombo->setCurrentIndex(prof.rgbEffect);
        rgbCombo->blockSignals(false);
    }

    if (dpiCombo) {
        dpiCombo->blockSignals(true);
        for (int i = 0; i < dpiCombo->count(); ++i) {
            if (dpiCombo->itemText(i).startsWith(QString::number(prof.dpi))) {
                dpiCombo->setCurrentIndex(i);
                break;
            }
        }
        dpiCombo->blockSignals(false);
    }

    if (pollCombo) {
        pollCombo->blockSignals(true);
        for (int i = 0; i < pollCombo->count(); ++i) {
            if (pollCombo->itemText(i).startsWith(QString::number(prof.pollingRate))) {
                pollCombo->setCurrentIndex(i);
                break;
            }
        }
        pollCombo->blockSignals(false);
    }

    if (diagWidget) {
        diagWidget->setDpi(prof.dpi);
    }
}

void MainWindow::onProfileChanged(int index) {
    if (index < 0 || !profileCombo) return;
    QString name = profileCombo->itemText(index);
    applyProfileToUi(name);
}

void MainWindow::onCreateProfile() {
    bool ok;
    QString name = QInputDialog::getText(this, "Create New Profile",
                                         "Enter a name for the new profile:",
                                         QLineEdit::Normal, "", &ok);
    if (ok && !name.trimmed().isEmpty()) {
        name = name.trimmed();
        if (ProfileManager::instance().createProfile(name, ProfileManager::instance().activeProfile())) {
            updateProfileList();
            int idx = profileCombo->findText(name);
            if (idx >= 0) profileCombo->setCurrentIndex(idx);
            QMessageBox::information(this, "Profile Created", QString("Profile \"%1\" created successfully!").arg(name));
        } else {
            QMessageBox::warning(this, "Error", "A profile with that name already exists.");
        }
    }
}

void MainWindow::onDeleteProfile() {
    QString current = ProfileManager::instance().activeProfileName();
    if (current == "Default") {
        QMessageBox::warning(this, "Cannot Delete", "The \"Default\" profile cannot be deleted.");
        return;
    }

    auto reply = QMessageBox::question(this, "Delete Profile",
                                       QString("Are you sure you want to delete profile \"%1\"?").arg(current),
                                       QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        ProfileManager::instance().deleteProfile(current);
        updateProfileList();
        applyProfileToUi(ProfileManager::instance().activeProfileName());
    }
}

void MainWindow::onSaveProfile() {
    MouseProfile &prof = ProfileManager::instance().activeProfile();
    prof.buttonBindings = buttonBindings;
    if (dpiCombo) {
        prof.dpi = dpiCombo->currentText().split(' ')[0].toInt();
    }
    if (pollCombo) {
        prof.pollingRate = pollCombo->currentText().split(' ')[0].toInt();
    }
    if (rgbCombo) {
        prof.rgbEffect = rgbCombo->currentIndex();
    }
    if (mouseView) {
        for (int z = 0; z < 5; ++z) {
            prof.zoneColors[z] = mouseView->customZoneColor(z);
        }
    }
    ProfileManager::instance().save();
}

// ─── BUTTON MAPPER TAB ──────────────────────────────────────────

QWidget* MainWindow::createButtonMapperTab() {
    QWidget *widget = new QWidget();
    QHBoxLayout *mainLayout = new QHBoxLayout(widget);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(14, 14, 14, 14);

    // Left: Interactive 3D mouse view
    mouseView = new MouseViewWidget();
    mouseView->setMinimumWidth(460);
    mainLayout->addWidget(mouseView, 3);

    // Right: Config panel
    QWidget *configPanel = new QWidget();
    configPanel->setObjectName("ConfigPanel");
    QVBoxLayout *panelLayout = new QVBoxLayout(configPanel);
    panelLayout->setSpacing(12);
    panelLayout->setContentsMargins(10, 10, 10, 10);

    QLabel *titleLabel = new QLabel("<h2 style='color: #00d2ff; margin: 0;'>Button Mapping & Hardware</h2>");
    actionLabel = new QLabel("Click a button on the 3D mouse model to configure it.");
    actionLabel->setWordWrap(true);
    actionLabel->setStyleSheet("color: #999; font-size: 13px; padding: 10px; background: #181820; border-radius: 8px; border: 1px solid #282835;");

    bindingLabel = new QLabel("Current Binding: <span style='color: #888;'>None</span>");
    bindingLabel->setWordWrap(true);
    bindingLabel->setStyleSheet("color: #ccc; font-size: 13px; padding: 10px; background: #141418; border-radius: 6px; border: 1px solid #282835;");

    QHBoxLayout *remapBtnRow = new QHBoxLayout();
    QPushButton *remapKeyBtn = new QPushButton("⌨  Remap to Key / Macro");
    QPushButton *testVirtualBtn = new QPushButton("⚡ Test Remap");
    QPushButton *resetBtn = new QPushButton("↺  Reset");

    remapKeyBtn->setStyleSheet(R"(
        QPushButton {
            background: #1f364d; color: #00d2ff; border: 1px solid #2d557a;
            padding: 8px 14px; border-radius: 6px; font-weight: bold; font-size: 12px;
        }
        QPushButton:hover { background: #264666; }
        QPushButton:disabled { background: #1a1a24; color: #444; border-color: #222; }
    )");

    testVirtualBtn->setStyleSheet(R"(
        QPushButton {
            background: #1f3d32; color: #00ffaa; border: 1px solid #2d6650;
            padding: 8px 12px; border-radius: 6px; font-weight: bold; font-size: 12px;
        }
        QPushButton:hover { background: #265442; }
        QPushButton:disabled { background: #1a1a24; color: #444; border-color: #222; }
    )");

    resetBtn->setStyleSheet(R"(
        QPushButton {
            background: #333340; color: white; border: none;
            padding: 8px 14px; border-radius: 6px; font-weight: bold; font-size: 12px;
        }
        QPushButton:hover { background: #444455; }
        QPushButton:disabled { background: #222228; color: #555; }
    )");

    remapKeyBtn->setEnabled(false);
    testVirtualBtn->setEnabled(false);
    resetBtn->setEnabled(false);

    remapBtnRow->addWidget(remapKeyBtn, 2);
    remapBtnRow->addWidget(testVirtualBtn, 1);
    remapBtnRow->addWidget(resetBtn, 1);

    panelLayout->addWidget(titleLabel);
    panelLayout->addWidget(actionLabel);
    panelLayout->addWidget(bindingLabel);
    panelLayout->addLayout(remapBtnRow);

    // Hardware Profiles Group (RGB & DPI)
    QGroupBox *hardwareBox = new QGroupBox("Lighting & Performance");
    hardwareBox->setStyleSheet(R"(
        QGroupBox {
            color: #00d2ff; font-weight: bold; font-size: 12px;
            border: 1px solid #282835; border-radius: 8px; padding-top: 16px;
            margin-top: 6px; background: #141418;
        }
        QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 4px; }
    )");
    QVBoxLayout *hwLayout = new QVBoxLayout(hardwareBox);
    hwLayout->setSpacing(8);

    // RGB Mode
    QHBoxLayout *rgbRow = new QHBoxLayout();
    QLabel *rgbLbl = new QLabel("RGB Effect:");
    rgbLbl->setStyleSheet("color: #aaa;");
    rgbCombo = new QComboBox();
    rgbCombo->addItems({"Neon Cyan", "Rainbow Cycle", "Cyberpunk Pink", "Matrix Green", "Crimson Fire", "Custom Zone Colors"});
    rgbRow->addWidget(rgbLbl);
    rgbRow->addWidget(rgbCombo);
    hwLayout->addLayout(rgbRow);

    // Custom Zone Color Buttons
    QHBoxLayout *zoneColorsRow = new QHBoxLayout();
    QLabel *zoneLbl = new QLabel("Zone RGB:");
    zoneLbl->setStyleSheet("color: #aaa;");
    zoneColorsRow->addWidget(zoneLbl);

    QStringList zoneNames = {"Body", "LMB", "RMB", "Wheel", "Glow"};
    for (int z = 0; z < 5; ++z) {
        QPushButton *btn = new QPushButton(zoneNames[z]);
        btn->setStyleSheet("padding: 4px; font-size: 10px; background: #222633; color: #00d2ff; border-radius: 4px;");
        connect(btn, &QPushButton::clicked, [this, z]() { onPickZoneColor(z); });
        zoneColorsRow->addWidget(btn);
    }
    hwLayout->addLayout(zoneColorsRow);

    // DPI Selector
    QHBoxLayout *dpiRow = new QHBoxLayout();
    QLabel *dpiLbl = new QLabel("DPI Stage:");
    dpiLbl->setStyleSheet("color: #aaa;");
    dpiCombo = new QComboBox();
    dpiCombo->addItems({"400 DPI", "800 DPI", "1200 DPI", "1600 DPI", "2400 DPI", "3200 DPI", "6400 DPI", "16000 DPI"});
    dpiCombo->setCurrentIndex(3); // 1600
    dpiRow->addWidget(dpiLbl);
    dpiRow->addWidget(dpiCombo);
    hwLayout->addLayout(dpiRow);

    // Polling rate
    QHBoxLayout *pollRow = new QHBoxLayout();
    QLabel *pollLbl = new QLabel("Polling Rate:");
    pollLbl->setStyleSheet("color: #aaa;");
    pollCombo = new QComboBox();
    pollCombo->addItems({"125 Hz (8ms)", "500 Hz (2ms)", "1000 Hz (1ms)", "2000 Hz (0.5ms)", "4000 Hz (0.25ms)", "8000 Hz (0.125ms)"});
    pollCombo->setCurrentIndex(2);
    pollRow->addWidget(pollLbl);
    pollRow->addWidget(pollCombo);
    hwLayout->addLayout(pollRow);

    panelLayout->addWidget(hardwareBox);

    // 3D Model & Mesh Management Group
    QGroupBox *meshBox = new QGroupBox("3D Model & Mesh Management");
    meshBox->setStyleSheet(hardwareBox->styleSheet());
    QVBoxLayout *meshLayout = new QVBoxLayout(meshBox);
    meshLayout->setSpacing(8);

    QHBoxLayout *meshBtnRow = new QHBoxLayout();
    QPushButton *importMeshBtn = new QPushButton("📂 Import OBJ/STL");
    QPushButton *exportMeshBtn = new QPushButton("💾 Export OBJ");
    QPushButton *resetMeshBtn = new QPushButton("↺ Reset Mesh");

    QString meshBtnStyle = R"(
        QPushButton {
            background: #1e2636; color: #00d2ff; border: 1px solid #2d3f58;
            padding: 6px 10px; border-radius: 6px; font-weight: bold; font-size: 11px;
        }
        QPushButton:hover { background: #26354c; border-color: #00d2ff; }
    )";
    importMeshBtn->setStyleSheet(meshBtnStyle);
    exportMeshBtn->setStyleSheet(meshBtnStyle);
    resetMeshBtn->setStyleSheet(meshBtnStyle);

    meshBtnRow->addWidget(importMeshBtn);
    meshBtnRow->addWidget(exportMeshBtn);
    meshBtnRow->addWidget(resetMeshBtn);
    meshLayout->addLayout(meshBtnRow);

    panelLayout->addWidget(meshBox);

    // 3D Viewport Controls Group
    QGroupBox *viewBox = new QGroupBox("3D Camera & Orbit");
    viewBox->setStyleSheet(hardwareBox->styleSheet());
    QVBoxLayout *viewLayout = new QVBoxLayout(viewBox);
    viewLayout->setSpacing(8);

    QHBoxLayout *presetRow = new QHBoxLayout();
    QLabel *viewLbl = new QLabel("Angle:");
    viewLbl->setStyleSheet("color: #aaa;");
    QComboBox *viewCombo = new QComboBox();
    viewCombo->addItems({"Isometric 3D", "Top View", "Left Flank (Side)", "Right Flank", "Front View"});
    presetRow->addWidget(viewLbl);
    presetRow->addWidget(viewCombo);

    QCheckBox *autoRotCheck = new QCheckBox("Auto-Rotate Orbit");
    autoRotCheck->setStyleSheet("color: #00d2ff; font-weight: bold; font-size: 11px;");
    presetRow->addWidget(autoRotCheck);
    viewLayout->addLayout(presetRow);

    // Zoom slider
    QHBoxLayout *zoomRow = new QHBoxLayout();
    QLabel *zoomLbl = new QLabel("Zoom:");
    zoomLbl->setStyleSheet("color: #aaa;");
    QSlider *zoomSlider = new QSlider(Qt::Horizontal);
    zoomSlider->setRange(50, 250);
    zoomSlider->setValue(135);
    zoomSlider->setStyleSheet(R"(
        QSlider::groove:horizontal { background: #282835; height: 6px; border-radius: 3px; }
        QSlider::handle:horizontal {
            background: #00d2ff; width: 14px; height: 14px; margin: -4px 0;
            border-radius: 7px;
        }
    )");
    QLabel *zoomValLbl = new QLabel("135%");
    zoomValLbl->setStyleSheet("color: #00d2ff; font-weight: bold; min-width: 40px;");
    zoomRow->addWidget(zoomLbl);
    zoomRow->addWidget(zoomSlider);
    zoomRow->addWidget(zoomValLbl);
    viewLayout->addLayout(zoomRow);

    panelLayout->addWidget(viewBox);
    panelLayout->addStretch();

    mainLayout->addWidget(configPanel, 2);

    // Connections
    connect(mouseView, &MouseViewWidget::mouseButtonClicked, this, &MainWindow::onMouseButtonClicked);
    connect(mouseView, &MouseViewWidget::mouseButtonClicked, [=](int, const QString&) {
        remapKeyBtn->setEnabled(true);
        testVirtualBtn->setEnabled(true);
        resetBtn->setEnabled(true);
    });

    connect(remapKeyBtn, &QPushButton::clicked, this, &MainWindow::onRemapKey);
    connect(testVirtualBtn, &QPushButton::clicked, this, &MainWindow::onTestVirtualRemap);
    connect(resetBtn, &QPushButton::clicked, this, &MainWindow::onResetBinding);

    connect(importMeshBtn, &QPushButton::clicked, this, &MainWindow::onLoadCustomMesh);
    connect(exportMeshBtn, &QPushButton::clicked, this, &MainWindow::onExportCurrentMesh);
    connect(resetMeshBtn, &QPushButton::clicked, this, &MainWindow::onResetMesh);

    connect(rgbCombo, &QComboBox::currentIndexChanged, [this](int idx) {
        if (mouseView) mouseView->setRgbEffect(static_cast<MouseViewWidget::RgbEffect>(idx));
        onSaveProfile();
    });

    connect(dpiCombo, &QComboBox::currentIndexChanged, [this](int) {
        int dpi = dpiCombo->currentText().split(' ')[0].toInt();
        if (mouseView) mouseView->setDpi(dpi);
        if (diagWidget) diagWidget->setDpi(dpi);
        onSaveProfile();
    });

    connect(pollCombo, &QComboBox::currentIndexChanged, [this](int) {
        onSaveProfile();
    });

    connect(viewCombo, &QComboBox::currentIndexChanged, [this](int idx) {
        if (mouseView) mouseView->setPreset(static_cast<MouseViewWidget::ViewPreset>(idx));
    });

    connect(autoRotCheck, &QCheckBox::toggled, [this](bool checked) {
        if (mouseView) mouseView->setAutoRotate(checked);
    });

    connect(zoomSlider, &QSlider::valueChanged, [this, zoomValLbl](int val) {
        zoomValLbl->setText(QString("%1%").arg(val));
        if (mouseView) mouseView->setZoom(val / 100.0f);
    });

    connect(mouseView, &MouseViewWidget::viewChanged, [zoomSlider, zoomValLbl](float, float, float z) {
        int pct = static_cast<int>(z * 100);
        zoomSlider->blockSignals(true);
        zoomSlider->setValue(pct);
        zoomSlider->blockSignals(false);
        zoomValLbl->setText(QString("%1%").arg(pct));
    });

    return widget;
}

void MainWindow::onMouseButtonClicked(int buttonId, const QString& buttonName) {
    selectedButtonId = buttonId;
    selectedButtonName = buttonName;
    actionLabel->setText(QString("<b style='color: #00d2ff;'>Selected:</b> %1<br><br>"
                                 "Click 'Remap to Key' to bind an action.").arg(buttonName));
    actionLabel->setStyleSheet("color: #ddd; font-size: 13px; padding: 10px; background: #1a1a24; border-radius: 8px; border: 1px solid #00d2ff;");

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
        QString("Enter keyboard key, shortcut (e.g. Ctrl+Shift+T), or macro name for \"%1\":").arg(selectedButtonName),
        QLineEdit::Normal, buttonBindings.value(selectedButtonId, ""), &ok);

    if (ok && !key.isEmpty()) {
        buttonBindings[selectedButtonId] = key;
        bindingLabel->setText(QString("Current Binding: <span style='color: #00ff90; font-weight: bold;'>%1</span>  ✓").arg(key));
        if (mouseView) mouseView->setBindings(buttonBindings);
        onSaveProfile();
    }
}

void MainWindow::onTestVirtualRemap() {
    if (selectedButtonId < 0) return;
    QString act = buttonBindings.value(selectedButtonId, "");
    if (act.isEmpty()) {
        QMessageBox::information(this, "No Remap Set", "This button is currently mapped to default. Set a remap first!");
        return;
    }

    if (!UInputManager::instance().isAvailable()) {
        QMessageBox::warning(this, "Virtual Device Inactive",
                             "Linux uinput virtual device is not available.\n\n"
                             "To enable system-wide hardware input:\n"
                             "1. sudo cp scripts/99-mousy.rules /etc/udev/rules.d/\n"
                             "2. sudo udevadm control --reload-rules && sudo udevadm trigger\n"
                             "3. sudo modprobe uinput");
        return;
    }

    bool success = UInputManager::instance().emitRemappedAction(act);
    if (success) {
        QMessageBox::information(this, "Remap Emitted", QString("Successfully triggered virtual action: %1 via /dev/uinput!").arg(act));
    } else {
        QMessageBox::warning(this, "Emulation Error", QString("Failed to translate action: %1").arg(act));
    }
}

void MainWindow::onResetBinding() {
    if (selectedButtonId >= 0) {
        buttonBindings.remove(selectedButtonId);
        bindingLabel->setText("Current Binding: <span style='color: #888;'>Default</span>");
        if (mouseView) mouseView->setBindings(buttonBindings);
        onSaveProfile();
    }
}

void MainWindow::onPickZoneColor(int zoneId) {
    if (!mouseView) return;
    QColor cur = mouseView->customZoneColor(zoneId);
    QColor picked = QColorDialog::getColor(cur, this, QString("Select Color for Zone %1").arg(zoneId));
    if (picked.isValid()) {
        mouseView->setCustomZoneColor(zoneId, picked);
        mouseView->setRgbEffect(MouseViewWidget::CustomColor);
        if (rgbCombo) rgbCombo->setCurrentIndex(MouseViewWidget::CustomColor);
        onSaveProfile();
    }
}

void MainWindow::onLoadCustomMesh() {
    if (!mouseView) return;
    QString path = QFileDialog::getOpenFileName(this, "Load 3D Mouse Mesh", "", "3D Mesh Files (*.obj *.stl)");
    if (!path.isEmpty()) {
        QString err;
        if (mouseView->loadCustomMesh(path, &err)) {
            QMessageBox::information(this, "3D Mesh Loaded", QString("Successfully imported custom 3D mesh:\n%1").arg(path));
        } else {
            QMessageBox::warning(this, "Load Failed", QString("Could not load mesh:\n%1").arg(err));
        }
    }
}

void MainWindow::onExportCurrentMesh() {
    if (!mouseView) return;
    QString path = QFileDialog::getSaveFileName(this, "Export 3D Mesh to OBJ", "mouse_model.obj", "Wavefront OBJ (*.obj)");
    if (!path.isEmpty()) {
        QString err;
        if (mouseView->exportCurrentMesh(path, &err)) {
            QMessageBox::information(this, "Export Succeeded", QString("Saved 3D mesh to:\n%1").arg(path));
        } else {
            QMessageBox::warning(this, "Export Failed", QString("Failed to export mesh:\n%1").arg(err));
        }
    }
}

void MainWindow::onResetMesh() {
    if (!mouseView) return;
    mouseView->resetToDefaultMesh();
    QMessageBox::information(this, "Mesh Reset", "Restored default high-polygon gaming mouse 3D geometry.");
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

    // Sound effects toggle
    QCheckBox *soundCheck = new QCheckBox("Synthesized Sound FX");
    soundCheck->setChecked(true);
    soundCheck->setStyleSheet("color: #00d2ff; font-weight: bold;");
    connect(soundCheck, &QCheckBox::toggled, [](bool checked) {
        AudioFx::instance().setEnabled(checked);
    });
    settingsLayout->addWidget(soundCheck);

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
    const MouseProfile &prof = ProfileManager::instance().activeProfile();
    QMessageBox::information(this, "Session Summary & Records",
        QString("<h3>🎯 Session Complete!</h3>"
                "<b>Hits:</b> %1<br>"
                "<b>Misses:</b> %2<br>"
                "<b>Accuracy:</b> %3%<br>"
                "<b>Avg Reaction Time:</b> %4 ms<br><br>"
                "<hr><br>"
                "<b>Active Profile:</b> %5<br>"
                "<b>Personal Best Accuracy:</b> %6%")
            .arg(hits).arg(misses).arg(accuracy, 0, 'f', 1).arg(static_cast<int>(avgReaction))
            .arg(prof.name).arg(prof.bestAccuracy.value("Classic", 0.0), 0, 'f', 1));
}

// ─── MACRO TAB ──────────────────────────────────────────────────

QWidget* MainWindow::createMacroTab() {
    return new MacroWidget();
}

// ─── DIAGNOSTICS TAB ────────────────────────────────────────────

QWidget* MainWindow::createDiagnosticsTab() {
    diagWidget = new DiagnosticsWidget(this);
    if (dpiCombo) {
        diagWidget->setDpi(dpiCombo->currentText().split(' ')[0].toInt());
    }
    return diagWidget;
}

void MainWindow::showAboutDialog() {
    QDialog dialog(this);
    dialog.setWindowTitle("About Mousy — Version & SemVer Breakdown");
    dialog.setMinimumWidth(540);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->setSpacing(16);
    layout->setContentsMargins(24, 24, 24, 24);

    QLabel *content = new QLabel(MousyVersion::semVerBreakdown());
    content->setWordWrap(true);
    content->setOpenExternalLinks(true);
    layout->addWidget(content);

    QPushButton *closeBtn = new QPushButton("Close");
    closeBtn->setFixedWidth(100);
    closeBtn->setStyleSheet("padding: 8px 16px; font-weight: bold;");
    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    layout->addWidget(closeBtn, 0, Qt::AlignRight);

    dialog.exec();
}
