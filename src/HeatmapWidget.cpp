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
    decayTimer->start(100); // Decay every 100ms for a smooth fading effect
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
    update();
}

void HeatmapWidget::setTrackingEnabled(bool enabled) {
    tracking = enabled;
}

void HeatmapWidget::decayHeat() {
    bool changed = false;
    for (int i = 0; i < moveHeat.size(); ++i) {
        if (moveHeat[i] > 0.001f) {
            moveHeat[i] *= 0.995f;
            changed = true;
        }
    }
    for (int i = 0; i < clickHeat.size(); ++i) {
        if (clickHeat[i] > 0.001f) {
            clickHeat[i] *= 0.998f;
            changed = true;
        }
    }
    if (changed) update();
}

QColor HeatmapWidget::heatColor(float value, bool isClick) const {
    value = std::min(value, 1.0f);
    if (isClick) {
        // Click heat: purple → magenta → white
        int r = static_cast<int>(100 + 155 * value);
        int g = static_cast<int>(50 * value);
        int b = static_cast<int>(180 + 75 * value);
        int a = static_cast<int>(40 + 200 * value);
        return QColor(r, g, b, a);
    } else {
        // Move heat: dark blue → cyan → green → yellow
        if (value < 0.33f) {
            float t = value / 0.33f;
            return QColor(0, static_cast<int>(100 * t), static_cast<int>(200 + 55 * t), static_cast<int>(30 + 150 * t));
        } else if (value < 0.66f) {
            float t = (value - 0.33f) / 0.33f;
            return QColor(0, static_cast<int>(100 + 155 * t), static_cast<int>(255 - 55 * t), static_cast<int>(100 + 100 * t));
        } else {
            float t = (value - 0.66f) / 0.34f;
            return QColor(static_cast<int>(255 * t), 255, static_cast<int>(200 - 200 * t), static_cast<int>(180 + 75 * t));
        }
    }
}

void HeatmapWidget::mouseMoveEvent(QMouseEvent *event) {
    if (!tracking || gridCols == 0 || gridRows == 0) return;

    int col = static_cast<int>(event->position().x()) / CellSize;
    int row = static_cast<int>(event->position().y()) / CellSize;

    if (col < 0 || col >= gridCols || row < 0 || row >= gridRows) return;

    // Gaussian splat (3x3) for smoother heatmap
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            int c = col + dx, r = row + dy;
            if (c >= 0 && c < gridCols && r >= 0 && r < gridRows) {
                float weight = (dx == 0 && dy == 0) ? 1.0f : 0.4f;
                int idx = r * gridCols + c;
                moveHeat[idx] += weight * 0.15f;
                if (moveHeat[idx] > maxMoveHeat) maxMoveHeat = moveHeat[idx];
            }
        }
    }
    totalMoves++;
    update();
}

void HeatmapWidget::mousePressEvent(QMouseEvent *event) {
    if (!tracking || gridCols == 0 || gridRows == 0) return;

    int col = static_cast<int>(event->position().x()) / CellSize;
    int row = static_cast<int>(event->position().y()) / CellSize;

    if (col < 0 || col >= gridCols || row < 0 || row >= gridRows) return;

    // Larger splat for clicks (5x5)
    for (int dy = -2; dy <= 2; ++dy) {
        for (int dx = -2; dx <= 2; ++dx) {
            int c = col + dx, r = row + dy;
            if (c >= 0 && c < gridCols && r >= 0 && r < gridRows) {
                float dist = std::sqrt(static_cast<float>(dx * dx + dy * dy));
                float weight = std::max(0.0f, 1.0f - dist / 3.0f);
                int idx = r * gridCols + c;
                clickHeat[idx] += weight * 2.0f;
                if (clickHeat[idx] > maxClickHeat) maxClickHeat = clickHeat[idx];
            }
        }
    }
    totalClicks++;
    update();
}

void HeatmapWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    // Dark background
    painter.fillRect(rect(), QColor(12, 12, 16));

    // Draw subtle grid
    painter.setPen(QPen(QColor(25, 25, 30), 1));
    for (int x = 0; x < width(); x += CellSize * 8) {
        painter.drawLine(x, 0, x, height());
    }
    for (int y = 0; y < height(); y += CellSize * 8) {
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

    // Draw stats overlay
    painter.setPen(QColor(200, 200, 200, 180));
    QFont font = painter.font();
    font.setPointSize(11);
    font.setBold(true);
    painter.setFont(font);

    QString statsText = QString("Moves: %1  |  Clicks: %2").arg(totalMoves).arg(totalClicks);
    painter.drawText(15, height() - 15, statsText);

    // Hint text when empty
    if (totalMoves == 0 && totalClicks == 0) {
        painter.setPen(QColor(100, 100, 100));
        font.setPointSize(16);
        painter.setFont(font);
        painter.drawText(rect(), Qt::AlignCenter, "Move and click your mouse here\nto generate a heatmap");
    }
}
