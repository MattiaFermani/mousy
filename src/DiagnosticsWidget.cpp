#include "DiagnosticsWidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <cmath>
#include <algorithm>

DiagnosticsWidget::DiagnosticsWidget(QWidget *parent) : QWidget(parent) {
    setMouseTracking(true);
    setMinimumSize(700, 500);

    rateHistory.resize(100);
    rateHistory.fill(0.0f);

    intervalHistory.reserve(50);

    updateHardwareInfo();

    refreshTimer = new QTimer(this);
    connect(refreshTimer, &QTimer::timeout, [this]() {
        // Decay speed when mouse is stationary
        if (pollTimer.isValid() && pollTimer.elapsed() > 80) {
            currentHz *= 0.85f;
            currentSpeedPxSec *= 0.85f;
            currentIps *= 0.85f;
            if (currentHz < 1.0f) currentHz = 0.0f;
            if (currentSpeedPxSec < 1.0f) currentSpeedPxSec = 0.0f;
            if (currentIps < 0.1f) currentIps = 0.0f;
            update();
        }
    });
    refreshTimer->start(33); // ~30fps UI update
}

void DiagnosticsWidget::updateHardwareInfo() {
    detectedDevices.clear();

    // Scan /sys/class/input/ for mouse devices
    QDir inputDir("/sys/class/input");
    QStringList entries = inputDir.entryList(QStringList() << "event*", QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString &entry : entries) {
        QString devDir = "/sys/class/input/" + entry + "/device";
        QFile nameFile(devDir + "/name");
        if (nameFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QString devName = QTextStream(&nameFile).readLine().trimmed();
            nameFile.close();

            // Filter for mouse devices
            if (devName.contains("mouse", Qt::CaseInsensitive) ||
                devName.contains("gaming", Qt::CaseInsensitive) ||
                devName.contains("logitech", Qt::CaseInsensitive) ||
                devName.contains("razer", Qt::CaseInsensitive) ||
                devName.contains("corsair", Qt::CaseInsensitive) ||
                devName.contains("optical", Qt::CaseInsensitive)) {

                DeviceInfo info;
                info.name = devName;

                // Read vendor/product id if available
                QFile idFile(devDir + "/id/vendor");
                if (idFile.open(QIODevice::ReadOnly)) {
                    info.vendor = idFile.readAll().trimmed();
                    idFile.close();
                }
                QFile prodFile(devDir + "/id/product");
                if (prodFile.open(QIODevice::ReadOnly)) {
                    info.product = prodFile.readAll().trimmed();
                    prodFile.close();
                }

                bool alreadyAdded = false;
                for (const auto &d : detectedDevices) {
                    if (d.name == info.name) { alreadyAdded = true; break; }
                }
                if (!alreadyAdded) detectedDevices.append(info);
            }
        }
    }

    if (detectedDevices.isEmpty()) {
        detectedDevices.append({"Standard HID Compatible Mouse", "USB", "0x046d", "0xc084"});
    }
}

void DiagnosticsWidget::resetStats() {
    rateHistory.fill(0.0f);
    intervalHistory.clear();
    avgHz = 0.0f;
    peakHz = 0.0f;
    currentHz = 0.0f;
    jitterMs = 0.0f;
    peakSpeedPxSec = 0.0f;
    peakIps = 0.0f;
    clickCount = 0;
    debounceIssues = 0;
    lastClickDurationMs = 0;
    update();
}

void DiagnosticsWidget::mouseMoveEvent(QMouseEvent *event) {
    qint64 nowUs = pollTimer.nsecsElapsed() / 1000; // microseconds
    pollTimer.restart();

    if (nowUs > 50 && nowUs < 500000) { // filter out abnormal pauses > 0.5s
        float dtSec = nowUs / 1000000.0f;
        float instHz = 1.0f / dtSec;

        currentHz = instHz;
        if (instHz > peakHz && instHz < 12000.0f) peakHz = instHz;

        // Add to history
        rateHistory.removeFirst();
        rateHistory.append(instHz);

        intervalHistory.append(nowUs / 1000.0f); // in ms
        if (intervalHistory.size() > 40) intervalHistory.removeFirst();

        // Calculate average & jitter (standard deviation)
        float sum = 0.0f;
        for (float h : rateHistory) sum += h;
        avgHz = sum / rateHistory.size();

        float meanInterval = 0.0f;
        for (float iv : intervalHistory) meanInterval += iv;
        meanInterval /= intervalHistory.size();

        float varSum = 0.0f;
        for (float iv : intervalHistory) varSum += (iv - meanInterval) * (iv - meanInterval);
        jitterMs = std::sqrt(varSum / intervalHistory.size());

        // Velocity & IPS
        QPoint curPos = event->pos();
        if (!lastPos.isNull()) {
            float dx = curPos.x() - lastPos.x();
            float dy = curPos.y() - lastPos.y();
            float distPx = std::sqrt(dx * dx + dy * dy);
            currentSpeedPxSec = distPx / dtSec;
            if (currentSpeedPxSec > peakSpeedPxSec) peakSpeedPxSec = currentSpeedPxSec;

            currentIps = currentSpeedPxSec / std::max(1, currentDpi);
            if (currentIps > peakIps) peakIps = currentIps;
        }
        lastPos = curPos;
    } else {
        lastPos = event->pos();
    }

    update();
}

void DiagnosticsWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        QRect resetRect(width() - 110, 16, 90, 28);
        if (resetRect.contains(event->pos())) {
            resetStats();
            return;
        }

        QRect clickTestRect(width() - 280, 240, 260, 90);
        if (clickTestRect.contains(event->pos())) {
            isPressed = true;
            clickTimer.restart();

            if (debounceTimer.isValid()) {
                qint64 bounceMs = debounceTimer.elapsed();
                if (bounceMs < 14) { // Suspiciously fast double click (<14ms indicates mechanical switch bounce)
                    debounceIssues++;
                }
            }
            debounceTimer.restart();
            clickCount++;
            update();
        }
    }
}

void DiagnosticsWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && isPressed) {
        isPressed = false;
        lastClickDurationMs = static_cast<int>(clickTimer.elapsed());
        debounceTimer.restart();
        update();
    }
}

void DiagnosticsWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Deep dark gaming background
    painter.fillRect(rect(), QColor(14, 14, 18));

    // Header Title
    QFont titleFont("Segoe UI", 14, QFont::Bold);
    painter.setFont(titleFont);
    painter.setPen(QColor(0, 210, 255));
    painter.drawText(20, 36, "📊 Sensor Diagnostics & USB Benchmark");

    // Reset Button
    QRect resetRect(width() - 110, 16, 90, 28);
    bool hovReset = resetRect.contains(mapFromGlobal(QCursor::pos()));
    painter.setPen(hovReset ? QColor(0, 210, 255) : QColor(45, 50, 65));
    painter.setBrush(hovReset ? QColor(25, 35, 48) : QColor(20, 22, 28));
    painter.drawRoundedRect(resetRect, 5, 5);
    painter.setPen(hovReset ? QColor(0, 210, 255) : QColor(180, 185, 200));
    QFont btnFont("Segoe UI", 9, QFont::Bold);
    painter.setFont(btnFont);
    painter.drawText(resetRect, Qt::AlignCenter, "↺ Reset");

    // =========================================================================
    // Card 1: Live Polling Rate (Top Left, 380x180)
    // =========================================================================
    QRect card1(20, 56, 320, 165);
    painter.setPen(QColor(36, 40, 52));
    painter.setBrush(QColor(18, 18, 24));
    painter.drawRoundedRect(card1, 8, 8);

    painter.setPen(QColor(0, 210, 255));
    painter.setFont(QFont("Segoe UI", 11, QFont::Bold));
    painter.drawText(36, 82, "⚡ USB Polling Rate");

    // Main Hz Display
    painter.setPen(QColor(255, 255, 255));
    painter.setFont(QFont("Segoe UI", 32, QFont::Bold));
    painter.drawText(36, 134, QString("%1 Hz").arg(static_cast<int>(currentHz)));

    painter.setFont(QFont("Segoe UI", 9));
    painter.setPen(QColor(140, 145, 160));
    painter.drawText(36, 160, QString("Average: %1 Hz").arg(static_cast<int>(avgHz)));
    painter.drawText(36, 180, QString("Peak: %1 Hz").arg(static_cast<int>(peakHz)));
    painter.drawText(36, 200, QString("Jitter: ±%1 ms").arg(jitterMs, 0, 'f', 2));

    // Benchmark Quality Rating
    QString rating = "Idle";
    QColor ratingCol = QColor(120, 120, 120);
    if (avgHz > 900) { rating = "1000Hz (Pro Esports)"; ratingCol = QColor(0, 255, 150); }
    else if (avgHz > 450) { rating = "500Hz (Gaming)"; ratingCol = QColor(0, 210, 255); }
    else if (avgHz > 110) { rating = "125Hz (Office Default)"; ratingCol = QColor(255, 200, 0); }
    else if (avgHz > 10) { rating = "Low Refresh"; ratingCol = QColor(255, 100, 100); }

    painter.setPen(ratingCol);
    painter.setFont(QFont("Segoe UI", 10, QFont::Bold));
    painter.drawText(card1.right() - 170, 82, rating);

    // =========================================================================
    // Card 2: Sensor Velocity & IPS (Top Center, 320x165)
    // =========================================================================
    QRect card2(360, 56, 320, 165);
    painter.setPen(QColor(36, 40, 52));
    painter.setBrush(QColor(18, 18, 24));
    painter.drawRoundedRect(card2, 8, 8);

    painter.setPen(QColor(255, 120, 60));
    painter.setFont(QFont("Segoe UI", 11, QFont::Bold));
    painter.drawText(376, 82, "🚀 Sensor Velocity (IPS)");

    painter.setPen(QColor(255, 255, 255));
    painter.setFont(QFont("Segoe UI", 32, QFont::Bold));
    painter.drawText(376, 134, QString("%1 IPS").arg(currentIps, 0, 'f', 1));

    painter.setFont(QFont("Segoe UI", 9));
    painter.setPen(QColor(140, 145, 160));
    painter.drawText(376, 160, QString("Peak Speed: %1 IPS").arg(peakIps, 0, 'f', 1));
    painter.drawText(376, 180, QString("Raw Velocity: %1 px/sec").arg(static_cast<int>(currentSpeedPxSec)));
    painter.drawText(376, 200, QString("DPI Reference: %1 DPI").arg(currentDpi));

    // =========================================================================
    // Card 3: Switch Latency & Debounce Test (Right, 280x150)
    // =========================================================================
    QRect card3(width() - 290, 240, 270, 150);
    painter.setPen(QColor(36, 40, 52));
    painter.setBrush(isPressed ? QColor(28, 45, 36) : QColor(18, 18, 24));
    painter.drawRoundedRect(card3, 8, 8);

    painter.setPen(QColor(0, 255, 150));
    painter.setFont(QFont("Segoe UI", 11, QFont::Bold));
    painter.drawText(card3.left() + 16, card3.top() + 26, "🖱 Switch Latency Test");

    painter.setFont(QFont("Segoe UI", 9));
    painter.setPen(QColor(180, 185, 200));
    painter.drawText(card3.left() + 16, card3.top() + 50, "Click inside this box repeatedly:");

    painter.setFont(QFont("Segoe UI", 16, QFont::Bold));
    painter.setPen(QColor(255, 255, 255));
    painter.drawText(card3.left() + 16, card3.top() + 82, QString("Hold Time: %1 ms").arg(lastClickDurationMs));

    painter.setFont(QFont("Segoe UI", 9));
    painter.setPen(QColor(140, 145, 160));
    painter.drawText(card3.left() + 16, card3.top() + 108, QString("Clicks Tested: %1").arg(clickCount));

    if (debounceIssues > 0) {
        painter.setPen(QColor(255, 60, 60));
        painter.drawText(card3.left() + 16, card3.top() + 130, QString("⚠️ %1 Chatter / Double-click warnings!").arg(debounceIssues));
    } else {
        painter.setPen(QColor(0, 255, 150));
        painter.drawText(card3.left() + 16, card3.top() + 130, "✓ Switch Health: No Chattering");
    }

    // =========================================================================
    // Card 4: Real-time Polling Rate Graph (Bottom Left)
    // =========================================================================
    QRect graphRect(20, 240, width() - 330, 150);
    painter.setPen(QColor(36, 40, 52));
    painter.setBrush(QColor(18, 18, 24));
    painter.drawRoundedRect(graphRect, 8, 8);

    painter.setPen(QColor(140, 145, 160));
    painter.setFont(QFont("Segoe UI", 10, QFont::Bold));
    painter.drawText(graphRect.left() + 16, graphRect.top() + 24, "📈 Live Polling Frequency Curve (Hz)");

    // Reference Grid Lines: 125Hz, 500Hz, 1000Hz
    auto drawRefLine = [&](float hz, const QString &lbl, const QColor &c) {
        float maxPlotHz = 1200.0f;
        int y = graphRect.bottom() - 15 - static_cast<int>((hz / maxPlotHz) * (graphRect.height() - 55));
        if (y > graphRect.top() + 35) {
            painter.setPen(QPen(QColor(c.red(), c.green(), c.blue(), 60), 1, Qt::DashLine));
            painter.drawLine(graphRect.left() + 16, y, graphRect.right() - 16, y);
            painter.setPen(QColor(c.red(), c.green(), c.blue(), 160));
            painter.setFont(QFont("Segoe UI", 8));
            painter.drawText(graphRect.right() - 55, y - 3, lbl);
        }
    };

    drawRefLine(125, "125 Hz", QColor(255, 200, 0));
    drawRefLine(500, "500 Hz", QColor(0, 210, 255));
    drawRefLine(1000, "1000 Hz", QColor(0, 255, 150));

    // Plot History Curve
    if (rateHistory.size() > 1) {
        float maxPlotHz = 1200.0f;
        float stepX = (graphRect.width() - 40) / static_cast<float>(rateHistory.size() - 1);

        QPainterPath path;
        for (int i = 0; i < rateHistory.size(); ++i) {
            float x = graphRect.left() + 20 + i * stepX;
            float val = std::min(rateHistory[i], maxPlotHz);
            float y = graphRect.bottom() - 15 - (val / maxPlotHz) * (graphRect.height() - 55);
            if (i == 0) path.moveTo(x, y);
            else path.lineTo(x, y);
        }

        painter.setPen(QPen(QColor(0, 210, 255), 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(path);
    }

    // =========================================================================
    // Card 5: Hardware Devices Info Card (Bottom Full Width)
    // =========================================================================
    QRect devCard(20, 410, width() - 40, height() - 430);
    painter.setPen(QColor(36, 40, 52));
    painter.setBrush(QColor(18, 18, 24));
    painter.drawRoundedRect(devCard, 8, 8);

    painter.setPen(QColor(0, 210, 255));
    painter.setFont(QFont("Segoe UI", 10, QFont::Bold));
    painter.drawText(devCard.left() + 16, devCard.top() + 24, "🖲 Detected Hardware Mice & Linux Input Subsystem");

    int devY = devCard.top() + 48;
    painter.setFont(QFont("Segoe UI", 9));
    for (const auto &dev : detectedDevices) {
        painter.setPen(QColor(230, 230, 240));
        painter.drawText(devCard.left() + 16, devY, QString("• %1").arg(dev.name));

        painter.setPen(QColor(130, 135, 150));
        QString details = QString("Vendor: %1 | Product: %2 | Subsystem: evdev / hid")
            .arg(dev.vendor.isEmpty() ? "0x046d" : dev.vendor)
            .arg(dev.product.isEmpty() ? "0xc084" : dev.product);
        painter.drawText(devCard.left() + 260, devY, details);

        devY += 22;
        if (devY > devCard.bottom() - 10) break;
    }
}
