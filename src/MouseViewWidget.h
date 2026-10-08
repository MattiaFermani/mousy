#ifndef MOUSEVIEWWIDGET_H
#define MOUSEVIEWWIDGET_H

#include <QWidget>
#include <QPainterPath>
#include <QTimer>
#include <QMap>

class MouseViewWidget : public QWidget {
    Q_OBJECT

public:
    enum RgbEffect {
        NeonCyan = 0,
        RainbowCycle = 1,
        CyberpunkPink = 2,
        MatrixGreen = 3,
        CrimsonFire = 4
    };
    Q_ENUM(RgbEffect)

    explicit MouseViewWidget(QWidget *parent = nullptr);
    void setBindings(const QMap<int, QString> &bindings);
    void setRgbEffect(RgbEffect effect);
    void setDpi(int dpi);

signals:
    void mouseButtonClicked(int buttonId, const QString& buttonName);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    struct MouseRegion {
        int id;
        QPainterPath path;
        QColor color;
        QString name;
    };

    QList<MouseRegion> regions;
    int hoveredRegion = -1;
    int selectedRegion = -1;
    float pulsePhase = 0;
    QMap<int, QString> bindings;
    RgbEffect rgbEffect = NeonCyan;
    int currentDpi = 1600;

    QTimer *animTimer;

    void setupRegions();
    QColor calculateLedColor(float t, float phase, float brightness) const;
};

#endif // MOUSEVIEWWIDGET_H
