#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <QColor>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QVector3D>

namespace ObjLoader {

// One polygon of the mouse mesh. buttonId identifies the clickable zone:
// 0 = body, 1 = left, 2 = right, 3 = wheel, 4 = forward, 5 = back, 6 = DPI, 7 = logo.
struct MeshFace {
    int buttonId = 0;
    QString name;
    QVector<QVector3D> vertices;
    QColor color; // optional; invalid = pick a default from buttonId
};

struct MeshData {
    QVector<MeshFace> faces;
    QString format;          // "obj" or "stl"
    int triangleCount = 0;
    QStringList groupNames;  // object/group names found in the file
};

// Refuse meshes larger than this: the view is software-rendered with QPainter.
constexpr int MaxTriangles = 40000;

// Loads .obj (with o/g/usemtl names mapped to buttons) or .stl (ASCII or binary).
bool load(const QString &path, MeshData &out, QString *error = nullptr);
bool parseObj(const QByteArray &data, MeshData &out, QString *error = nullptr);
bool parseStl(const QByteArray &data, MeshData &out, QString *error = nullptr);

// Re-orients a mesh so height -> +Y and length -> +Z, centers it, puts it on y = 0
// and scales it so its length equals targetLength. flip rotates it 180 degrees.
void normalize(MeshData &mesh, float targetLength, bool flip);

// Writes faces as OBJ, naming each object "mousy_btn_<id>_<Name>" so it round-trips.
bool saveObj(const QString &path, const QVector<MeshFace> &faces, QString *error = nullptr);

// Maps an OBJ object/group/material name to a button zone (0 if unknown).
int buttonIdForGroupName(const QString &name);
QString buttonName(int buttonId);

} // namespace ObjLoader

using MeshFace = ObjLoader::MeshFace;
using MeshData = ObjLoader::MeshData;

#endif // OBJLOADER_H
