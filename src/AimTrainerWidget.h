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

    enum GameMode { Classic, Flick, Tracking, GridShot };
    Q_ENUM(GameMode)

public slots:
    void startGame();
    void stopGame();
    void setTargetSize(int size);
    void setSpawnInterval(int ms);
    void setGameMode(GameMode mode);

signals:
    void statsUpdated(int hits, int misses, double avgReaction, double accuracy);
    void gameFinished(int hits, int misses, double avgReaction, double accuracy);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    struct Target {
        QPointF center;
        float radius;
        float life;
        float maxLife;
        QColor color;
        float spawnScale; // 0..1 grow-in animation
    };

    struct Particle {
        QPointF pos;
        QPointF vel;
        QColor color;
        float life;
        float size;
    };

    struct HitText {
        QPointF pos;
        QString text;
        QColor color;
        float life;
    };

    QList<Target> targets;
    QList<Particle> particles;
    QList<HitText> hitTexts;

    QTimer *gameTimer;
    QTimer *spawnTimer;
    QElapsedTimer reactionTimer;

    bool gameRunning = false;
    GameMode gameMode = Classic;

    int hits = 0;
    int misses = 0;
    int spawned = 0;
    double totalReactionMs = 0;
    int bestStreak = 0;
    int currentStreak = 0;

    int targetSize = 30;
    int spawnIntervalMs = 1200;

    // Grid shot
    int gridShotIndex = 0;

    // Tracking
    QPointF trackingTarget;
    QPointF trackingCurrent;
    float trackingScore = 0;
    int trackingFrames = 0;

    // Countdown
    int countdown = 0;
    QTimer *countdownTimer;

    void spawnTarget();
    void gameTick();
    void emitStats();
    void spawnParticles(QPointF pos, QColor color, int count);
    void addHitText(QPointF pos, const QString &text, QColor color);

    float pulsePhase = 0;
};

#endif // AIMTRAINERWIDGET_H
