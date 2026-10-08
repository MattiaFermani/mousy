#include "MouseViewWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QLinearGradient>
#include <QRadialGradient>
#include <cmath>

MouseViewWidget::MouseViewWidget(QWidget *parent) : QWidget(parent) {
    setMouseTracking(true);
    setupRegions();

    animTimer = new QTimer(this);
    connect(animTimer, &QTimer::timeout, [this]() {
        pulsePhase += 0.05f;
        if (pulsePhase > 2 * M_PI) pulsePhase -= 2 * M_PI;
        update();
    });
    animTimer->start(16); // ~60fps
}

void MouseViewWidget::setBindings(const QMap<int, QString> &b) {
    bindings = b;
    update();
}

void MouseViewWidget::setRgbEffect(RgbEffect effect) {
    rgbEffect = effect;
    update();
}

void MouseViewWidget::setDpi(int dpi) {
    currentDpi = dpi;
    update();
}

void MouseViewWidget::setupRegions() {
    // Body (Bottom layer)
    QPainterPath body;
    body.addRoundedRect(100, 150, 200, 260, 100, 100);
    regions.append({0, body, QColor(26, 26, 32), "Body"});

    // Left Button
    QPainterPath leftBtn;
    leftBtn.addRoundedRect(100, 30, 95, 140, 25, 25);
    regions.append({1, leftBtn, QColor(42, 42, 50), "Left Click"});

    // Right Button
    QPainterPath rightBtn;
    rightBtn.addRoundedRect(205, 30, 95, 140, 25, 25);
    regions.append({2, rightBtn, QColor(42, 42, 50), "Right Click"});

    // Scroll Wheel
    QPainterPath wheel;
    wheel.addRoundedRect(185, 50, 30, 70, 15, 15);
    regions.append({3, wheel, QColor(18, 18, 22), "Wheel"});
    
    // Side buttons (Left side)
    QPainterPath sideBtn1;
    sideBtn1.addRoundedRect(80, 180, 18, 55, 7, 7);
    regions.append({4, sideBtn1, QColor(48, 48, 58), "Forward"});

    QPainterPath sideBtn2;
    sideBtn2.addRoundedRect(80, 255, 18, 55, 7, 7);
    regions.append({5, sideBtn2, QColor(48, 48, 58), "Back"});

    // DPI Button
    QPainterPath dpiBtn;
    dpiBtn.addRoundedRect(185, 165, 30, 24, 6, 6);
    regions.append({6, dpiBtn, QColor(50, 35, 65), "DPI"});
}

QColor MouseViewWidget::calculateLedColor(float t, float phase, float brightness) const {
    int r = 0, g = 0, b = 0;
    switch (rgbEffect) {
        case RainbowCycle: {
            float hue = std::fmod((phase / (2 * M_PI) + t) * 360.0f, 360.0f);
            return QColor::fromHsv(static_cast<int>(hue), 230, static_cast<int>(255 * brightness));
        }
        case CyberpunkPink: {
            float wave = 0.5f + 0.5f * std::sin(phase + t * 3.0f);
            r = static_cast<int>((255 * wave + 255 * (1 - wave)) * brightness);
            g = static_cast<int>((20 * wave + 210 * (1 - wave)) * brightness);
            b = static_cast<int>((150 * wave + 0 * (1 - wave)) * brightness);
            break;
        }
        case MatrixGreen: {
            float wave = 0.4f + 0.6f * std::max(0.0f, std::sin(phase * 1.5f + t * 4.0f));
            r = static_cast<int>(10 * brightness);
            g = static_cast<int>(255 * wave * brightness);
            b = static_cast<int>(80 * wave * brightness);
            break;
        }
        case CrimsonFire: {
            float wave = 0.4f + 0.6f * std::max(0.0f, std::sin(phase * 1.8f + t * 5.0f));
            r = static_cast<int>(255 * brightness);
            g = static_cast<int>(60 * wave * brightness);
            b = 0;
            break;
        }
        case NeonCyan:
        default: {
            float wave = 0.4f + 0.6f * std::max(0.0f, std::sin(phase + t * 4.0f));
            r = 0;
            g = static_cast<int>(210 * wave * brightness);
            b = static_cast<int>(255 * wave * brightness);
            break;
        }
    }
    return QColor(std::clamp(r, 0, 255), std::clamp(g, 0, 255), std::clamp(b, 0, 255));
}

void MouseViewWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Center drawing
    float offsetX = width() / 2.0f - 200;
    float offsetY = height() / 2.0f - 230;
    painter.translate(offsetX, offsetY);

    // Ambient glow behind mouse using current RGB theme
    QColor ambientColor = calculateLedColor(0.5f, pulsePhase, 0.4f);
    ambientColor.setAlpha(35);
    QRadialGradient glow(200, 220, 280);
    glow.setColorAt(0, ambientColor);
    glow.setColorAt(0.6, QColor(ambientColor.red(), ambientColor.green(), ambientColor.blue(), 10));
    glow.setColorAt(1, QColor(0, 0, 0, 0));
    painter.fillRect(-80, -80, 560, 560, glow);

    // Selected button pulse glow
    if (selectedRegion > 0) {
        for (const auto &region : regions) {
            if (region.id == selectedRegion) {
                float pulse = 0.5f + 0.5f * std::sin(pulsePhase * 2);
                QRectF bounds = region.path.boundingRect();
                QRadialGradient selGlow(bounds.center(), bounds.width());
                QColor selColor = calculateLedColor(0.2f, pulsePhase, 0.8f);
                selColor.setAlpha(static_cast<int>(70 * pulse));
                selGlow.setColorAt(0, selColor);
                selGlow.setColorAt(1, QColor(0, 0, 0, 0));
                painter.setPen(Qt::NoPen);
                painter.setBrush(selGlow);
                painter.drawEllipse(bounds.center(), bounds.width() * 1.2, bounds.height() * 1.2);
                break;
            }
        }
    }

    for (const auto& region : regions) {
        bool isHovered = (hoveredRegion == region.id);
        bool isSelected = (selectedRegion == region.id);
        
        QLinearGradient gradient(0, 0, 0, 450);
        QColor baseColor = region.color;
        
        if (isSelected && region.id != 0) {
            float pulse = 0.85f + 0.15f * std::sin(pulsePhase * 2);
            gradient.setColorAt(0, baseColor.lighter(static_cast<int>(180 * pulse)));
            gradient.setColorAt(1, baseColor.lighter(static_cast<int>(140 * pulse)));
        } else if (isHovered && region.id != 0) {
            gradient.setColorAt(0, baseColor.lighter(150));
            gradient.setColorAt(1, baseColor.lighter(120));
        } else {
            gradient.setColorAt(0, baseColor.lighter(110));
            gradient.setColorAt(1, baseColor.darker(110));
        }
        
        // Shadow
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, 90));
        painter.translate(0, 6);
        painter.drawPath(region.path);
        painter.translate(0, -6);

        // Shape outline & fill
        painter.setBrush(gradient);
        QColor themeHighlight = calculateLedColor(0.5f, pulsePhase, 1.0f);
        if (isSelected && region.id != 0) {
            float pulse = 0.5f + 0.5f * std::sin(pulsePhase * 2);
            themeHighlight.setAlpha(static_cast<int>(180 + 75 * pulse));
            painter.setPen(QPen(themeHighlight, 3));
        } else if (isHovered && region.id != 0) {
            themeHighlight.setAlpha(160);
            painter.setPen(QPen(themeHighlight, 2));
        } else {
            painter.setPen(QPen(QColor(15, 15, 18), 2));
        }
        painter.drawPath(region.path);
        
        // Text labels
        if (region.id != 0) {
            QRectF bounds = region.path.boundingRect();
            
            QColor textColor = isSelected ? themeHighlight : 
                               isHovered ? QColor(220, 220, 220) : QColor(140, 140, 140);
            painter.setPen(textColor);
            QFont font = painter.font();
            font.setBold(true);
            font.setPointSize(bounds.width() > 50 ? 10 : 8);
            painter.setFont(font);
            
            QString label = region.name;
            if (region.id == 6) {
                // DPI Button shows current DPI
                label = QString("DPI\n%1").arg(currentDpi);
            } else if (bindings.contains(region.id)) {
                label = region.name + "\n→ " + bindings[region.id];
            }
            painter.drawText(bounds, Qt::AlignCenter | Qt::TextWordWrap, label);
        }
    }

    // Draw LED strip on bottom of mouse
    float ledY = 390;
    float ledStartX = 125;
    float ledWidth = 150;
    int ledCount = 14;
    for (int i = 0; i < ledCount; ++i) {
        float t = static_cast<float>(i) / ledCount;
        float brightness = 0.4f + 0.6f * std::max(0.0f, std::sin(pulsePhase + t * 4.0f));
        QColor ledCol = calculateLedColor(t, pulsePhase, brightness);
        
        float x = ledStartX + t * ledWidth;
        QRadialGradient ledGlow(x, ledY, 12);
        ledCol.setAlpha(static_cast<int>(190 * brightness));
        ledGlow.setColorAt(0, ledCol);
        ledGlow.setColorAt(1, QColor(0, 0, 0, 0));
        painter.setPen(Qt::NoPen);
        painter.setBrush(ledGlow);
        painter.drawEllipse(QPointF(x, ledY), 12, 12);
        
        // Core dot
        ledCol.setAlpha(255);
        painter.setBrush(ledCol);
        painter.drawEllipse(QPointF(x, ledY), 3.5, 3.5);
    }
}

void MouseViewWidget::mousePressEvent(QMouseEvent *event) {
    QPointF pt = event->position();
    pt.setX(pt.x() - (width() / 2.0f - 200));
    pt.setY(pt.y() - (height() / 2.0f - 230));

    for (int i = regions.size() - 1; i >= 0; --i) {
        if (regions[i].path.contains(pt) && regions[i].id != 0) {
            selectedRegion = regions[i].id;
            emit mouseButtonClicked(regions[i].id, regions[i].name);
            update();
            break;
        }
    }
}

void MouseViewWidget::mouseMoveEvent(QMouseEvent *event) {
    QPointF pt = event->position();
    pt.setX(pt.x() - (width() / 2.0f - 200));
    pt.setY(pt.y() - (height() / 2.0f - 230));

    int newHoveredRegion = -1;
    for (int i = regions.size() - 1; i >= 0; --i) {
        if (regions[i].path.contains(pt) && regions[i].id != 0) {
            newHoveredRegion = regions[i].id;
            break;
        }
    }

    if (newHoveredRegion != hoveredRegion) {
        hoveredRegion = newHoveredRegion;
        setCursor(hoveredRegion >= 0 ? Qt::PointingHandCursor : Qt::ArrowCursor);
        update();
    }
}
