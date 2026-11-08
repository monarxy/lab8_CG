#pragma once

#include "../geometry/Point3D.h"

// Два способа перевода 3D-точки в 2D согласно формулам из лекции.
enum class ProjectionType {
    Perspective,
    Axonometric
};

struct ProjectedPoint {
    double x = 0.0;
    double y = 0.0;
    double depth = 0.0;
    bool valid = false;
};

namespace Projection {

ProjectedPoint projectPoint(ProjectionType type, const Point3D& point);

} // namespace Projection
