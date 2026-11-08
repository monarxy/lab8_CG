#include "Camera.h"
#include <cmath>

Camera::Camera() : position(0, 0, 8), direction(0, 0, -1) {
    setPerspective(0.82, 1.0, 0.1, 100.0);
}

void Camera::orbit(double yawDegrees, double pitchDegrees, double distance) {
    const double pi = 3.14159265358979323846;
    const double yaw = yawDegrees * pi / 180.0;
    const double pitch = pitchDegrees * pi / 180.0;
    position = Point3D(distance * std::cos(pitch) * std::sin(yaw),
                       distance * std::sin(pitch),
                       distance * std::cos(pitch) * std::cos(yaw));
    direction = Point3D(-position.x, -position.y, -position.z);
    const double length = std::sqrt(direction.x*direction.x + direction.y*direction.y + direction.z*direction.z);
    if (length > 1e-12) { direction.x/=length; direction.y/=length; direction.z/=length; }
}

void Camera::setPerspective(double fov, double aspect, double nearPlane, double farPlane) {
    projectionMatrix = Matrix4();
    const double f = 1.0 / std::tan(fov * 0.5);
    projectionMatrix.m[0][0] = f / aspect;
    projectionMatrix.m[1][1] = f;
    projectionMatrix.m[2][2] = (farPlane + nearPlane) / (nearPlane - farPlane);
    projectionMatrix.m[2][3] = (2.0 * farPlane * nearPlane) / (nearPlane - farPlane);
    projectionMatrix.m[3][2] = -1.0;
}
