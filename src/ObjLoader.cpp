#include "ObjLoader.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDataStream>
#include <QRegularExpression>
#include <cmath>
#include <limits>

namespace ObjLoader {

int buttonIdForGroupName(const QString &name) {
    QString lower = name.toLower();
    if (lower.contains("left") || lower.contains("lmb") || lower.contains("btn_1") || lower.contains("button1")) return 1;
    if (lower.contains("right") || lower.contains("rmb") || lower.contains("btn_2") || lower.contains("button2")) return 2;
    if (lower.contains("wheel") || lower.contains("scroll") || lower.contains("btn_3") || lower.contains("middle")) return 3;
    if (lower.contains("forward") || lower.contains("m4") || lower.contains("btn_4") || lower.contains("thumb_f")) return 4;
    if (lower.contains("back") || lower.contains("m5") || lower.contains("btn_5") || lower.contains("thumb_b")) return 5;
    if (lower.contains("dpi") || lower.contains("cpi") || lower.contains("btn_6")) return 6;
    if (lower.contains("logo") || lower.contains("rgb") || lower.contains("btn_7")) return 7;
    return 0; // Chassis body
}

QString buttonName(int buttonId) {
    switch (buttonId) {
        case 1: return "Left Click";
        case 2: return "Right Click";
        case 3: return "Scroll Wheel";
        case 4: return "Forward Button";
        case 5: return "Back Button";
        case 6: return "DPI Switch";
        case 7: return "RGB Logo";
        default: return "Body Chassis";
    }
}

bool load(const QString &path, MeshData &out, QString *error) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (error) *error = QString("Could not open file: %1").arg(path);
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QString ext = QFileInfo(path).suffix().toLower();
    if (ext == "stl") {
        return parseStl(data, out, error);
    } else {
        return parseObj(data, out, error);
    }
}

bool parseObj(const QByteArray &data, MeshData &out, QString *error) {
    out.faces.clear();
    out.groupNames.clear();
    out.format = "obj";
    out.triangleCount = 0;

    QVector<QVector3D> vertices;
    vertices.reserve(10000);

    int currentBtnId = 0;
    QString currentGroupName = "Body";

    QTextStream stream(data);
    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#')) continue;

        if (line.startsWith("v ")) {
            QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            if (parts.size() >= 4) {
                vertices.append(QVector3D(parts[1].toFloat(), parts[2].toFloat(), parts[3].toFloat()));
            }
        } else if (line.startsWith("o ") || line.startsWith("g ") || line.startsWith("usemtl ")) {
            QString name = line.section(' ', 1).trimmed();
            if (!name.isEmpty()) {
                currentGroupName = name;
                currentBtnId = buttonIdForGroupName(name);
                if (!out.groupNames.contains(name)) out.groupNames.append(name);
            }
        } else if (line.startsWith("f ")) {
            QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            QVector<int> faceIndices;
            for (int i = 1; i < parts.size(); ++i) {
                QString idxStr = parts[i].section('/', 0, 0);
                int idx = idxStr.toInt();
                if (idx < 0) idx = vertices.size() + idx + 1; // negative index support
                if (idx > 0 && idx <= vertices.size()) {
                    faceIndices.append(idx - 1);
                }
            }

            if (faceIndices.size() >= 3) {
                // Fan triangulation for n-gons
                for (int i = 1; i < faceIndices.size() - 1; ++i) {
                    if (out.triangleCount >= MaxTriangles) {
                        if (error) *error = QString("Mesh exceeds maximum limit of %1 triangles.").arg(MaxTriangles);
                        return false;
                    }

                    MeshFace face;
                    face.buttonId = currentBtnId;
                    face.name = (currentBtnId > 0) ? buttonName(currentBtnId) : currentGroupName;
                    face.vertices.append(vertices[faceIndices[0]]);
                    face.vertices.append(vertices[faceIndices[i]]);
                    face.vertices.append(vertices[faceIndices[i + 1]]);
                    out.faces.append(face);
                    out.triangleCount++;
                }
            }
        }
    }

    if (out.faces.isEmpty()) {
        if (error) *error = "No valid faces found in OBJ file.";
        return false;
    }

    return true;
}

bool parseStl(const QByteArray &data, MeshData &out, QString *error) {
    out.faces.clear();
    out.groupNames.clear();
    out.format = "stl";
    out.triangleCount = 0;

    // Check if binary STL: minimum 84 bytes (80 header + 4 count)
    if (data.size() >= 84) {
        quint32 triCount = 0;
        memcpy(&triCount, data.constData() + 80, 4);
        quint64 expectedSize = 84 + static_cast<quint64>(triCount) * 50;

        if (expectedSize == static_cast<quint64>(data.size()) && triCount > 0) {
            if (triCount > static_cast<quint32>(MaxTriangles)) {
                if (error) *error = QString("STL has %1 triangles, exceeding limit of %2").arg(triCount).arg(MaxTriangles);
                return false;
            }

            const char *ptr = data.constData() + 84;
            for (quint32 i = 0; i < triCount; ++i) {
                ptr += 12; // skip normal (3 * float)
                float v[9];
                memcpy(v, ptr, 36);
                ptr += 36;
                ptr += 2; // skip attribute byte count

                MeshFace face;
                face.buttonId = 0;
                face.name = "Body";
                face.vertices.append(QVector3D(v[0], v[1], v[2]));
                face.vertices.append(QVector3D(v[3], v[4], v[5]));
                face.vertices.append(QVector3D(v[6], v[7], v[8]));
                out.faces.append(face);
                out.triangleCount++;
            }
            return true;
        }
    }

    // ASCII STL fallback
    QTextStream stream(data);
    QVector<QVector3D> currentTri;
    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        if (line.startsWith("vertex ")) {
            QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            if (parts.size() >= 4) {
                currentTri.append(QVector3D(parts[1].toFloat(), parts[2].toFloat(), parts[3].toFloat()));
            }
            if (currentTri.size() == 3) {
                if (out.triangleCount >= MaxTriangles) {
                    if (error) *error = QString("STL exceeds limit of %1 triangles").arg(MaxTriangles);
                    return false;
                }
                MeshFace face;
                face.buttonId = 0;
                face.name = "Body";
                face.vertices = currentTri;
                out.faces.append(face);
                out.triangleCount++;
                currentTri.clear();
            }
        }
    }

    if (out.faces.isEmpty()) {
        if (error) *error = "Failed to parse STL file or no triangles found.";
        return false;
    }

    return true;
}

void normalize(MeshData &mesh, float targetLength, bool flip) {
    if (mesh.faces.isEmpty()) return;

    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::lowest();

    for (const auto &f : mesh.faces) {
        for (const auto &v : f.vertices) {
            if (v.x() < minX) minX = v.x();
            if (v.x() > maxX) maxX = v.x();
            if (v.y() < minY) minY = v.y();
            if (v.y() > maxY) maxY = v.y();
            if (v.z() < minZ) minZ = v.z();
            if (v.z() > maxZ) maxZ = v.z();
        }
    }

    float sizeX = maxX - minX;
    float sizeY = maxY - minY;
    float sizeZ = maxZ - minZ;

    // Detect primary length axis (usually Z or Y or X)
    float maxDim = std::max({sizeX, sizeY, sizeZ});
    if (maxDim <= 0.0001f) return;

    float scale = targetLength / maxDim;
    float centerX = (minX + maxX) * 0.5f;
    float centerZ = (minZ + maxZ) * 0.5f;

    for (auto &f : mesh.faces) {
        for (auto &v : f.vertices) {
            float x = (v.x() - centerX) * scale;
            float y = (v.y() - minY) * scale; // align bottom to Y=0
            float z = (v.z() - centerZ) * scale;

            if (flip) {
                x = -x;
                z = -z;
            }
            v = QVector3D(x, y, z);
        }
    }
}

bool saveObj(const QString &path, const QVector<MeshFace> &faces, QString *error) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (error) *error = QString("Could not write to file: %1").arg(path);
        return false;
    }

    QTextStream out(&file);
    out << "# Mousy 3D Mouse Model Export\n";
    out << "# Generated by Mousy - https://github.com/MattiaFermani/mousy\n\n";

    int vertexOffset = 1;
    int currentId = -1;

    for (const auto &f : faces) {
        if (f.buttonId != currentId) {
            currentId = f.buttonId;
            QString gname = QString("mousy_btn_%1_%2").arg(currentId).arg(f.name.simplified().replace(' ', '_'));
            out << "o " << gname << "\n";
            out << "g " << gname << "\n";
        }

        for (const auto &v : f.vertices) {
            out << "v " << v.x() << " " << v.y() << " " << v.z() << "\n";
        }

        out << "f";
        for (int i = 0; i < f.vertices.size(); ++i) {
            out << " " << (vertexOffset + i);
        }
        out << "\n";
        vertexOffset += f.vertices.size();
    }

    file.close();
    return true;
}

} // namespace ObjLoader
