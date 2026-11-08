#include "Transformations.h"

#include <cmath>

namespace Transformations {

Matrix4 scaleAroundCenter(const Point3D& center, double factor)
{
    return Matrix4::translation(center.x, center.y, center.z)
           * Matrix4::scaling(factor, factor, factor)
           * Matrix4::translation(-center.x, -center.y, -center.z);
}

Matrix4 rotateAroundCenterAxis(const Point3D& center, int axis, double angle)
{
    Matrix4 rotation = Matrix4::rotationZ(angle);
    if (axis == 0) {
        rotation = Matrix4::rotationX(angle);
    } else if (axis == 1) {
        rotation = Matrix4::rotationY(angle);
    }

    return Matrix4::translation(center.x, center.y, center.z)
           * rotation
           * Matrix4::translation(-center.x, -center.y, -center.z);
}

Matrix4 rotateAroundLine(const Point3D& a, const Point3D& b, double angle)
{
    double x = b.x - a.x;
    double y = b.y - a.y;
    double z = b.z - a.z;

    const double length = std::sqrt(x * x + y * y + z * z);
    if (length < 1e-10) {
        return Matrix4::identity();
    }

    x /= length;
    y /= length;
    z /= length;

    const double cosine = std::cos(angle);
    const double sine = std::sin(angle);
    const double t = 1.0 - cosine;

    Matrix4 rotation = Matrix4::identity();
    rotation.m[0][0] = t * x * x + cosine;
    rotation.m[0][1] = t * x * y - sine * z;
    rotation.m[0][2] = t * x * z + sine * y;
    rotation.m[1][0] = t * x * y + sine * z;
    rotation.m[1][1] = t * y * y + cosine;
    rotation.m[1][2] = t * y * z - sine * x;
    rotation.m[2][0] = t * x * z - sine * y;
    rotation.m[2][1] = t * y * z + sine * x;
    rotation.m[2][2] = t * z * z + cosine;

    return Matrix4::translation(a.x, a.y, a.z)
           * rotation
           * Matrix4::translation(-a.x, -a.y, -a.z);
}

} // namespace Transformations
