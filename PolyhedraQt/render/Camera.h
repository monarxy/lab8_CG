#pragma once
#include "../geometry/Point3D.h"
#include "../math/Matrix4.h"

// Камера хранит положение, направление взгляда и матрицу перспективной проекции.
class Camera {
public:
    Point3D position;
    Point3D direction;
    Matrix4 projectionMatrix;

    Camera();
    void orbit(double yawDegrees, double pitchDegrees, double distance);
    void setPerspective(double verticalFovRadians, double aspect, double nearPlane, double farPlane);
};
