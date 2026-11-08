#include "PolyhedronFactory.h"

#include <QString>

#include <algorithm>
#include <cmath>

namespace {

QColor faceColor(int index)
{
    static const QColor colors[] = {
        QColor(90, 160, 210),
        QColor(236, 164, 75),
        QColor(112, 190, 145),
        QColor(190, 135, 205),
        QColor(220, 115, 120),
        QColor(100, 185, 190)
    };

    return colors[index % 6];
}

double distanceSquared(const Point3D& a, const Point3D& b)
{
    const double x = a.x - b.x;
    const double y = a.y - b.y;
    const double z = a.z - b.z;
    return x * x + y * y + z * z;
}

Polyhedron createIcosahedron()
{
    Polyhedron polyhedron;
    const double goldenRatio = (1.0 + std::sqrt(5.0)) / 2.0;

    polyhedron.vertices = {
        {-1, goldenRatio, 0}, {1, goldenRatio, 0},
        {-1, -goldenRatio, 0}, {1, -goldenRatio, 0},
        {0, -1, goldenRatio}, {0, 1, goldenRatio},
        {0, -1, -goldenRatio}, {0, 1, -goldenRatio},
        {goldenRatio, 0, -1}, {goldenRatio, 0, 1},
        {-goldenRatio, 0, -1}, {-goldenRatio, 0, 1}
    };

    double shortestEdgeSquared = 1e9;
    for (int i = 0; i < polyhedron.vertices.size(); ++i) {
        for (int j = i + 1; j < polyhedron.vertices.size(); ++j) {
            shortestEdgeSquared = std::min(
                shortestEdgeSquared,
                distanceSquared(polyhedron.vertices[i], polyhedron.vertices[j]));
        }
    }

    int faceIndex = 0;
    for (int i = 0; i < polyhedron.vertices.size(); ++i) {
        for (int j = i + 1; j < polyhedron.vertices.size(); ++j) {
            for (int k = j + 1; k < polyhedron.vertices.size(); ++k) {
                const double ij = distanceSquared(polyhedron.vertices[i], polyhedron.vertices[j]);
                const double ik = distanceSquared(polyhedron.vertices[i], polyhedron.vertices[k]);
                const double jk = distanceSquared(polyhedron.vertices[j], polyhedron.vertices[k]);

                if (std::abs(ij - shortestEdgeSquared) < 1e-6
                    && std::abs(ik - shortestEdgeSquared) < 1e-6
                    && std::abs(jk - shortestEdgeSquared) < 1e-6) {
                    polyhedron.faces.append(
                        Polygon({i, j, k}, faceColor(faceIndex++)));
                }
            }
        }
    }

    polyhedron.normalizeSize();
    return polyhedron;
}

Polyhedron createDodecahedron()
{
    Polyhedron polyhedron;
    const Polyhedron icosahedron = createIcosahedron();

    for (const Polygon& face : icosahedron.faces) {
        Point3D center;
        for (int vertexIndex : face.indices) {
            center.x += icosahedron.vertices[vertexIndex].x;
            center.y += icosahedron.vertices[vertexIndex].y;
            center.z += icosahedron.vertices[vertexIndex].z;
        }

        const double count = face.indices.size();
        center.x /= count;
        center.y /= count;
        center.z /= count;
        polyhedron.vertices.append(center);
    }

    int faceIndex = 0;
    for (int vertexIndex = 0; vertexIndex < icosahedron.vertices.size(); ++vertexIndex) {
        QVector<int> surroundingFaces;
        for (int faceIndexInIcosahedron = 0;
             faceIndexInIcosahedron < icosahedron.faces.size();
             ++faceIndexInIcosahedron) {
            if (icosahedron.faces[faceIndexInIcosahedron].indices.contains(vertexIndex)) {
                surroundingFaces.append(faceIndexInIcosahedron);
            }
        }

        const Point3D center = icosahedron.vertices[vertexIndex];
        const double normalLength = std::sqrt(
            center.x * center.x + center.y * center.y + center.z * center.z);
        const Point3D normal(center.x / normalLength,
                             center.y / normalLength,
                             center.z / normalLength);

        Point3D u(std::abs(normal.x) < 0.8 ? 1.0 : 0.0,
                  std::abs(normal.x) < 0.8 ? 0.0 : 1.0,
                  0.0);
        const double dot = u.x * normal.x + u.y * normal.y + u.z * normal.z;
        u.x -= dot * normal.x;
        u.y -= dot * normal.y;
        u.z -= dot * normal.z;

        const double uLength = std::sqrt(u.x * u.x + u.y * u.y + u.z * u.z);
        u.x /= uLength;
        u.y /= uLength;
        u.z /= uLength;

        const Point3D v(normal.y * u.z - normal.z * u.y,
                        normal.z * u.x - normal.x * u.z,
                        normal.x * u.y - normal.y * u.x);

        const auto angleAroundVertex = [&](int faceIndexInIcosahedron) {
            const Point3D point = polyhedron.vertices[faceIndexInIcosahedron];
            const double dx = point.x - center.x;
            const double dy = point.y - center.y;
            const double dz = point.z - center.z;
            const double alongV = dx * v.x + dy * v.y + dz * v.z;
            const double alongU = dx * u.x + dy * u.y + dz * u.z;
            return std::atan2(alongV, alongU);
        };

        std::sort(surroundingFaces.begin(), surroundingFaces.end(),
                  [&](int a, int b) {
                      return angleAroundVertex(a) < angleAroundVertex(b);
                  });

        polyhedron.faces.append(Polygon(surroundingFaces, faceColor(faceIndex++)));
    }

    polyhedron.normalizeSize();
    return polyhedron;
}

} // namespace

Polyhedron PolyhedronFactory::create(Type type)
{
    if (type == Icosahedron) {
        return createIcosahedron();
    }
    if (type == Dodecahedron) {
        return createDodecahedron();
    }

    Polyhedron polyhedron;

    if (type == Tetrahedron) {
        polyhedron.vertices = {
            {1, 1, 1}, {-1, -1, 1}, {-1, 1, -1}, {1, -1, -1}
        };
        polyhedron.faces = {
            Polygon({0, 1, 2}, faceColor(0)),
            Polygon({0, 3, 1}, faceColor(1)),
            Polygon({0, 2, 3}, faceColor(2)),
            Polygon({1, 3, 2}, faceColor(3))
        };
    } else if (type == Hexahedron) {
        polyhedron.vertices = {
            {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
            {-1, -1, 1}, {1, -1, 1}, {1, 1, 1}, {-1, 1, 1}
        };
        polyhedron.faces = {
            Polygon({0, 3, 2, 1}, faceColor(0)),
            Polygon({4, 5, 6, 7}, faceColor(1)),
            Polygon({0, 1, 5, 4}, faceColor(2)),
            Polygon({1, 2, 6, 5}, faceColor(3)),
            Polygon({2, 3, 7, 6}, faceColor(4)),
            Polygon({3, 0, 4, 7}, faceColor(5))
        };
    } else if (type == Octahedron) {
        polyhedron.vertices = {
            {1, 0, 0}, {-1, 0, 0}, {0, 1, 0},
            {0, -1, 0}, {0, 0, 1}, {0, 0, -1}
        };
        polyhedron.faces = {
            Polygon({0, 2, 4}, faceColor(0)),
            Polygon({2, 1, 4}, faceColor(1)),
            Polygon({1, 3, 4}, faceColor(2)),
            Polygon({3, 0, 4}, faceColor(3)),
            Polygon({2, 0, 5}, faceColor(4)),
            Polygon({1, 2, 5}, faceColor(5)),
            Polygon({3, 1, 5}, faceColor(0)),
            Polygon({0, 3, 5}, faceColor(1))
        };
    }

    polyhedron.normalizeSize();
    return polyhedron;
}

QString PolyhedronFactory::name(Type type)
{
    switch (type) {
    case Tetrahedron:
        return "Тетраэдр";
    case Hexahedron:
        return "Гексаэдр (куб)";
    case Octahedron:
        return "Октаэдр";
    case Icosahedron:
        return "Икосаэдр";
    case Dodecahedron:
        return "Додекаэдр";
    }

    return {};
}
