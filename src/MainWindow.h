#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QLabel>
#include <QMap>
#include <QString>
#include <QComboBox>
#include <QPushButton>

class MouseViewWidget;
class HeatmapWidget;
class AimTrainerWidget;
class DiagnosticsWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onMouseButtonClicked(int buttonId, const QString& buttonName);
    void onRemapKey();
    void onResetBinding();
    void onTestVirtualRemap();
    void onAimStatsUpdated(int hits, int misses, double avgReaction, double accuracy);
    void onAimGameFinished(int hits, int misses, double avgReaction, double accuracy);
    void onHeatmapStatsChanged(int moves, int clicks);
    void onExportHeatmap();
    void showAboutDialog();

    // Profile slots
    void onProfileChanged(int index);
    void onCreateProfile();
    void onDeleteProfile();
    void onSaveProfile();

    // 3D Model & RGB zone slots
    void onLoadCustomMesh();
    void onExportCurrentMesh();
    void onResetMesh();
    void onPickZoneColor(int zoneId);

private:
    QTabWidget *tabWidget = nullptr;
    QLabel *actionLabel = nullptr;
    QLabel *bindingLabel = nullptr;
    QLabel *uinputStatusLabel = nullptr;
    int selectedButtonId = -1;
    QString selectedButtonName;

    // Current key bindings per button ID
    QMap<int, QString> buttonBindings;

    // Core widgets
    MouseViewWidget *mouseView = nullptr;
    HeatmapWidget *heatmapWidget = nullptr;
    AimTrainerWidget *aimWidget = nullptr;
    DiagnosticsWidget *diagWidget = nullptr;

    // Profile management UI
    QComboBox *profileCombo = nullptr;
    QComboBox *rgbCombo = nullptr;
    QComboBox *dpiCombo = nullptr;
    QComboBox *pollCombo = nullptr;

    // Heatmap stats
    QLabel *heatmapStatsLabel = nullptr;

    // Aim trainer stats
    QLabel *aimHitsLabel = nullptr;
    QLabel *aimMissLabel = nullptr;
    QLabel *aimReactionLabel = nullptr;
    QLabel *aimAccuracyLabel = nullptr;
    QLabel *aimStreakLabel = nullptr;

    void setupUi();
    QWidget* createTopProfileBar();
    QWidget* createButtonMapperTab();
    QWidget* createHeatmapTab();
    QWidget* createAimTrainerTab();
    QWidget* createMacroTab();
    QWidget* createDiagnosticsTab();

    void applyProfileToUi(const QString &profileName);
    void updateProfileList();
};

#endif // MAINWINDOW_H
