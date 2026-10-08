#include "HeatmapWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QLinearGradient>
#include <cmath>

HeatmapWidget::HeatmapWidget(QWidget *parent) : QWidget(parent) {
    setMouseTracking(true);
    setMinimumSize(400, 300);

    decayTimer = new QTimer(this);
    connect(decayTimer, &QTimer::timeout, this, &HeatmapWidget::decayHeat);
    decayTimer->start(80);

    trailTimer = new QTimer(this);
    connect(trailTimer, &QTimer::timeout, this, &HeatmapWidget::ageTrail);
    trailTimer->start(30);
}

void HeatmapWidget::allocateGrid() {
    gridCols = width() / CellSize + 1;
    gridRows = height() / CellSize + 1;
    int total = gridCols * gridRows;
    moveHeat.resize(total);
    clickHeat.resize(total);
    moveHeat.fill(0.0f);
    clickHeat.fill(0.0f);
    maxMoveHeat = 1.0f;
    maxClickHeat = 1.0f;
}

void HeatmapWidget::resizeEvent(QResizeEvent *event) {
    allocateGrid();
    QWidget::resizeEvent(event);
}

void HeatmapWidget::clearHeatmap() {
    moveHeat.fill(0.0f);
    clickHeat.fill(0.0f);
    maxMoveHeat = 1.0f;
    maxClickHeat = 1.0f;
    totalMoves = 0;
    totalClicks = 0;
    trail.clear();
    emit statsChanged(0, 0);
    update();
}

void HeatmapWidget::setTrackingEnabled(bool enabled) {
    tracking = enabled;
}

void HeatmapWidget::setDisplayMode(DisplayMode mode) {
    displayMode = mode;
    update();
}

void HeatmapWidget::setTrailEnabled(bool enabled) {
    showTrail = enabled;
    if (!showTrail) trail.clear();
    update();
}

bool HeatmapWidget::exportImage(const QString &filePath) {
    return grab().save(filePath);
}

void HeatmapWidget::decayHeat() {
    bool changed = false;
    for (int i = 0; i < moveHeat.size(); ++i) {
        if (moveHeat[i] > 0.001f) {
            moveHeat[i] *= 0.993f;
            changed = true;
        }
    }
    for (int i = 0; i < clickHeat.size(); ++i) {
        if (clickHeat[i] > 0.001f) {
            clickHeat[i] *= 0.997f;
            changed = true;
        }
    }
    if (changed) update();
}

void HeatmapWidget::ageTrail() {
    bool changed = false;
    for (int i = trail.size() - 1; i >= 0; --i) {
        trail[i].age += 0.015f;
        if (trail[i].age >= 1.0f) {
            trail.removeAt(i);
        }
        changed = true;
    }
    if (changed) update();
}

QColor HeatmapWidget::heatColor(float value, bool isClick) const {
    value = std::min(value, 1.0f);
    if (isClick) {
        // Purple → magenta → white
        int r = static_cast<int>(80 + 175 * value);
        int g = static_cast<int>(20 + 80 * value * value);
        int b = static_cast<int>(160 + 95 * value);
        int a = static_cast<int>(50 + 200 * value);
        return QColor(r, g, b, a);
    } else {
        // Dark blue → cyan → green → yellow → white
        if (value < 0.25f) {
            float t = value / 0.25f;
            return QColor(0, static_cast<int>(60 * t), static_cast<int>(180 + 75 * t), static_cast<int>(30 + 120 * t));
        } else if (value < 0.5f) {
            float t = (value - 0.25f) / 0.25f;
            return QColor(0, static_cast<int>(60 + 195 * t), static_cast<int>(255 - 55 * t), static_cast<int>(120 + 80 * t));
        } else if (value < 0.75f) {
            float t = (value - 0.5f) / 0.25f;
            return QColor(static_cast<int>(255 * t), 255, static_cast<int>(200 - 200 * t), static_cast<int>(180 + 50 * t));
        } else {
            float t = (value - 0.75f) / 0.25f;
            return QColor(255, 255, static_cast<int>(255 * t), static_cast<int>(220 + 35 * t));
        }
    }
}

void HeatmapWidget::mouseMoveEvent(QMouseEvent *event) {
    if (!tracking || gridCols == 0 || gridRows == 0) return;

    QPointF pos = event->position();
    int col = static_cast<int>(pos.x()) / CellSize;
    int row = static_cast<int>(pos.y()) / CellSize;

    if (col < 0 || col >= gridCols || row < 0 || row >= gridRows) return;

    // Gaussian splat (5x5) for smoother heatmap
    for (int dy = -2; dy <= 2; ++dy) {
        for (int dx = -2; dx <= 2; ++dx) {
            int c = col + dx, r = row + dy;
            if (c >= 0 && c < gridCols && r >= 0 && r < gridRows) {
                float dist = std::sqrt(static_cast<float>(dx * dx + dy * dy));
                float weight = std::max(0.0f, 1.0f - dist / 3.0f);
                int idx = r * gridCols + c;
                moveHeat[idx] += weight * 0.12f;
                if (moveHeat[idx] > maxMoveHeat) maxMoveHeat = moveHeat[idx];
            }
        }
    }

    // Add to trail
    TrailPoint tp;
    tp.pos = pos;
    tp.age = 0;
    trail.append(tp);
    if (trail.size() > MaxTrail) trail.removeFirst();

    totalMoves++;
    emit statsChanged(totalMoves, totalClicks);
    update();
}

void HeatmapWidget::mousePressEvent(QMouseEvent *event) {
    if (!tracking || gridCols == 0 || gridRows == 0) return;

    int col = static_cast<int>(event->position().x()) / CellSize;
    int row = static_cast<int>(event->position().y()) / CellSize;

    if (col < 0 || col >= gridCols || row < 0 || row >= gridRows) return;

    // Larger splat for clicks (7x7)
    for (int dy = -3; dy <= 3; ++dy) {
        for (int dx = -3; dx <= 3; ++dx) {
            int c = col + dx, r = row + dy;
            if (c >= 0 && c < gridCols && r >= 0 && r < gridRows) {
                float dist = std::sqrt(static_cast<float>(dx * dx + dy * dy));
                float weight = std::max(0.0f, 1.0f - dist / 4.0f);
                int idx = r * gridCols + c;
                clickHeat[idx] += weight * 2.5f;
                if (clickHeat[idx] > maxClickHeat) maxClickHeat = clickHeat[idx];
            }
        }
    }
    totalClicks++;
    emit statsChanged(totalMoves, totalClicks);
    update();
}

void HeatmapWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    // Dark background
    painter.fillRect(rect(), QColor(10, 10, 14));

    // Subtle grid
    painter.setPen(QPen(QColor(22, 22, 28), 1));
    for (int x = 0; x < width(); x += CellSize * 10) {
        painter.drawLine(x, 0, x, height());
    }
    for (int y = 0; y < height(); y += CellSize * 10) {
        painter.drawLine(0, y, width(), y);
    }

    if (gridCols == 0 || gridRows == 0) return;

    // Draw heat cells
    for (int r = 0; r < gridRows; ++r) {
        for (int c = 0; c < gridCols; ++c) {
            int idx = r * gridCols + c;
            float mv = (maxMoveHeat > 0) ? moveHeat[idx] / maxMoveHeat : 0;
            float cl = (maxClickHeat > 0) ? clickHeat[idx] / maxClickHeat : 0;

            if (mv < 0.01f && cl < 0.01f) continue;

            QRect cellRect(c * CellSize, r * CellSize, CellSize, CellSize);

            if (displayMode == Combined || displayMode == MoveHeat) {
                if (mv > 0.01f) {
                    painter.fillRect(cellRect, heatColor(mv, false));
                }
            }
            if (displayMode == Combined || displayMode == ClickHeat) {
                if (cl > 0.01f) {
                    painter.fillRect(cellRect, heatColor(cl, true));
                }
            }
        }
    }

    // Draw trail path
    painter.setRenderHint(QPainter::Antialiasing, true);
    if (showTrail && trail.size() >= 2) {
        for (int i = 1; i < trail.size(); ++i) {
            float age = trail[i].age;
            int alpha = static_cast<int>(180 * (1.0f - age));
            float width = 2.0f * (1.0f - age) + 0.5f;
            
            QColor trailColor(0, 210, 255, std::max(0, alpha));
            painter.setPen(QPen(trailColor, width, Qt::SolidLine, Qt::RoundCap));
            painter.drawLine(trail[i - 1].pos, trail[i].pos);
        }
    }

    // Click ripple markers
    // (clicks show as expanding rings via the click heat, but we also draw the trail dots)

    // Stats overlay
    painter.setPen(QColor(200, 200, 200, 160));
    QFont font = painter.font();
    font.setPointSize(11);
    font.setBold(true);
    painter.setFont(font);

    // Mode indicator
    QString modeText;
    switch (displayMode) {
        case MoveHeat: modeText = "Movement"; break;
        case ClickHeat: modeText = "Clicks"; break;
        case Combined: modeText = "Combined"; break;
    }
    painter.drawText(15, 25, QString("Mode: %1").arg(modeText));
    painter.drawText(15, height() - 15, QString("Moves: %1  |  Clicks: %2").arg(totalMoves).arg(totalClicks));

    // Hint text when empty
    if (totalMoves == 0 && totalClicks == 0) {
        painter.setPen(QColor(80, 80, 80));
        font.setPointSize(16);
        painter.setFont(font);
        painter.drawText(rect(), Qt::AlignCenter, "Move and click your mouse here\nto generate a heatmap");
    }
}
