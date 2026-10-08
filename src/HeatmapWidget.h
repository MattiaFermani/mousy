#ifndef HEATMAPWIDGET_H
#define HEATMAPWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QElapsedTimer>
#include <QVector>
#include <QPointF>

class HeatmapWidget : public QWidget {
    Q_OBJECT

public:
    explicit HeatmapWidget(QWidget *parent = nullptr);

public slots:
    void clearHeatmap();
    void setTrackingEnabled(bool enabled);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    // Heatmap grid
    int gridCols = 0;
    int gridRows = 0;
    static constexpr int CellSize = 8;
    QVector<float> moveHeat;   // movement density
    QVector<float> clickHeat;  // click density

    // Stats
    int totalMoves = 0;
    int totalClicks = 0;
    float maxMoveHeat = 1.0f;
    float maxClickHeat = 1.0f;

    bool tracking = true;

    // Decay timer
    QTimer *decayTimer;
    void decayHeat();

    // Display mode
    enum DisplayMode { MoveHeat, ClickHeat, Combined };
    DisplayMode displayMode = Combined;

    void allocateGrid();
    QColor heatColor(float value, bool isClick) const;

    friend class HeatmapTab;
};

#endif // HEATMAPWIDGET_H
