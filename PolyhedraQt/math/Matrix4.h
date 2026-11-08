#pragma once

#include <array>

#include "../geometry/Point3D.h"

class Matrix4 {
public:
    std::array<std::array<double, 4>, 4> m{};

    Matrix4();

    static Matrix4 identity();
    static Matrix4 translation(double x, double y, double z);
    static Matrix4 scaling(double x, double y, double z);
    static Matrix4 rotationX(double radians);
    static Matrix4 rotationY(double radians);
    static Matrix4 rotationZ(double radians);
    static Matrix4 reflectionXY();
    static Matrix4 reflectionXZ();
    static Matrix4 reflectionYZ();
    Matrix4 operator*(const Matrix4& other) const;
    Point3D transformPoint(const Point3D& point) const;
};
