#ifndef MOUSEVIEWWIDGET_H
#define MOUSEVIEWWIDGET_H

#include <QWidget>
#include <QVector3D>
#include <QMatrix4x4>
#include <QPolygonF>
#include <QTimer>
#include <QMap>
#include <QVector>
#include <QElapsedTimer>
#include "ObjLoader.h"

class MouseViewWidget : public QWidget {
    Q_OBJECT

public:
    enum RgbEffect {
        NeonCyan = 0,
        RainbowCycle = 1,
        CyberpunkPink = 2,
        MatrixGreen = 3,
        CrimsonFire = 4,
        CustomColor = 5
    };
    Q_ENUM(RgbEffect)

    enum ViewPreset {
        Isometric = 0,
        Top = 1,
        LeftFlank = 2,
        RightFlank = 3,
        Front = 4
    };
    Q_ENUM(ViewPreset)

    explicit MouseViewWidget(QWidget *parent = nullptr);

    void setBindings(const QMap<int, QString> &bindings);
    void setRgbEffect(RgbEffect effect);
    void setDpi(int dpi);
    void setCustomZoneColor(int zoneId, const QColor &color);
    QColor customZoneColor(int zoneId) const;

    bool loadCustomMesh(const QString &path, QString *error = nullptr);
    bool exportCurrentMesh(const QString &path, QString *error = nullptr);
    void resetToDefaultMesh();

    bool isAutoRotateEnabled() const { return autoRotate; }

public slots:
    void setRotation(float pitch, float yaw, float roll = 0.0f);
    void setZoom(float zoom);
    void resetView();
    void setPreset(ViewPreset preset);
    void setAutoRotate(bool enabled);

signals:
    void mouseButtonClicked(int buttonId, const QString& buttonName);
    void viewChanged(float pitch, float yaw, float zoom);
    void meshLoaded(const QString &fileName, int triangleCount);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    struct Face3D {
        int buttonId;
        QString name;
        QVector<QVector3D> vertices;
        QColor baseColor;
    };

    struct ProjectedFace {
        int buttonId;
        QString name;
        QPolygonF poly2D;
        float depth;
        QColor shadedColor;
        QVector3D normal;
        QPointF center2D;
    };

    struct Led3D {
        QVector3D pos;
        float t;
    };

    QVector<Face3D> faces;
    QVector<Led3D> leds;

    // Camera & Transform state
    float pitch = 25.0f;  // X-axis rotation
    float yaw = -35.0f;   // Y-axis rotation
    float roll = 0.0f;    // Z-axis rotation
    float zoom = 1.35f;   // Scale factor

    // Momentum / Inertia
    float velPitch = 0.0f;
    float velYaw = 0.0f;
    bool autoRotate = false;

    // Interaction state
    QPoint lastMousePos;
    QPoint pressMousePos;
    QElapsedTimer dragTimer;
    bool isDragging = false;
    int hoveredRegion = -1;
    int selectedRegion = -1;

    // Animation & Settings
    float pulsePhase = 0.0f;
    QMap<int, QString> bindings;
    QMap<int, QColor> zoneColors;
    RgbEffect rgbEffect = NeonCyan;
    int currentDpi = 1600;

    QTimer *animTimer;

    // HUD Buttons in 2D
    struct HudButton {
        QRect rect;
        QString id;
        QString text;
        ViewPreset preset;
    };
    QVector<HudButton> hudButtons;

    void buildDefaultMesh();
    void buildLeds3D();
    QColor calculateLedColor(float t, float phase, float brightness) const;
    void render3D(QPainter &painter);
    void renderHud(QPainter &painter);
};

#endif // MOUSEVIEWWIDGET_H
