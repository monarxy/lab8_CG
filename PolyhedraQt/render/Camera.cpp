#include "Camera.h"

#include <cmath>

Camera::Camera()
    : position(0.0, 0.0, 8.0),
    direction(0.0, 0.0, -1.0)
{
    setPerspective(
        0.82,
        1.0,
        0.1,
        100.0
        );
}

void Camera::orbit(
    double yawDegrees,
    double pitchDegrees,
    double distance
    )
{
    const double pi = 3.14159265358979323846;

    const double yaw = yawDegrees * pi / 180.0;
    const double pitch = pitchDegrees * pi / 180.0;

    position.x = distance * std::cos(pitch) * std::sin(yaw);
    position.y = distance * std::sin(pitch);
    position.z = distance * std::cos(pitch) * std::cos(yaw);

    direction.x = -position.x;
    direction.y = -position.y;
    direction.z = -position.z;

    const double length = std::sqrt(
        direction.x * direction.x
        + direction.y * direction.y
        + direction.z * direction.z
        );

    if (length > 1e-12) {
        direction.x /= length;
        direction.y /= length;
        direction.z /= length;
    }
}

void Camera::setPerspective(
    double verticalFovRadians,
    double aspect,
    double nearPlane,
    double farPlane
    )
{
    projectionMatrix = Matrix4();

    const double f = 1.0 / std::tan(verticalFovRadians * 0.5);

    projectionMatrix.m[0][0] = f / aspect;
    projectionMatrix.m[1][1] = f;

    projectionMatrix.m[2][2] =
        (farPlane + nearPlane) / (nearPlane - farPlane);

    projectionMatrix.m[2][3] =
        (2.0 * farPlane * nearPlane) / (nearPlane - farPlane);

    projectionMatrix.m[3][2] = -1.0;
}

void Camera::setOrthographic(
    double viewHeight,
    double aspect,
    double nearPlane,
    double farPlane
    )
{
    projectionMatrix = Matrix4();

    const double viewWidth = viewHeight * aspect;

    projectionMatrix.m[0][0] = 2.0 / viewWidth;
    projectionMatrix.m[1][1] = 2.0 / viewHeight;

    projectionMatrix.m[2][2] =
        -2.0 / (farPlane - nearPlane);

    projectionMatrix.m[2][3] =
        -(farPlane + nearPlane) / (farPlane - nearPlane);

    projectionMatrix.m[3][3] = 1.0;
}