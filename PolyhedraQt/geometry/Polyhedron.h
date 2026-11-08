#pragma once

#include <QVector>

#include "Point3D.h"
#include "Polygon.h"
#include "../math/Matrix4.h"

class Polyhedron {
public:
    QVector<Point3D> vertices;
    QVector<Polygon> faces;

    void applyMatrix(const Matrix4& matrix);
    void calculateNormals();
    Point3D center() const;
    void normalizeSize(double targetRadius = 1.6);
};
