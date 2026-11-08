#include "Polyhedron.h"

#include <cmath>

namespace {

Point3D cross(const Point3D& a, const Point3D& b)
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

double dot(const Point3D& a, const Point3D& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Point3D subtract(const Point3D& a, const Point3D& b)
{
    return {
        a.x - b.x,
        a.y - b.y,
        a.z - b.z
    };
}

}

void Polyhedron::applyMatrix(const Matrix4& matrix)
{
    bool normalsExist = true;

    for (const Polygon& face : faces) {
        const double length = std::sqrt(
            face.normal.x * face.normal.x
            + face.normal.y * face.normal.y
            + face.normal.z * face.normal.z
            );

        if (length < 1e-12) {
            normalsExist = false;
            break;
        }
    }

    if (!normalsExist) {
        calculateNormals();
    }

    for (Point3D& point : vertices) {
        point = matrix.transformPoint(point);
    }

    for (Polygon& face : faces) {
        face.normal = matrix.transformNormal(face.normal);
    }
}

Point3D Polyhedron::center() const
{
    if (vertices.isEmpty()) {
        return {};
    }

    Point3D result;

    for (const Point3D& point : vertices) {
        result.x += point.x;
        result.y += point.y;
        result.z += point.z;
    }

    const double count = vertices.size();

    result.x /= count;
    result.y /= count;
    result.z /= count;

    return result;
}

void Polyhedron::normalizeSize(double targetRadius)
{
    if (vertices.isEmpty()) {
        return;
    }

    const Point3D centerPoint = center();

    double maxRadius = 0.0;

    for (const Point3D& point : vertices) {
        const double dx = point.x - centerPoint.x;
        const double dy = point.y - centerPoint.y;
        const double dz = point.z - centerPoint.z;

        maxRadius = qMax(
            maxRadius,
            std::sqrt(dx * dx + dy * dy + dz * dz)
            );
    }

    if (maxRadius > 1e-9) {
        const double scale = targetRadius / maxRadius;

        applyMatrix(
            Matrix4::translation(
                -centerPoint.x,
                -centerPoint.y,
                -centerPoint.z
                )
            * Matrix4::scaling(scale, scale, scale)
            );
    }
}

void Polyhedron::calculateNormals()
{

    const Point3D objectCenter = center();

    for (Polygon& face : faces) {
        face.normal = {};

        if (face.indices.size() < 3) {
            continue;
        }

        const int i0 = face.indices[0];
        const int i1 = face.indices[1];
        const int i2 = face.indices[2];

        if (i0 < 0 || i1 < 0 || i2 < 0 ||
            i0 >= vertices.size() ||
            i1 >= vertices.size() ||
            i2 >= vertices.size()) {
            continue;
        }

        const Point3D a = subtract(
            vertices[i1],
            vertices[i0]
            );

        const Point3D b = subtract(
            vertices[i2],
            vertices[i0]
            );

        Point3D normal = cross(a, b);

        const Point3D faceCenter = {
            (vertices[i0].x + vertices[i1].x + vertices[i2].x) / 3.0,
            (vertices[i0].y + vertices[i1].y + vertices[i2].y) / 3.0,
            (vertices[i0].z + vertices[i1].z + vertices[i2].z) / 3.0
        };

        if (dot(normal, subtract(faceCenter, objectCenter)) < 0.0) {
            normal.x = -normal.x;
            normal.y = -normal.y;
            normal.z = -normal.z;
        }

        const double length = std::sqrt(dot(normal, normal));

        if (length > 1e-12) {
            face.normal.x = normal.x / length;
            face.normal.y = normal.y / length;
            face.normal.z = normal.z / length;
        }
    }
}