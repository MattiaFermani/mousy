#include "AimTrainerWidget.h"
#include "AudioFx.h"
#include "ProfileManager.h"
#include <QPainter>
#include <QMouseEvent>
#include <QRadialGradient>
#include <cmath>

static const QList<QColor> NeonPalette = {
    QColor(0, 210, 255),   // Cyan
    QColor(255, 60, 120),  // Pink
    QColor(0, 255, 150),   // Mint
    QColor(255, 200, 0),   // Gold
    QColor(180, 60, 255),  // Purple
    QColor(255, 100, 0),   // Orange
};

AimTrainerWidget::AimTrainerWidget(QWidget *parent) : QWidget(parent) {
    setMinimumSize(400, 300);
    setCursor(Qt::CrossCursor);

    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &AimTrainerWidget::gameTick);

    spawnTimer = new QTimer(this);
    connect(spawnTimer, &QTimer::timeout, this, &AimTrainerWidget::spawnTarget);

    countdownTimer = new QTimer(this);
    connect(countdownTimer, &QTimer::timeout, [this]() {
        countdown--;
        if (countdown <= 0) {
            AudioFx::instance().playCountdown(true);
            countdownTimer->stop();
            gameRunning = true;
            gameTimer->start(16);
            spawnTimer->start(spawnIntervalMs);
            reactionTimer.start();
            spawnTarget();
        } else {
            AudioFx::instance().playCountdown(false);
        }
        update();
    });
}

void AimTrainerWidget::startGame() {
    targets.clear();
    particles.clear();
    hitTexts.clear();
    hits = 0;
    misses = 0;
    spawned = 0;
    totalReactionMs = 0;
    pulsePhase = 0;
    currentStreak = 0;
    bestStreak = 0;
    gridShotIndex = 0;
    trackingScore = 0;
    trackingFrames = 0;
    gameRunning = false;

    // Start countdown
    countdown = 3;
    AudioFx::instance().playCountdown(false);
    countdownTimer->start(700);
    emitStats();
    update();
}

void AimTrainerWidget::stopGame() {
    gameRunning = false;
    countdown = 0;
    gameTimer->stop();
    spawnTimer->stop();
    countdownTimer->stop();

    double avgReaction = (hits > 0) ? (totalReactionMs / hits) : 0;
    double accuracy = (hits + misses > 0) ? (100.0 * hits / (hits + misses)) : 0;

    // Save personal records to active profile
    QString modeName = (gameMode == Flick) ? "Flick" : (gameMode == GridShot ? "GridShot" : "Classic");
    MouseProfile &prof = ProfileManager::instance().activeProfile();
    if (hits > prof.highScores.value(modeName, 0)) prof.highScores[modeName] = hits;
    if (accuracy > prof.bestAccuracy.value(modeName, 0.0)) prof.bestAccuracy[modeName] = accuracy;
    if (bestStreak > prof.bestStreak.value(modeName, 0)) prof.bestStreak[modeName] = bestStreak;
    if (avgReaction > 0 && (prof.bestReactionMs.value(modeName, 9999) == 0 || avgReaction < prof.bestReactionMs.value(modeName, 9999))) {
        prof.bestReactionMs[modeName] = static_cast<int>(avgReaction);
    }
    ProfileManager::instance().save();

    emit gameFinished(hits, misses, avgReaction, accuracy);

    targets.clear();
    emitStats();
    update();
}

void AimTrainerWidget::setTargetSize(int size) { targetSize = size; }
void AimTrainerWidget::setSpawnInterval(int ms) {
    spawnIntervalMs = ms;
    if (gameRunning) spawnTimer->setInterval(ms);
}
void AimTrainerWidget::setGameMode(GameMode mode) { gameMode = mode; }

void AimTrainerWidget::spawnTarget() {
    if (!gameRunning) return;

    int margin = targetSize + 30;

    if (gameMode == GridShot) {
        // 3x3 grid positions
        int cols = 3, rows = 3;
        float cellW = (width() - 2 * margin) / static_cast<float>(cols);
        float cellH = (height() - 2 * margin) / static_cast<float>(rows);
        int idx = gridShotIndex % (cols * rows);
        // Shuffle order with a simple pattern
        int order[] = {4, 0, 8, 2, 6, 1, 7, 3, 5};
        idx = order[idx % 9];
        int c = idx % cols, r = idx / cols;
        float x = margin + c * cellW + cellW / 2;
        float y = margin + r * cellH + cellH / 2;
        gridShotIndex++;

        Target t;
        t.center = QPointF(x, y);
        t.radius = static_cast<float>(targetSize);
        t.life = 1.0f;
        t.maxLife = 1.0f;
        t.color = NeonPalette[QRandomGenerator::global()->bounded(NeonPalette.size())];
        t.spawnScale = 0.0f;
        targets.append(t);
    } else if (gameMode == Flick) {
        // Spawn at edges, forcing fast flick movements
        int edge = QRandomGenerator::global()->bounded(4);
        float x, y;
        switch (edge) {
            case 0: x = margin; y = QRandomGenerator::global()->bounded(margin, std::max(margin+1, height()-margin)); break;
            case 1: x = width() - margin; y = QRandomGenerator::global()->bounded(margin, std::max(margin+1, height()-margin)); break;
            case 2: x = QRandomGenerator::global()->bounded(margin, std::max(margin+1, width()-margin)); y = margin; break;
            default: x = QRandomGenerator::global()->bounded(margin, std::max(margin+1, width()-margin)); y = height() - margin; break;
        }
        Target t;
        t.center = QPointF(x, y);
        t.radius = static_cast<float>(targetSize * 0.8);
        t.life = 1.0f;
        t.maxLife = 1.0f;
        t.color = NeonPalette[QRandomGenerator::global()->bounded(NeonPalette.size())];
        t.spawnScale = 0.0f;
        targets.append(t);
    } else {
        // Classic mode
        float x = QRandomGenerator::global()->bounded(margin, std::max(margin + 1, width() - margin));
        float y = QRandomGenerator::global()->bounded(margin, std::max(margin + 1, height() - margin));

        Target t;
        t.center = QPointF(x, y);
        t.radius = static_cast<float>(targetSize);
        t.life = 1.0f;
        t.maxLife = 1.0f;
        t.color = NeonPalette[QRandomGenerator::global()->bounded(NeonPalette.size())];
        t.spawnScale = 0.0f;
        targets.append(t);
    }
    spawned++;
    reactionTimer.restart();
}

void AimTrainerWidget::gameTick() {
    pulsePhase += 0.05f;
    if (pulsePhase > 2 * M_PI) pulsePhase -= 2 * M_PI;

    // Fade out targets
    for (int i = targets.size() - 1; i >= 0; --i) {
        targets[i].life -= 0.006f;
        targets[i].spawnScale = std::min(1.0f, targets[i].spawnScale + 0.08f); // Grow-in
        if (targets[i].life <= 0) {
            // Spawn red particles for miss
            spawnParticles(targets[i].center, QColor(255, 60, 60), 8);
            addHitText(targets[i].center, "MISS", QColor(255, 60, 60));
            AudioFx::instance().playMiss();
            targets.removeAt(i);
            misses++;
            currentStreak = 0;
            emitStats();
        }
    }

    // Update particles
    for (int i = particles.size() - 1; i >= 0; --i) {
        particles[i].pos += particles[i].vel;
        particles[i].vel *= 0.96f; // friction
        particles[i].vel.setY(particles[i].vel.y() + 0.15f); // gravity
        particles[i].life -= 0.025f;
        if (particles[i].life <= 0) particles.removeAt(i);
    }

    // Update hit texts
    for (int i = hitTexts.size() - 1; i >= 0; --i) {
        hitTexts[i].pos.setY(hitTexts[i].pos.y() - 1.5);
        hitTexts[i].life -= 0.02f;
        if (hitTexts[i].life <= 0) hitTexts.removeAt(i);
    }

    update();
}

void AimTrainerWidget::mousePressEvent(QMouseEvent *event) {
    if (!gameRunning) return;

    QPointF click = event->position();
    bool hitSomething = false;

    for (int i = targets.size() - 1; i >= 0; --i) {
        QPointF diff = click - targets[i].center;
        float dist = std::sqrt(diff.x() * diff.x() + diff.y() * diff.y());
        float effectiveRadius = targets[i].radius * targets[i].spawnScale;
        if (dist <= effectiveRadius) {
            hits++;
            currentStreak++;
            if (currentStreak > bestStreak) bestStreak = currentStreak;
            totalReactionMs += reactionTimer.elapsed();

            // Particles!
            spawnParticles(targets[i].center, targets[i].color, 20);

            // Hit text
            int reactionMs = static_cast<int>(reactionTimer.elapsed());
            QString text;
            QColor textColor;
            if (reactionMs < 200) { text = "INSANE!"; textColor = QColor(255, 200, 0); }
            else if (reactionMs < 350) { text = "GREAT!"; textColor = QColor(0, 255, 150); }
            else if (reactionMs < 500) { text = "GOOD"; textColor = QColor(0, 210, 255); }
            else { text = "OK"; textColor = QColor(180, 180, 180); }

            if (currentStreak >= 5) {
                text += QString("  🔥x%1").arg(currentStreak);
                textColor = QColor(255, 150, 0);
            }
            addHitText(targets[i].center, text, textColor);

            if (currentStreak >= 5 && currentStreak % 5 == 0) {
                AudioFx::instance().playStreak();
            } else {
                AudioFx::instance().playHit();
            }

            targets.removeAt(i);
            hitSomething = true;
            reactionTimer.restart();
            break;
        }
    }

    if (!hitSomething) {
        misses++;
        currentStreak = 0;
        spawnParticles(click, QColor(255, 60, 60, 150), 6);
        addHitText(click, "MISS", QColor(255, 60, 60));
        AudioFx::instance().playMiss();
    }
    emitStats();
    update();
}

void AimTrainerWidget::mouseMoveEvent(QMouseEvent *event) {
    // Used for tracking mode in the future
    Q_UNUSED(event);
}

void AimTrainerWidget::spawnParticles(QPointF pos, QColor color, int count) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.pos = pos;
        float angle = QRandomGenerator::global()->bounded(360) * M_PI / 180.0f;
        float speed = 2.0f + QRandomGenerator::global()->bounded(60) / 10.0f;
        p.vel = QPointF(std::cos(angle) * speed, std::sin(angle) * speed);
        p.color = color;
        p.life = 0.6f + QRandomGenerator::global()->bounded(40) / 100.0f;
        p.size = 2.0f + QRandomGenerator::global()->bounded(40) / 10.0f;
        particles.append(p);
    }
}

void AimTrainerWidget::addHitText(QPointF pos, const QString &text, QColor color) {
    HitText ht;
    ht.pos = pos;
    ht.text = text;
    ht.color = color;
    ht.life = 1.0f;
    hitTexts.append(ht);
}

void AimTrainerWidget::emitStats() {
    double avgReaction = (hits > 0) ? (totalReactionMs / hits) : 0;
    double accuracy = (hits + misses > 0) ? (100.0 * hits / (hits + misses)) : 0;
    emit statsUpdated(hits, misses, avgReaction, accuracy);
}

void AimTrainerWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Background
    QRadialGradient bg(width() / 2, height() / 2, std::max(width(), height()) / 2);
    bg.setColorAt(0, QColor(18, 18, 24));
    bg.setColorAt(1, QColor(6, 6, 10));
    painter.fillRect(rect(), bg);

    // Grid
    painter.setPen(QPen(QColor(25, 25, 32), 1));
    for (int x = 0; x < width(); x += 40) painter.drawLine(x, 0, x, height());
    for (int y = 0; y < height(); y += 40) painter.drawLine(0, y, width(), y);

    // Countdown
    if (countdown > 0) {
        painter.setPen(Qt::NoPen);
        // Big number
        QFont font = painter.font();
        font.setPointSize(80);
        font.setBold(true);
        painter.setFont(font);
        painter.setPen(QColor(0, 210, 255, 200));
        painter.drawText(rect(), Qt::AlignCenter, QString::number(countdown));
        return;
    }

    if (!gameRunning && targets.isEmpty() && hits == 0 && misses == 0) {
        painter.setPen(QColor(80, 80, 90));
        QFont font = painter.font();
        font.setPointSize(18);
        font.setBold(true);
        painter.setFont(font);
        painter.drawText(rect(), Qt::AlignCenter, "Press START to begin training");
        return;
    }

    // Draw targets
    for (const auto& t : targets) {
        float scale = t.spawnScale;
        float pulse = 1.0f + 0.06f * std::sin(pulsePhase * 3.0f);
        float r = t.radius * pulse * scale;
        int alpha = static_cast<int>(255 * t.life * scale);

        if (r < 1) continue;

        // Outer glow
        QRadialGradient glow(t.center, r * 3.0);
        QColor glowColor = t.color;
        glowColor.setAlpha(static_cast<int>(30 * t.life * scale));
        glow.setColorAt(0, glowColor);
        glow.setColorAt(1, QColor(0, 0, 0, 0));
        painter.setPen(Qt::NoPen);
        painter.setBrush(glow);
        painter.drawEllipse(t.center, r * 3.0, r * 3.0);

        // Outer ring
        QColor ringColor = t.color;
        ringColor.setAlpha(alpha);
        painter.setPen(QPen(ringColor, 3));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(t.center, r, r);

        // Middle ring
        ringColor.setAlpha(static_cast<int>(alpha * 0.5));
        painter.setPen(QPen(ringColor, 2));
        painter.drawEllipse(t.center, r * 0.65, r * 0.65);

        // Inner ring
        ringColor.setAlpha(static_cast<int>(alpha * 0.3));
        painter.setPen(QPen(ringColor, 1.5));
        painter.drawEllipse(t.center, r * 0.35, r * 0.35);

        // Bullseye fill
        QRadialGradient bullseye(t.center, r * 0.3);
        QColor centerColor = t.color;
        centerColor.setAlpha(alpha);
        bullseye.setColorAt(0, centerColor);
        bullseye.setColorAt(1, QColor(centerColor.red(), centerColor.green(), centerColor.blue(), static_cast<int>(alpha * 0.2)));
        painter.setPen(Qt::NoPen);
        painter.setBrush(bullseye);
        painter.drawEllipse(t.center, r * 0.3, r * 0.3);

        // Life bar
        float barWidth = r * 2;
        float barHeight = 4;
        float barX = t.center.x() - r;
        float barY = t.center.y() + r + 10;
        painter.setBrush(QColor(30, 30, 30, static_cast<int>(100 * scale)));
        painter.drawRoundedRect(QRectF(barX, barY, barWidth, barHeight), 2, 2);

        // Life fill - color changes as life decreases
        QColor lifeColor;
        if (t.life > 0.5f) lifeColor = QColor(0, 255, 150, alpha);
        else if (t.life > 0.25f) lifeColor = QColor(255, 200, 0, alpha);
        else lifeColor = QColor(255, 60, 60, alpha);
        painter.setBrush(lifeColor);
        painter.drawRoundedRect(QRectF(barX, barY, barWidth * t.life, barHeight), 2, 2);
    }

    // Draw particles
    for (const auto& p : particles) {
        int alpha = static_cast<int>(255 * p.life);
        QColor c = p.color;
        c.setAlpha(alpha);
        painter.setPen(Qt::NoPen);
        painter.setBrush(c);
        painter.drawEllipse(p.pos, p.size * p.life, p.size * p.life);
    }

    // Draw hit texts
    for (const auto& ht : hitTexts) {
        QFont font = painter.font();
        font.setPointSize(static_cast<int>(14 + 4 * ht.life));
        font.setBold(true);
        painter.setFont(font);
        QColor c = ht.color;
        c.setAlpha(static_cast<int>(255 * ht.life));
        painter.setPen(c);
        painter.drawText(static_cast<int>(ht.pos.x()) - 40, static_cast<int>(ht.pos.y()), 80, 30,
                         Qt::AlignCenter, ht.text);
    }

    // Streak display
    if (currentStreak >= 3 && gameRunning) {
        QFont font = painter.font();
        font.setPointSize(14);
        font.setBold(true);
        painter.setFont(font);
        float streakPulse = 0.7f + 0.3f * std::sin(pulsePhase * 4);
        painter.setPen(QColor(255, 150, 0, static_cast<int>(255 * streakPulse)));
        painter.drawText(width() - 160, 30, QString("🔥 Streak: %1").arg(currentStreak));
    }
}
