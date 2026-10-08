#ifndef AIMTRAINERWIDGET_H
#define AIMTRAINERWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QElapsedTimer>
#include <QList>
#include <QPointF>
#include <QRandomGenerator>

class AimTrainerWidget : public QWidget {
    Q_OBJECT

public:
    explicit AimTrainerWidget(QWidget *parent = nullptr);

public slots:
    void startGame();
    void stopGame();
    void setTargetSize(int size);
    void setSpawnInterval(int ms);

signals:
    void statsUpdated(int hits, int misses, double avgReaction, double accuracy);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    struct Target {
        QPointF center;
        float radius;
        float life;      // 0..1, fades out
        float maxLife;
        QColor color;
    };

    QList<Target> targets;
    QTimer *gameTimer;
    QTimer *spawnTimer;
    QElapsedTimer reactionTimer;

    bool gameRunning = false;
    int hits = 0;
    int misses = 0;
    int spawned = 0;
    double totalReactionMs = 0;

    int targetSize = 30;
    int spawnIntervalMs = 1200;

    void spawnTarget();
    void gameTick();
    void emitStats();

    // Visual
    float pulsePhase = 0;
};

#endif // AIMTRAINERWIDGET_H
