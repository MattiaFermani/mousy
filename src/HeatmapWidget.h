#ifndef HEATMAPWIDGET_H
#define HEATMAPWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QElapsedTimer>
#include <QVector>
#include <QPointF>
#include <QList>

class HeatmapWidget : public QWidget {
    Q_OBJECT

public:
    explicit HeatmapWidget(QWidget *parent = nullptr);

    enum DisplayMode { MoveHeat, ClickHeat, Combined };
    Q_ENUM(DisplayMode)

    int getMoves() const { return totalMoves; }
    int getClicks() const { return totalClicks; }
    bool exportImage(const QString &filePath);

public slots:
    void clearHeatmap();
    void setTrackingEnabled(bool enabled);
    void setDisplayMode(DisplayMode mode);
    void setTrailEnabled(bool enabled);

signals:
    void statsChanged(int moves, int clicks);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    // Heatmap grid
    int gridCols = 0;
    int gridRows = 0;
    static constexpr int CellSize = 6;
    QVector<float> moveHeat;
    QVector<float> clickHeat;

    // Trail
    struct TrailPoint {
        QPointF pos;
        float age;
    };
    QList<TrailPoint> trail;
    static constexpr int MaxTrail = 200;
    bool showTrail = true;

    // Stats
    int totalMoves = 0;
    int totalClicks = 0;
    float maxMoveHeat = 1.0f;
    float maxClickHeat = 1.0f;

    bool tracking = true;
    DisplayMode displayMode = Combined;

    // Timers
    QTimer *decayTimer;
    QTimer *trailTimer;
    void decayHeat();
    void ageTrail();

    void allocateGrid();
    QColor heatColor(float value, bool isClick) const;
};

#endif // HEATMAPWIDGET_H
