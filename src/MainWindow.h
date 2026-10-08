#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QLabel>
#include <QMap>
#include <QString>
#include <QComboBox>

class MouseViewWidget;
class HeatmapWidget;
class AimTrainerWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onMouseButtonClicked(int buttonId, const QString& buttonName);
    void onRemapKey();
    void onResetBinding();
    void onAimStatsUpdated(int hits, int misses, double avgReaction, double accuracy);
    void onAimGameFinished(int hits, int misses, double avgReaction, double accuracy);
    void onHeatmapStatsChanged(int moves, int clicks);
    void onExportHeatmap();

private:
    QTabWidget *tabWidget;
    QLabel *actionLabel;
    QLabel *bindingLabel;
    int selectedButtonId = -1;
    QString selectedButtonName;

    // Current key bindings per button ID
    QMap<int, QString> buttonBindings;

    // Core widgets
    MouseViewWidget *mouseView = nullptr;
    HeatmapWidget *heatmapWidget = nullptr;
    AimTrainerWidget *aimWidget = nullptr;

    // Heatmap stats
    QLabel *heatmapStatsLabel = nullptr;

    // Aim trainer stats
    QLabel *aimHitsLabel = nullptr;
    QLabel *aimMissLabel = nullptr;
    QLabel *aimReactionLabel = nullptr;
    QLabel *aimAccuracyLabel = nullptr;
    QLabel *aimStreakLabel = nullptr;

    void setupUi();
    QWidget* createButtonMapperTab();
    QWidget* createHeatmapTab();
    QWidget* createAimTrainerTab();
    QWidget* createMacroTab();
};

#endif // MAINWINDOW_H
