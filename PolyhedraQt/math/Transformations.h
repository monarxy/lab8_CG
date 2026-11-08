#pragma once

#include "Matrix4.h"
#include "../geometry/Point3D.h"

namespace Transformations {

Matrix4 rotateAroundCenterAxis(const Point3D& center, int axis, double radians);
Matrix4 scaleAroundCenter(const Point3D& center, double factor);
Matrix4 rotateAroundLine(const Point3D& a, const Point3D& b, double radians);

} // namespace Transformations
