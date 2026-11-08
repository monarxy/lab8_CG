#pragma once

#include "../geometry/Point3D.h"
#include "../math/Matrix4.h"

class Camera {
public:
    Point3D position;
    Point3D direction;

    Matrix4 projectionMatrix;

    Camera();

    void orbit(
        double yawDegrees,
        double pitchDegrees,
        double distance
        );

    void setPerspective(
        double verticalFovRadians,
        double aspect,
        double nearPlane,
        double farPlane
        );

    void setOrthographic(
        double viewHeight,
        double aspect,
        double nearPlane,
        double farPlane
        );
};