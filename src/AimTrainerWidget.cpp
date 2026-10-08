#include "AimTrainerWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QRadialGradient>
#include <cmath>

AimTrainerWidget::AimTrainerWidget(QWidget *parent) : QWidget(parent) {
    setMinimumSize(400, 300);
    setCursor(Qt::CrossCursor);

    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &AimTrainerWidget::gameTick);

    spawnTimer = new QTimer(this);
    connect(spawnTimer, &QTimer::timeout, this, &AimTrainerWidget::spawnTarget);
}

void AimTrainerWidget::startGame() {
    targets.clear();
    hits = 0;
    misses = 0;
    spawned = 0;
    totalReactionMs = 0;
    pulsePhase = 0;
    gameRunning = true;

    gameTimer->start(16); // ~60fps
    spawnTimer->start(spawnIntervalMs);
    reactionTimer.start();
    emitStats();
    update();
}

void AimTrainerWidget::stopGame() {
    gameRunning = false;
    gameTimer->stop();
    spawnTimer->stop();
    targets.clear();
    emitStats();
    update();
}

void AimTrainerWidget::setTargetSize(int size) {
    targetSize = size;
}

void AimTrainerWidget::setSpawnInterval(int ms) {
    spawnIntervalMs = ms;
    if (gameRunning) spawnTimer->setInterval(ms);
}

void AimTrainerWidget::spawnTarget() {
    if (!gameRunning) return;

    int margin = targetSize + 20;
    float x = QRandomGenerator::global()->bounded(margin, std::max(margin + 1, width() - margin));
    float y = QRandomGenerator::global()->bounded(margin, std::max(margin + 1, height() - margin));

    // Random color from a neon palette
    QList<QColor> palette = {
        QColor(0, 210, 255),   // Cyan
        QColor(255, 60, 120),  // Pink
        QColor(0, 255, 150),   // Mint
        QColor(255, 200, 0),   // Gold
        QColor(180, 60, 255),  // Purple
    };
    QColor color = palette[QRandomGenerator::global()->bounded(palette.size())];

    Target t;
    t.center = QPointF(x, y);
    t.radius = static_cast<float>(targetSize);
    t.life = 1.0f;
    t.maxLife = 1.0f;
    t.color = color;
    targets.append(t);
    spawned++;
    reactionTimer.restart();
}

void AimTrainerWidget::gameTick() {
    pulsePhase += 0.05f;
    if (pulsePhase > 2 * M_PI) pulsePhase -= 2 * M_PI;

    // Fade out targets
    for (int i = targets.size() - 1; i >= 0; --i) {
        targets[i].life -= 0.008f; // ~2 seconds lifespan at 60fps
        if (targets[i].life <= 0) {
            targets.removeAt(i);
            misses++;
            emitStats();
        }
    }
    update();
}

void AimTrainerWidget::mousePressEvent(QMouseEvent *event) {
    if (!gameRunning) return;

    QPointF click = event->position();
    bool hitSomething = false;

    // Check in reverse so we hit top-most target first
    for (int i = targets.size() - 1; i >= 0; --i) {
        QPointF diff = click - targets[i].center;
        float dist = std::sqrt(diff.x() * diff.x() + diff.y() * diff.y());
        if (dist <= targets[i].radius) {
            hits++;
            totalReactionMs += reactionTimer.elapsed();
            targets.removeAt(i);
            hitSomething = true;
            break;
        }
    }

    if (!hitSomething) {
        misses++;
    }
    emitStats();
    update();
}

void AimTrainerWidget::emitStats() {
    double avgReaction = (hits > 0) ? (totalReactionMs / hits) : 0;
    double accuracy = (hits + misses > 0) ? (100.0 * hits / (hits + misses)) : 0;
    emit statsUpdated(hits, misses, avgReaction, accuracy);
}

void AimTrainerWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Dark background with subtle radial
    QRadialGradient bg(width() / 2, height() / 2, std::max(width(), height()) / 2);
    bg.setColorAt(0, QColor(20, 20, 25));
    bg.setColorAt(1, QColor(8, 8, 12));
    painter.fillRect(rect(), bg);

    // Draw crosshair grid
    painter.setPen(QPen(QColor(30, 30, 35), 1));
    for (int x = 0; x < width(); x += 40) painter.drawLine(x, 0, x, height());
    for (int y = 0; y < height(); y += 40) painter.drawLine(0, y, width(), y);

    if (!gameRunning && targets.isEmpty()) {
        painter.setPen(QColor(100, 100, 100));
        QFont font = painter.font();
        font.setPointSize(18);
        font.setBold(true);
        painter.setFont(font);
        painter.drawText(rect(), Qt::AlignCenter, "Press START to begin training");
        return;
    }

    // Draw targets
    for (const auto& t : targets) {
        float pulse = 1.0f + 0.08f * std::sin(pulsePhase * 3.0f);
        float r = t.radius * pulse;
        int alpha = static_cast<int>(255 * t.life);

        // Outer glow
        QRadialGradient glow(t.center, r * 2.5);
        QColor glowColor = t.color;
        glowColor.setAlpha(static_cast<int>(40 * t.life));
        glow.setColorAt(0, glowColor);
        glow.setColorAt(1, QColor(0, 0, 0, 0));
        painter.setPen(Qt::NoPen);
        painter.setBrush(glow);
        painter.drawEllipse(t.center, r * 2.5, r * 2.5);

        // Outer ring
        QColor ringColor = t.color;
        ringColor.setAlpha(alpha);
        painter.setPen(QPen(ringColor, 3));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(t.center, r, r);

        // Inner ring
        ringColor.setAlpha(static_cast<int>(alpha * 0.6));
        painter.setPen(QPen(ringColor, 2));
        painter.drawEllipse(t.center, r * 0.6, r * 0.6);

        // Bullseye
        QRadialGradient bullseye(t.center, r * 0.35);
        QColor centerColor = t.color;
        centerColor.setAlpha(alpha);
        bullseye.setColorAt(0, centerColor);
        bullseye.setColorAt(1, QColor(centerColor.red(), centerColor.green(), centerColor.blue(), static_cast<int>(alpha * 0.3)));
        painter.setPen(Qt::NoPen);
        painter.setBrush(bullseye);
        painter.drawEllipse(t.center, r * 0.35, r * 0.35);

        // Life bar under target
        float barWidth = r * 2;
        float barHeight = 4;
        float barX = t.center.x() - r;
        float barY = t.center.y() + r + 8;
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(40, 40, 40, static_cast<int>(120 * t.life)));
        painter.drawRoundedRect(QRectF(barX, barY, barWidth, barHeight), 2, 2);
        painter.setBrush(QColor(t.color.red(), t.color.green(), t.color.blue(), alpha));
        painter.drawRoundedRect(QRectF(barX, barY, barWidth * t.life, barHeight), 2, 2);
    }
}
