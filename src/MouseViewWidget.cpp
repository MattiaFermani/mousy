#include "MouseViewWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QLinearGradient>
#include <QRadialGradient>

MouseViewWidget::MouseViewWidget(QWidget *parent) : QWidget(parent) {
    setMouseTracking(true); // Required to track mouse hover events
    setupRegions();
}

void MouseViewWidget::setupRegions() {
    // Body (Bottom layer)
    QPainterPath body;
    body.addRoundedRect(100, 150, 200, 260, 100, 100);
    regions.append({0, body, QColor(30, 30, 35), "Body"});

    // Left Button
    QPainterPath leftBtn;
    leftBtn.addRoundedRect(100, 30, 95, 140, 25, 25);
    regions.append({1, leftBtn, QColor(45, 45, 50), "Left Click"});

    // Right Button
    QPainterPath rightBtn;
    rightBtn.addRoundedRect(205, 30, 95, 140, 25, 25);
    regions.append({2, rightBtn, QColor(45, 45, 50), "Right Click"});

    // Scroll Wheel
    QPainterPath wheel;
    wheel.addRoundedRect(185, 50, 30, 70, 15, 15);
    regions.append({3, wheel, QColor(20, 20, 25), "Wheel"});
    
    // Side buttons (Left side)
    QPainterPath sideBtn1;
    sideBtn1.addRoundedRect(85, 180, 15, 60, 7, 7);
    regions.append({4, sideBtn1, QColor(50, 50, 60), "Forward"});

    QPainterPath sideBtn2;
    sideBtn2.addRoundedRect(85, 260, 15, 60, 7, 7);
    regions.append({5, sideBtn2, QColor(50, 50, 60), "Back"});
}

void MouseViewWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Center drawing in widget
    painter.translate(width() / 2 - 200, height() / 2 - 230);

    // Draw an ambient neon glow behind the mouse
    QRadialGradient glow(200, 220, 250);
    glow.setColorAt(0, QColor(0, 210, 255, 40));
    glow.setColorAt(1, QColor(0, 0, 0, 0));
    painter.fillRect(-50, -50, 500, 500, glow);

    for (const auto& region : regions) {
        bool isHovered = (hoveredRegion == region.id);
        
        // Setup modern gradient for buttons
        QLinearGradient gradient(0, 0, 0, 400);
        QColor baseColor = region.color;
        
        if (isHovered && region.id != 0) { // Don't highlight the body aggressively
            gradient.setColorAt(0, baseColor.lighter(150));
            gradient.setColorAt(1, baseColor.lighter(120));
        } else {
            gradient.setColorAt(0, baseColor.lighter(110));
            gradient.setColorAt(1, baseColor.darker(110));
        }
        
        // Draw Shadow
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, 100));
        painter.translate(0, 5);
        painter.drawPath(region.path);
        painter.translate(0, -5);

        // Draw Shape
        painter.setBrush(gradient);
        if (isHovered && region.id != 0) {
            painter.setPen(QPen(QColor(0, 210, 255), 2)); // Neon cyan border on hover
        } else {
            painter.setPen(QPen(QColor(15, 15, 15), 2)); // Dark border
        }
        painter.drawPath(region.path);
        
        // Draw Text
        if (region.id != 0) { // Hide text on body
            painter.setPen(isHovered ? QColor(0, 210, 255) : QColor(180, 180, 180));
            QFont font = painter.font();
            font.setBold(true);
            font.setPointSize(10);
            painter.setFont(font);
            
            QRectF bounds = region.path.boundingRect();
            painter.drawText(bounds, Qt::AlignCenter | Qt::TextWordWrap, region.name);
        }
    }
}

void MouseViewWidget::mousePressEvent(QMouseEvent *event) {
    QPointF pt = event->position();
    pt.setX(pt.x() - (width() / 2 - 200));
    pt.setY(pt.y() - (height() / 2 - 230));

    // Reverse order for z-index click detection
    for (int i = regions.size() - 1; i >= 0; --i) {
        if (regions[i].path.contains(pt) && regions[i].id != 0) { // Exclude body click
            emit mouseButtonClicked(regions[i].id, regions[i].name);
            break;
        }
    }
}

void MouseViewWidget::mouseMoveEvent(QMouseEvent *event) {
    QPointF pt = event->position();
    pt.setX(pt.x() - (width() / 2 - 200));
    pt.setY(pt.y() - (height() / 2 - 230));

    int newHoveredRegion = -1;
    for (int i = regions.size() - 1; i >= 0; --i) {
        if (regions[i].path.contains(pt) && regions[i].id != 0) {
            newHoveredRegion = regions[i].id;
            break;
        }
    }

    if (newHoveredRegion != hoveredRegion) {
        hoveredRegion = newHoveredRegion;
        update();
    }
}
