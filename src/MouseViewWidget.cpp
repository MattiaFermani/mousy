#include "MouseViewWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QFileDialog>
#include <QMessageBox>
#include <cmath>
#include <algorithm>

MouseViewWidget::MouseViewWidget(QWidget *parent) : QWidget(parent) {
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setMinimumSize(420, 420);

    // Default zone colors
    zoneColors[0] = QColor(24, 24, 30);  // Body
    zoneColors[1] = QColor(38, 38, 48);  // LMB
    zoneColors[2] = QColor(38, 38, 48);  // RMB
    zoneColors[3] = QColor(16, 16, 20);  // Wheel
    zoneColors[4] = QColor(0, 210, 255); // Underglow

    buildDefaultMesh();
    buildLeds3D();

    animTimer = new QTimer(this);
    connect(animTimer, &QTimer::timeout, [this]() {
        pulsePhase += 0.05f;
        if (pulsePhase > 2.0f * static_cast<float>(M_PI)) pulsePhase -= 2.0f * static_cast<float>(M_PI);

        // Momentum / Auto-rotation physics
        if (autoRotate) {
            yaw += 0.45f;
            if (yaw > 180.0f) yaw -= 360.0f;
            update();
        } else if (!isDragging && (std::abs(velYaw) > 0.02f || std::abs(velPitch) > 0.02f)) {
            yaw += velYaw;
            pitch += velPitch;
            pitch = std::clamp(pitch, -85.0f, 85.0f);
            if (yaw > 180.0f) yaw -= 360.0f;
            if (yaw < -180.0f) yaw += 360.0f;
            velYaw *= 0.91f;   // Friction deceleration
            velPitch *= 0.91f;
            update();
        } else {
            update();
        }
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

void MouseViewWidget::setCustomZoneColor(int zoneId, const QColor &color) {
    zoneColors[zoneId] = color;
    update();
}

QColor MouseViewWidget::customZoneColor(int zoneId) const {
    return zoneColors.value(zoneId, QColor(0, 210, 255));
}

void MouseViewWidget::setRotation(float p, float y, float r) {
    pitch = std::clamp(p, -85.0f, 85.0f);
    yaw = y;
    roll = r;
    emit viewChanged(pitch, yaw, zoom);
    update();
}

void MouseViewWidget::setZoom(float z) {
    zoom = std::clamp(z, 0.5f, 3.0f);
    emit viewChanged(pitch, yaw, zoom);
    update();
}

void MouseViewWidget::resetView() {
    setPreset(Isometric);
    velPitch = 0.0f;
    velYaw = 0.0f;
}

void MouseViewWidget::setAutoRotate(bool enabled) {
    autoRotate = enabled;
    update();
}

void MouseViewWidget::setPreset(ViewPreset preset) {
    switch (preset) {
        case Top:
            pitch = 85.0f; yaw = 0.0f; roll = 0.0f; zoom = 1.35f;
            break;
        case LeftFlank:
            pitch = 10.0f; yaw = -90.0f; roll = 0.0f; zoom = 1.45f;
            break;
        case RightFlank:
            pitch = 10.0f; yaw = 90.0f; roll = 0.0f; zoom = 1.45f;
            break;
        case Front:
            pitch = 0.0f; yaw = 0.0f; roll = 0.0f; zoom = 1.45f;
            break;
        case Isometric:
        default:
            pitch = 28.0f; yaw = -38.0f; roll = 0.0f; zoom = 1.35f;
            break;
    }
    velPitch = 0.0f;
    velYaw = 0.0f;
    emit viewChanged(pitch, yaw, zoom);
    update();
}

bool MouseViewWidget::loadCustomMesh(const QString &path, QString *error) {
    ObjLoader::MeshData mesh;
    if (!ObjLoader::load(path, mesh, error)) {
        return false;
    }

    ObjLoader::normalize(mesh, 175.0f, false);

    faces.clear();
    for (const auto &mf : mesh.faces) {
        Face3D f;
        f.buttonId = mf.buttonId;
        f.name = mf.name.isEmpty() ? ObjLoader::buttonName(mf.buttonId) : mf.name;
        f.vertices = mf.vertices;
        f.baseColor = mf.color.isValid() ? mf.color : (mf.buttonId > 0 ? QColor(42, 42, 54) : QColor(26, 26, 32));
        faces.append(f);
    }

    emit meshLoaded(QFileInfo(path).fileName(), mesh.triangleCount);
    update();
    return true;
}

bool MouseViewWidget::exportCurrentMesh(const QString &path, QString *error) {
    QVector<MeshFace> exportFaces;
    for (const auto &f : faces) {
        MeshFace mf;
        mf.buttonId = f.buttonId;
        mf.name = f.name;
        mf.vertices = f.vertices;
        mf.color = f.baseColor;
        exportFaces.append(mf);
    }
    return ObjLoader::saveObj(path, exportFaces, error);
}

void MouseViewWidget::resetToDefaultMesh() {
    buildDefaultMesh();
    update();
}

void MouseViewWidget::buildDefaultMesh() {
    faces.clear();

    QColor darkShell = zoneColors.value(0, QColor(24, 24, 30));
    QColor buttonShell = zoneColors.value(1, QColor(38, 38, 48));
    QColor sideButtonColor = QColor(45, 45, 58);
    QColor wheelColor = zoneColors.value(3, QColor(16, 16, 20));

    // 1. LEFT BUTTON (LMB, ID = 1)
    faces.append({1, "Left Click", {
        QVector3D(-42, 22, 10),
        QVector3D(-6,  24, 10),
        QVector3D(-7,  16, 85),
        QVector3D(-38, 14, 80)
    }, buttonShell});

    faces.append({1, "Left Click", {
        QVector3D(-38, 14, 80),
        QVector3D(-7,  16, 85),
        QVector3D(-7,   6, 88),
        QVector3D(-36,  5, 83)
    }, buttonShell.darker(115)});

    faces.append({1, "Left Click", {
        QVector3D(-42, 22, 10),
        QVector3D(-38, 14, 80),
        QVector3D(-44,  8, 75),
        QVector3D(-48, 12, 10)
    }, buttonShell.darker(125)});

    faces.append({1, "Left Click", {
        QVector3D(-6, 24, 10),
        QVector3D(-7, 16, 85),
        QVector3D(-5, 12, 85),
        QVector3D(-4, 18, 10)
    }, darkShell.darker(130)});

    // 2. RIGHT BUTTON (RMB, ID = 2)
    faces.append({2, "Right Click", {
        QVector3D(6,   24, 10),
        QVector3D(42,  22, 10),
        QVector3D(38,  14, 80),
        QVector3D(7,   16, 85)
    }, buttonShell});

    faces.append({2, "Right Click", {
        QVector3D(7,   16, 85),
        QVector3D(38,  14, 80),
        QVector3D(36,   5, 83),
        QVector3D(7,    6, 88)
    }, buttonShell.darker(115)});

    faces.append({2, "Right Click", {
        QVector3D(42,  22, 10),
        QVector3D(48,  12, 10),
        QVector3D(44,   8, 75),
        QVector3D(38,  14, 80)
    }, buttonShell.darker(125)});

    faces.append({2, "Right Click", {
        QVector3D(6,  24, 10),
        QVector3D(4,  18, 10),
        QVector3D(5,  12, 85),
        QVector3D(7,  16, 85)
    }, darkShell.darker(130)});

    // 3. SCROLL WHEEL (ID = 3)
    int wheelSegments = 8;
    float wR = 12.0f;
    float wCenterY = 17.0f;
    float wCenterZ = 45.0f;
    float wHalfW = 4.2f;

    for (int i = 0; i < wheelSegments; ++i) {
        float a1 = static_cast<float>(i) * 2.0f * static_cast<float>(M_PI) / wheelSegments;
        float a2 = static_cast<float>(i + 1) * 2.0f * static_cast<float>(M_PI) / wheelSegments;

        float z1 = wCenterZ + std::cos(a1) * wR;
        float y1 = wCenterY + std::sin(a1) * wR;
        float z2 = wCenterZ + std::cos(a2) * wR;
        float y2 = wCenterY + std::sin(a2) * wR;

        faces.append({3, "Scroll Wheel", {
            QVector3D(-wHalfW, y1, z1),
            QVector3D( wHalfW, y1, z1),
            QVector3D( wHalfW, y2, z2),
            QVector3D(-wHalfW, y2, z2)
        }, (i % 2 == 0) ? wheelColor : wheelColor.lighter(135)});

        faces.append({3, "Scroll Wheel", {
            QVector3D(-wHalfW, wCenterY, wCenterZ),
            QVector3D(-wHalfW, y1, z1),
            QVector3D(-wHalfW, y2, z2)
        }, QColor(80, 85, 95)});

        faces.append({3, "Scroll Wheel", {
            QVector3D(wHalfW, wCenterY, wCenterZ),
            QVector3D(wHalfW, y2, z2),
            QVector3D(wHalfW, y1, z1)
        }, QColor(80, 85, 95)});
    }

    // 4. DPI BUTTON (ID = 6)
    faces.append({6, "DPI Switch", {
        QVector3D(-5, 27, -4),
        QVector3D( 5, 27, -4),
        QVector3D( 5, 26,  16),
        QVector3D(-5, 26,  16)
    }, QColor(60, 42, 75)});

    faces.append({6, "DPI Switch", {
        QVector3D(-5, 27, -4),
        QVector3D(-5, 26,  16),
        QVector3D(-7, 23,  16),
        QVector3D(-7, 24,  -4)
    }, QColor(45, 30, 58)});

    faces.append({6, "DPI Switch", {
        QVector3D( 5, 27, -4),
        QVector3D( 7, 24, -4),
        QVector3D( 7, 23,  16),
        QVector3D( 5, 26,  16)
    }, QColor(45, 30, 58)});

    // 5. SIDE BUTTONS (Forward M4 = 4, Back M5 = 5)
    faces.append({4, "Forward Button", {
        QVector3D(-47, 18, 12),
        QVector3D(-44, 21, 14),
        QVector3D(-42, 19, 44),
        QVector3D(-45, 16, 42)
    }, sideButtonColor});

    faces.append({4, "Forward Button", {
        QVector3D(-45, 16, 42),
        QVector3D(-42, 19, 44),
        QVector3D(-43, 14, 43),
        QVector3D(-46, 12, 41)
    }, sideButtonColor.darker(120)});

    faces.append({5, "Back Button", {
        QVector3D(-49, 16, -24),
        QVector3D(-46, 19, -22),
        QVector3D(-45, 18,   8),
        QVector3D(-48, 15,   6)
    }, sideButtonColor});

    faces.append({5, "Back Button", {
        QVector3D(-48, 15,   6),
        QVector3D(-45, 18,   8),
        QVector3D(-46, 13,   7),
        QVector3D(-49, 11,   5)
    }, sideButtonColor.darker(120)});

    // 6. BODY & PALM REST (ID = 0)
    faces.append({0, "Palm Rest", {
        QVector3D(-38, 25, 10),
        QVector3D( 38, 25, 10),
        QVector3D( 36, 36, -30),
        QVector3D(-36, 36, -30)
    }, darkShell.lighter(115)});

    faces.append({0, "Palm Rest", {
        QVector3D(-36, 36, -30),
        QVector3D( 36, 36, -30),
        QVector3D( 30, 22, -75),
        QVector3D(-30, 22, -75)
    }, darkShell.lighter(105)});

    faces.append({0, "Body Chassis", {
        QVector3D(-30, 22, -75),
        QVector3D( 30, 22, -75),
        QVector3D( 24,  4, -92),
        QVector3D(-24,  4, -92)
    }, darkShell});

    faces.append({0, "Thumb Rest", {
        QVector3D(-48, 15, 10),
        QVector3D(-38, 25, 10),
        QVector3D(-36, 36, -30),
        QVector3D(-50, 18, -30)
    }, darkShell.darker(105)});

    faces.append({0, "Thumb Rest Wing", {
        QVector3D(-50, 18, -30),
        QVector3D(-36, 36, -30),
        QVector3D(-30, 22, -75),
        QVector3D(-45,  6, -65)
    }, darkShell.darker(110)});

    faces.append({0, "Chassis Base", {
        QVector3D(-52,  2, 10),
        QVector3D(-48, 15, 10),
        QVector3D(-50, 18, -30),
        QVector3D(-54,  2, -30)
    }, darkShell.darker(120)});

    faces.append({0, "Right Flank", {
        QVector3D(38, 25, 10),
        QVector3D(48, 14, 10),
        QVector3D(46, 16, -30),
        QVector3D(36, 36, -30)
    }, darkShell.darker(105)});

    faces.append({0, "Right Flank", {
        QVector3D(36, 36, -30),
        QVector3D(46, 16, -30),
        QVector3D(38,  6, -65),
        QVector3D(30, 22, -75)
    }, darkShell.darker(110)});

    faces.append({0, "Chassis Base", {
        QVector3D(48, 14, 10),
        QVector3D(50,  2, 10),
        QVector3D(48,  2, -30),
        QVector3D(46, 16, -30)
    }, darkShell.darker(120)});

    faces.append({0, "Chassis Base", {
        QVector3D(-38, 1, 80),
        QVector3D( 38, 1, 80),
        QVector3D( 48, 1, -30),
        QVector3D( 24, 1, -92),
        QVector3D(-24, 1, -92),
        QVector3D(-52, 1, -30)
    }, darkShell.darker(140)});
}

void MouseViewWidget::buildLeds3D() {
    leds.clear();
    QVector<QVector3D> rim = {
        QVector3D(  0, 2,  88),
        QVector3D( 20, 2,  82),
        QVector3D( 38, 2,  70),
        QVector3D( 46, 2,  40),
        QVector3D( 49, 2,  10),
        QVector3D( 48, 2, -25),
        QVector3D( 42, 2, -55),
        QVector3D( 28, 2, -80),
        QVector3D(  0, 2, -94),
        QVector3D(-28, 2, -80),
        QVector3D(-42, 2, -55),
        QVector3D(-50, 2, -25),
        QVector3D(-53, 2,  10),
        QVector3D(-48, 2,  40),
        QVector3D(-38, 2,  70),
        QVector3D(-20, 2,  82)
    };

    int total = rim.size();
    for (int i = 0; i < total; ++i) {
        Led3D led;
        led.pos = rim[i];
        led.t = static_cast<float>(i) / total;
        leds.append(led);
    }
}

QColor MouseViewWidget::calculateLedColor(float t, float phase, float brightness) const {
    if (rgbEffect == CustomColor) {
        QColor c = zoneColors.value(4, QColor(0, 210, 255));
        return QColor(static_cast<int>(c.red() * brightness),
                      static_cast<int>(c.green() * brightness),
                      static_cast<int>(c.blue() * brightness));
    }

    int r = 0, g = 0, b = 0;
    switch (rgbEffect) {
        case RainbowCycle: {
            float hue = std::fmod((phase / (2.0f * static_cast<float>(M_PI)) + t) * 360.0f, 360.0f);
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

    painter.fillRect(rect(), QColor(14, 14, 18));
    render3D(painter);
    renderHud(painter);
}

void MouseViewWidget::render3D(QPainter &painter) {
    float centerX = width() * 0.5f;
    float centerY = height() * 0.5f + 10.0f;
    float camDist = 580.0f;

    QMatrix4x4 mat;
    mat.rotate(pitch, 1, 0, 0);
    mat.rotate(yaw,   0, 1, 0);
    mat.rotate(roll,  0, 0, 1);

    QVector3D lightDir(0.35f, 0.85f, -0.55f);
    lightDir.normalize();

    // Ambient glow
    QColor rgbGlow = calculateLedColor(0.5f, pulsePhase, 0.55f);
    QRadialGradient ambientGlow(centerX, centerY, 190 * zoom);
    ambientGlow.setColorAt(0.0, QColor(rgbGlow.red(), rgbGlow.green(), rgbGlow.blue(), 55));
    ambientGlow.setColorAt(0.6, QColor(rgbGlow.red(), rgbGlow.green(), rgbGlow.blue(), 12));
    ambientGlow.setColorAt(1.0, QColor(0, 0, 0, 0));
    painter.fillRect(rect(), ambientGlow);

    QVector<ProjectedFace> projectedList;
    projectedList.reserve(faces.size());

    for (const auto &face : faces) {
        if (face.vertices.size() < 3) continue;

        QVector<QVector3D> transVerts;
        transVerts.reserve(face.vertices.size());
        QPolygonF poly;
        float depthSum = 0.0f;
        QPointF center2D(0, 0);

        for (const auto &v : face.vertices) {
            QVector3D tv = mat.map(v * zoom);
            transVerts.append(tv);
            depthSum += tv.z();

            float persp = camDist / (camDist - tv.z());
            float px = centerX + tv.x() * persp;
            float py = centerY - tv.y() * persp;
            poly << QPointF(px, py);
            center2D += QPointF(px, py);
        }

        center2D /= static_cast<qreal>(face.vertices.size());
        float avgDepth = depthSum / face.vertices.size();

        QVector3D e1 = transVerts[1] - transVerts[0];
        QVector3D e2 = transVerts[2] - transVerts[0];
        QVector3D normal = QVector3D::crossProduct(e1, e2).normalized();

        float cosTheta = QVector3D::dotProduct(normal, lightDir);
        float diffuse = 0.28f + 0.72f * std::max(0.0f, cosTheta);

        QVector3D viewDir(0, 0, -1);
        QVector3D reflect = 2.0f * QVector3D::dotProduct(normal, lightDir) * normal - lightDir;
        float spec = std::pow(std::max(0.0f, QVector3D::dotProduct(reflect, viewDir)), 14.0f) * 0.35f;

        QColor base = face.baseColor;
        bool isHovered = (hoveredRegion == face.buttonId && face.buttonId > 0);
        bool isSelected = (selectedRegion == face.buttonId && face.buttonId > 0);

        if (isSelected) {
            float p = 0.85f + 0.15f * std::sin(pulsePhase * 2.5f);
            base = base.lighter(static_cast<int>(180 * p));
        } else if (isHovered) {
            base = base.lighter(150);
        }

        int r = std::clamp(static_cast<int>(base.red() * diffuse + 255 * spec), 0, 255);
        int g = std::clamp(static_cast<int>(base.green() * diffuse + 255 * spec), 0, 255);
        int b = std::clamp(static_cast<int>(base.blue() * diffuse + 255 * spec), 0, 255);

        ProjectedFace pf;
        pf.buttonId = face.buttonId;
        pf.name = face.name;
        pf.poly2D = poly;
        pf.depth = avgDepth;
        pf.shadedColor = QColor(r, g, b);
        pf.normal = normal;
        pf.center2D = center2D;
        projectedList.append(pf);
    }

    // Depth Sorting
    std::sort(projectedList.begin(), projectedList.end(), [](const ProjectedFace &a, const ProjectedFace &b) {
        return a.depth < b.depth;
    });

    QColor themeHighlight = calculateLedColor(0.5f, pulsePhase, 1.0f);

    for (const auto &pf : projectedList) {
        bool isHovered = (hoveredRegion == pf.buttonId && pf.buttonId > 0);
        bool isSelected = (selectedRegion == pf.buttonId && pf.buttonId > 0);

        painter.setBrush(pf.shadedColor);

        if (isSelected) {
            float p = 0.5f + 0.5f * std::sin(pulsePhase * 3.0f);
            QColor selBorder = themeHighlight;
            selBorder.setAlpha(static_cast<int>(180 + 75 * p));
            painter.setPen(QPen(selBorder, 2.5));
        } else if (isHovered) {
            QColor hovBorder = themeHighlight;
            hovBorder.setAlpha(170);
            painter.setPen(QPen(hovBorder, 1.8));
        } else {
            painter.setPen(QPen(QColor(18, 18, 22), 1.0));
        }

        painter.drawPolygon(pf.poly2D);

        // Labels
        if (pf.buttonId > 0 && pf.normal.z() < -0.2f) {
            QString labelText;
            if (pf.buttonId == 6) {
                labelText = QString("DPI\n%1").arg(currentDpi);
            } else if (bindings.contains(pf.buttonId)) {
                labelText = pf.name + "\n[" + bindings[pf.buttonId] + "]";
            } else if (isSelected || isHovered) {
                labelText = pf.name;
            }

            if (!labelText.isEmpty()) {
                painter.setPen(isSelected ? themeHighlight : (isHovered ? QColor(240, 240, 240) : QColor(160, 160, 175)));
                QFont f = painter.font();
                f.setPointSize(pf.buttonId == 6 ? 7 : 8);
                f.setBold(true);
                painter.setFont(f);
                QRectF bound = pf.poly2D.boundingRect();
                painter.drawText(bound, Qt::AlignCenter | Qt::TextWordWrap, labelText);
            }
        }
    }

    // 3D RGB LEDs
    for (const auto &led : leds) {
        QVector3D tv = mat.map(led.pos * zoom);
        float persp = camDist / (camDist - tv.z());
        float px = centerX + tv.x() * persp;
        float py = centerY - tv.y() * persp;

        float brightness = 0.45f + 0.55f * std::max(0.0f, std::sin(pulsePhase + led.t * 5.0f));
        QColor col = calculateLedColor(led.t, pulsePhase, brightness);

        float radius = 7.0f * persp * zoom;
        QRadialGradient ledBloom(px, py, radius);
        col.setAlpha(static_cast<int>(170 * brightness));
        ledBloom.setColorAt(0.0, col);
        ledBloom.setColorAt(1.0, QColor(0, 0, 0, 0));
        painter.setPen(Qt::NoPen);
        painter.setBrush(ledBloom);
        painter.drawEllipse(QPointF(px, py), radius, radius);

        col.setAlpha(255);
        painter.setBrush(col);
        painter.drawEllipse(QPointF(px, py), 2.2f * persp, 2.2f * persp);
    }
}

void MouseViewWidget::renderHud(QPainter &painter) {
    hudButtons.clear();

    int btnX = 14;
    int btnY = 14;
    int btnH = 26;

    QVector<QPair<QString, QString>> tools = {
        {"reset", "⟲ Reset"},
        {"top", "Top"},
        {"side", "Side"},
        {"front", "Front"},
        {"orbit", autoRotate ? "⏹ Stop Orbit" : "▶ Orbit"},
        {"import", "📂 Load 3D"},
        {"export", "💾 Export 3D"}
    };

    QFont btnFont("Segoe UI", 8, QFont::Bold);
    painter.setFont(btnFont);

    for (const auto &item : tools) {
        int btnW = (item.first == "reset") ? 60 : (item.first == "orbit" ? 75 : (item.first.contains("port") ? 70 : 44));
        QRect btnRect(btnX, btnY, btnW, btnH);

        bool isHov = btnRect.contains(mapFromGlobal(QCursor::pos()));

        painter.setPen(isHov ? QPen(QColor(0, 210, 255), 1.5) : QPen(QColor(40, 45, 58), 1));
        painter.setBrush(isHov ? QColor(28, 38, 52) : QColor(20, 22, 28, 220));
        painter.drawRoundedRect(btnRect, 5, 5);

        painter.setPen(isHov ? QColor(0, 210, 255) : QColor(190, 195, 205));
        painter.drawText(btnRect, Qt::AlignCenter, item.second);

        HudButton hb;
        hb.rect = btnRect;
        hb.id = item.first;
        hb.text = item.second;
        hb.preset = Isometric;
        if (item.first == "top") hb.preset = Top;
        else if (item.first == "side") hb.preset = LeftFlank;
        else if (item.first == "front") hb.preset = Front;
        hudButtons.append(hb);

        btnX += btnW + 6;
    }

    // Zoom Buttons
    int zoomBtnSize = 26;
    QRect zoomOutRect(width() - 68, 14, zoomBtnSize, btnH);
    QRect zoomInRect(width() - 36, 14, zoomBtnSize, btnH);

    auto drawMiniBtn = [&](const QRect &r, const QString &sym) {
        bool hov = r.contains(mapFromGlobal(QCursor::pos()));
        painter.setPen(hov ? QPen(QColor(0, 210, 255), 1.5) : QPen(QColor(40, 45, 58), 1));
        painter.setBrush(hov ? QColor(28, 38, 52) : QColor(20, 22, 28, 220));
        painter.drawRoundedRect(r, 5, 5);
        painter.setPen(hov ? QColor(0, 210, 255) : QColor(210, 210, 220));
        painter.drawText(r, Qt::AlignCenter, sym);
    };

    drawMiniBtn(zoomOutRect, "−");
    drawMiniBtn(zoomInRect, "+");

    HudButton hbOut{zoomOutRect, "zoom_out", "−", Isometric};
    HudButton hbIn{zoomInRect, "zoom_in", "+", Isometric};
    hudButtons.append(hbOut);
    hudButtons.append(hbIn);

    // Stats bar
    painter.setPen(QColor(130, 135, 150));
    QFont statsFont("Segoe UI", 9);
    painter.setFont(statsFont);

    QString stats = QString("Pitch: %1°  |  Yaw: %2°  |  Zoom: %3%  |  Faces: %4")
        .arg(static_cast<int>(pitch))
        .arg(static_cast<int>(yaw))
        .arg(static_cast<int>(zoom * 100))
        .arg(faces.size());
    painter.drawText(16, height() - 16, stats);

    // Hint banner
    painter.setPen(QColor(90, 95, 110));
    painter.drawText(QRect(0, height() - 38, width() - 16, 20), Qt::AlignRight, "🖱 Drag to Rotate with Momentum • Scroll to Zoom • Click Button to Configure");
}

void MouseViewWidget::mousePressEvent(QMouseEvent *event) {
    pressMousePos = event->pos();
    lastMousePos = event->pos();
    dragTimer.restart();
    velPitch = 0.0f;
    velYaw = 0.0f;

    // Check HUD Buttons
    for (const auto &hb : hudButtons) {
        if (hb.rect.contains(event->pos())) {
            if (hb.id == "zoom_in") {
                setZoom(zoom + 0.15f);
            } else if (hb.id == "zoom_out") {
                setZoom(zoom - 0.15f);
            } else if (hb.id == "reset") {
                resetView();
            } else if (hb.id == "orbit") {
                setAutoRotate(!autoRotate);
            } else if (hb.id == "import") {
                QString path = QFileDialog::getOpenFileName(this, "Import 3D Mouse Model", "", "3D Meshes (*.obj *.stl)");
                if (!path.isEmpty()) {
                    QString err;
                    if (!loadCustomMesh(path, &err)) {
                        QMessageBox::warning(this, "Import Error", err);
                    }
                }
            } else if (hb.id == "export") {
                QString path = QFileDialog::getSaveFileName(this, "Export 3D Mouse Model", "mousy_model.obj", "Wavefront OBJ (*.obj)");
                if (!path.isEmpty()) {
                    QString err;
                    if (!exportCurrentMesh(path, &err)) {
                        QMessageBox::warning(this, "Export Error", err);
                    } else {
                        QMessageBox::information(this, "Export Success", "Model successfully saved to:\n" + path);
                    }
                }
            } else {
                setPreset(hb.preset);
            }
            return;
        }
    }

    isDragging = false;
}

void MouseViewWidget::mouseMoveEvent(QMouseEvent *event) {
    QPoint delta = event->pos() - lastMousePos;

    if (event->buttons() & (Qt::LeftButton | Qt::RightButton | Qt::MiddleButton)) {
        if ((event->pos() - pressMousePos).manhattanLength() > 4) {
            isDragging = true;
            autoRotate = false; // user interaction cancels auto-rotate

            float deltaYaw = delta.x() * 0.45f;
            float deltaPitch = delta.y() * 0.45f;

            yaw += deltaYaw;
            pitch += deltaPitch;
            pitch = std::clamp(pitch, -85.0f, 85.0f);

            // Record velocity for momentum
            velYaw = deltaYaw * 0.8f;
            velPitch = deltaPitch * 0.8f;

            emit viewChanged(pitch, yaw, zoom);
            update();
            lastMousePos = event->pos();
            return;
        }
    }

    lastMousePos = event->pos();

    // Hover picking in 3D
    float centerX = width() * 0.5f;
    float centerY = height() * 0.5f + 10.0f;
    float camDist = 580.0f;

    QMatrix4x4 mat;
    mat.rotate(pitch, 1, 0, 0);
    mat.rotate(yaw,   0, 1, 0);
    mat.rotate(roll,  0, 0, 1);

    QVector<ProjectedFace> projectedList;
    for (const auto &face : faces) {
        if (face.buttonId <= 0) continue;

        QVector<QVector3D> transVerts;
        QPolygonF poly;
        float depthSum = 0.0f;

        for (const auto &v : face.vertices) {
            QVector3D tv = mat.map(v * zoom);
            transVerts.append(tv);
            depthSum += tv.z();
            float persp = camDist / (camDist - tv.z());
            poly << QPointF(centerX + tv.x() * persp, centerY - tv.y() * persp);
        }

        ProjectedFace pf;
        pf.buttonId = face.buttonId;
        pf.name = face.name;
        pf.poly2D = poly;
        pf.depth = depthSum / face.vertices.size();
        projectedList.append(pf);
    }

    std::sort(projectedList.begin(), projectedList.end(), [](const ProjectedFace &a, const ProjectedFace &b) {
        return a.depth > b.depth;
    });

    int newHover = -1;
    for (const auto &pf : projectedList) {
        if (pf.poly2D.containsPoint(event->pos(), Qt::OddEvenFill)) {
            newHover = pf.buttonId;
            break;
        }
    }

    if (newHover != hoveredRegion) {
        hoveredRegion = newHover;
        setCursor(hoveredRegion > 0 ? Qt::PointingHandCursor : Qt::ArrowCursor);
        update();
    }
}

void MouseViewWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (!isDragging && (event->pos() - pressMousePos).manhattanLength() <= 4) {
        float centerX = width() * 0.5f;
        float centerY = height() * 0.5f + 10.0f;
        float camDist = 580.0f;

        QMatrix4x4 mat;
        mat.rotate(pitch, 1, 0, 0);
        mat.rotate(yaw,   0, 1, 0);
        mat.rotate(roll,  0, 0, 1);

        QVector<ProjectedFace> projectedList;
        for (const auto &face : faces) {
            if (face.buttonId <= 0) continue;

            QVector<QVector3D> transVerts;
            QPolygonF poly;
            float depthSum = 0.0f;

            for (const auto &v : face.vertices) {
                QVector3D tv = mat.map(v * zoom);
                transVerts.append(tv);
                depthSum += tv.z();
                float persp = camDist / (camDist - tv.z());
                poly << QPointF(centerX + tv.x() * persp, centerY - tv.y() * persp);
            }

            ProjectedFace pf;
            pf.buttonId = face.buttonId;
            pf.name = face.name;
            pf.poly2D = poly;
            pf.depth = depthSum / face.vertices.size();
            projectedList.append(pf);
        }

        std::sort(projectedList.begin(), projectedList.end(), [](const ProjectedFace &a, const ProjectedFace &b) {
            return a.depth > b.depth;
        });

        for (const auto &pf : projectedList) {
            if (pf.poly2D.containsPoint(event->pos(), Qt::OddEvenFill)) {
                selectedRegion = pf.buttonId;
                emit mouseButtonClicked(pf.buttonId, pf.name);
                update();
                break;
            }
        }
    }

    isDragging = false;
}

void MouseViewWidget::wheelEvent(QWheelEvent *event) {
    float delta = event->angleDelta().y() * 0.0012f;
    setZoom(zoom + delta);
    event->accept();
}

void MouseViewWidget::leaveEvent(QEvent *event) {
    Q_UNUSED(event);
    if (hoveredRegion != -1) {
        hoveredRegion = -1;
        setCursor(Qt::ArrowCursor);
        update();
    }
}
