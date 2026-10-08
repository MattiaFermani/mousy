#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QLabel>
#include <QMap>
#include <QString>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onMouseButtonClicked(int buttonId, const QString& buttonName);
    void onRemapKey();
    void onAimStatsUpdated(int hits, int misses, double avgReaction, double accuracy);

private:
    QTabWidget *tabWidget;
    QLabel *actionLabel;
    QLabel *bindingLabel;
    int selectedButtonId = -1;
    QString selectedButtonName;

    // Current key bindings per button ID
    QMap<int, QString> buttonBindings;

    void setupUi();
    QWidget* createButtonMapperTab();
    QWidget* createHeatmapTab();
    QWidget* createAimTrainerTab();
    QWidget* createMacroTab();

    // Aim trainer stats labels
    QLabel *aimHitsLabel = nullptr;
    QLabel *aimMissLabel = nullptr;
    QLabel *aimReactionLabel = nullptr;
    QLabel *aimAccuracyLabel = nullptr;
};

#endif // MAINWINDOW_H
