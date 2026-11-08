#include "ModelGenerators.h"
#include <cmath>

namespace {
constexpr double Pi = 3.14159265358979323846;
Point3D rotateAroundAxis(const Point3D& p, int axis, double angle)
{
    const double c = std::cos(angle), s = std::sin(angle);
    if (axis == 0) return Point3D(p.x, p.y*c - p.z*s, p.y*s + p.z*c);
    if (axis == 1) return Point3D(p.x*c + p.z*s, p.y, -p.x*s + p.z*c);
    return Point3D(p.x*c - p.y*s, p.x*s + p.y*c, p.z);
}
}

bool ModelGenerators::revolution(const QVector<Point3D>& profile, int axis, int segments,
                                 Polyhedron& result, QString* error)
{
    if (profile.size() < 2 || segments < 3 || axis < 0 || axis > 2) {
        if (error) *error = "Нужно минимум две точки образующей и не менее трёх разбиений.";
        return false;
    }
    Polyhedron model;
    for (int segment = 0; segment < segments; ++segment) {
        const double angle = 2.0 * Pi * segment / segments;
        for (const Point3D& point : profile)
            model.vertices.append(rotateAroundAxis(point, axis, angle));
    }
    const int profileSize = profile.size();
    for (int segment = 0; segment < segments; ++segment) {
        const int next = (segment + 1) % segments;
        for (int i = 0; i < profileSize - 1; ++i) {
            const int a = segment * profileSize + i;
            const int b = segment * profileSize + i + 1;
            const int c = next * profileSize + i + 1;
            const int d = next * profileSize + i;
            model.faces.append(Polygon({a, b, c, d}));
        }
    }
    result = model;
    return true;
}

bool ModelGenerators::surface(int functionIndex, double minValue, double maxValue,
                              int divisions, Polyhedron& result, QString* error)
{
    if (divisions < 2 || maxValue <= minValue || functionIndex < 0 || functionIndex > 3) {
        if (error) *error = "Проверьте диапазон и количество разбиений (не менее двух).";
        return false;
    }
    Polyhedron model;
    const int side = divisions + 1;
    const double step = (maxValue - minValue) / divisions;
    for (int iy = 0; iy <= divisions; ++iy) {
        const double y = minValue + iy * step;
        for (int ix = 0; ix <= divisions; ++ix) {
            const double x = minValue + ix * step;
            double z = 0.0;
            switch (functionIndex) {
            case 0: z = x * x + y * y; break;
            case 1: z = std::sin(x) * std::cos(y); break;
            case 2: z = x * y; break;
            case 3: z = std::sin(std::sqrt(x*x + y*y)); break;
            }
            model.vertices.append(Point3D(x, y, z));
        }
    }
    for (int iy = 0; iy < divisions; ++iy) {
        for (int ix = 0; ix < divisions; ++ix) {
            const int a = iy * side + ix;
            const int b = a + 1;
            const int d = (iy + 1) * side + ix;
            const int c = d + 1;
            model.faces.append(Polygon({a, b, c, d}, QColor(115, 175, 135)));
        }
    }
    result = model;
    return true;
}
