#include "Matrix4.h"

#include <cmath>

Matrix4::Matrix4()
{
    for (auto& row : m) {
        row.fill(0.0);
    }
}

Matrix4 Matrix4::identity()
{
    Matrix4 result;

    for (int i = 0; i < 4; ++i) {
        result.m[i][i] = 1.0;
    }

    return result;
}

Matrix4 Matrix4::translation(double x, double y, double z)
{
    Matrix4 result = identity();

    result.m[0][3] = x;
    result.m[1][3] = y;
    result.m[2][3] = z;

    return result;
}

Matrix4 Matrix4::scaling(double x, double y, double z)
{
    Matrix4 result = identity();

    result.m[0][0] = x;
    result.m[1][1] = y;
    result.m[2][2] = z;

    return result;
}

Matrix4 Matrix4::rotationX(double angle)
{
    Matrix4 result = identity();

    result.m[1][1] = std::cos(angle);
    result.m[1][2] = -std::sin(angle);
    result.m[2][1] = std::sin(angle);
    result.m[2][2] = std::cos(angle);

    return result;
}

Matrix4 Matrix4::rotationY(double angle)
{
    Matrix4 result = identity();

    result.m[0][0] = std::cos(angle);
    result.m[0][2] = std::sin(angle);
    result.m[2][0] = -std::sin(angle);
    result.m[2][2] = std::cos(angle);

    return result;
}

Matrix4 Matrix4::rotationZ(double angle)
{
    Matrix4 result = identity();

    result.m[0][0] = std::cos(angle);
    result.m[0][1] = -std::sin(angle);
    result.m[1][0] = std::sin(angle);
    result.m[1][1] = std::cos(angle);

    return result;
}

Matrix4 Matrix4::reflectionXY()
{
    return scaling(1.0, 1.0, -1.0);
}

Matrix4 Matrix4::reflectionXZ()
{
    return scaling(1.0, -1.0, 1.0);
}

Matrix4 Matrix4::reflectionYZ()
{
    return scaling(-1.0, 1.0, 1.0);
}

Matrix4 Matrix4::operator*(const Matrix4& other) const
{
    Matrix4 result;

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            for (int k = 0; k < 4; ++k) {
                result.m[i][j] += m[i][k] * other.m[k][j];
            }
        }
    }

    return result;
}

Point3D Matrix4::transformPoint(const Point3D& point) const
{
    const double input[4] = {
        point.x,
        point.y,
        point.z,
        1.0
    };

    double output[4] = {
        0.0,
        0.0,
        0.0,
        0.0
    };

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            output[i] += m[i][j] * input[j];
        }
    }

    if (std::abs(output[3]) > 1e-10 &&
        std::abs(output[3] - 1.0) > 1e-10) {

        return {
            output[0] / output[3],
            output[1] / output[3],
            output[2] / output[3]
        };
    }

    return {
        output[0],
        output[1],
        output[2]
    };
}

Point3D Matrix4::transformNormal(const Point3D& normal) const
{
    const double a = m[0][0];
    const double b = m[0][1];
    const double c = m[0][2];

    const double d = m[1][0];
    const double e = m[1][1];
    const double f = m[1][2];

    const double g = m[2][0];
    const double h = m[2][1];
    const double i = m[2][2];

    const double determinant =
        a * (e * i - f * h)
        - b * (d * i - f * g)
        + c * (d * h - e * g);

    if (std::abs(determinant) < 1e-12) {
        return {};
    }

    const double normalMatrix[3][3] = {
        {
            e * i - f * h,
            f * g - d * i,
            d * h - e * g
        },
        {
            c * h - b * i,
            a * i - c * g,
            b * g - a * h
        },
        {
            b * f - c * e,
            c * d - a * f,
            a * e - b * d
        }
    };

    Point3D result;

    result.x = (
                   normalMatrix[0][0] * normal.x
                   + normalMatrix[0][1] * normal.y
                   + normalMatrix[0][2] * normal.z
                   ) / determinant;

    result.y = (
                   normalMatrix[1][0] * normal.x
                   + normalMatrix[1][1] * normal.y
                   + normalMatrix[1][2] * normal.z
                   ) / determinant;

    result.z = (
                   normalMatrix[2][0] * normal.x
                   + normalMatrix[2][1] * normal.y
                   + normalMatrix[2][2] * normal.z
                   ) / determinant;

    const double length = std::sqrt(
        result.x * result.x
        + result.y * result.y
        + result.z * result.z
        );

    if (length > 1e-12) {
        result.x /= length;
        result.y /= length;
        result.z /= length;
    }

    return result;
}