#pragma once

struct Point3D {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    Point3D() = default;
    Point3D(double xValue, double yValue, double zValue)
        : x(xValue), y(yValue), z(zValue)
    {
    }
};
