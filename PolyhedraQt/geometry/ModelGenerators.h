#pragma once
#include "Polyhedron.h"
#include <QString>

class ModelGenerators {
public:
    static bool revolution(const QVector<Point3D>& profile, int axis, int segments,
                           Polyhedron& result, QString* error = nullptr);
    static bool surface(int functionIndex, double minValue, double maxValue,
                        int divisions, Polyhedron& result, QString* error = nullptr);
};
