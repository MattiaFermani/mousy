#ifndef DIAGNOSTICSWIDGET_H
#define DIAGNOSTICSWIDGET_H

#include <QWidget>
#include <QElapsedTimer>
#include <QTimer>
#include <QVector>
#include <QLabel>
#include <QPushButton>

class DiagnosticsWidget : public QWidget {
    Q_OBJECT

public:
    explicit DiagnosticsWidget(QWidget *parent = nullptr);
    void setDpi(int dpi) { currentDpi = dpi; }

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    int currentDpi = 1600;

    // Polling rate benchmark
    QElapsedTimer pollTimer;
    QVector<float> intervalHistory;
    QVector<float> rateHistory;
    float currentHz = 0.0f;
    float avgHz = 0.0f;
    float peakHz = 0.0f;
    float jitterMs = 0.0f;

    // Velocity & IPS benchmark
    QPoint lastPos;
    float currentSpeedPxSec = 0.0f;
    float peakSpeedPxSec = 0.0f;
    float currentIps = 0.0f;
    float peakIps = 0.0f;

    // Switch latency & debounce
    QElapsedTimer clickTimer;
    QElapsedTimer debounceTimer;
    qint64 lastClickReleaseTime = 0;
    int lastClickDurationMs = 0;
    int clickCount = 0;
    int debounceIssues = 0;
    bool isPressed = false;

    // Hardware mouse devices
    struct DeviceInfo {
        QString name;
        QString bus;
        QString vendor;
        QString product;
    };
    QList<DeviceInfo> detectedDevices;

    QTimer *refreshTimer;

    void updateHardwareInfo();
    void resetStats();
};

#endif // DIAGNOSTICSWIDGET_H
