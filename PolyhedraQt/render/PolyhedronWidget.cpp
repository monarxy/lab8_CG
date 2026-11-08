#include "PolyhedronWidget.h"

#include "../geometry/PolyhedronFactory.h"
#include "../math/Matrix4.h"

#include <QPainter>
#include <QImage>
#include <QPaintEvent>
#include <QSizePolicy>

#include <algorithm>
#include <cmath>
#include <limits>

namespace {

struct V3 {
    double x;
    double y;
    double z;
};

V3 subtract(const V3& a, const V3& b)
{
    return {
        a.x - b.x,
        a.y - b.y,
        a.z - b.z
    };
}

V3 cross(const V3& a, const V3& b)
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

double dot(const V3& a, const V3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

V3 normalize(const V3& vector)
{
    const double length = std::sqrt(dot(vector, vector));

    if (length < 1e-12) {
        return {0.0, 0.0, 0.0};
    }

    return {
        vector.x / length,
        vector.y / length,
        vector.z / length
    };
}

struct Vertex {
    double x;
    double y;
    double z;

    QPointF screen;
};

bool hasNormals(const Polyhedron& polyhedron)
{
    for (const Polygon& face : polyhedron.faces) {
        const double length = std::sqrt(
            face.normal.x * face.normal.x
            + face.normal.y * face.normal.y
            + face.normal.z * face.normal.z
            );

        if (length < 1e-12) {
            return false;
        }
    }

    return !polyhedron.faces.isEmpty();
}

}

PolyhedronWidget::PolyhedronWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumSize(280, 280);

    setSizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Expanding
        );

    setAutoFillBackground(true);
    setPalette(QPalette(QColor(248, 249, 251)));

    camera.orbit(
        cameraYaw,
        cameraPitch,
        cameraDistance
        );
}

void PolyhedronWidget::setPolyhedron(const Polyhedron& p)
{
    polyhedron = p;

    // Первоначально вычисляем нормали только тогда,
    // когда они ещё не были заданы.
    if (!hasNormals(polyhedron)) {
        polyhedron.calculateNormals();
    }

    update();
}

void PolyhedronWidget::setProjectionType(ProjectionType type)
{
    projectionType = type;
    update();
}

void PolyhedronWidget::setBackFaceCulling(bool enabled)
{
    cullingEnabled = enabled;
    update();
}

void PolyhedronWidget::setZBufferEnabled(bool enabled)
{
    zBufferEnabled = enabled;
    update();
}

void PolyhedronWidget::setCamera(
    double yawDegrees,
    double pitchDegrees,
    double distance
    )
{
    cameraYaw = yawDegrees;
    cameraPitch = pitchDegrees;
    cameraDistance = distance;

    camera.orbit(
        cameraYaw,
        cameraPitch,
        cameraDistance
        );

    update();
}

void PolyhedronWidget::paintEvent(QPaintEvent*)
{
    QPainter out(this);

    out.setRenderHint(QPainter::Antialiasing, true);
    out.fillRect(rect(), QColor(248, 249, 251));

    if (polyhedron.vertices.isEmpty()) {
        return;
    }

    QVector<Polyhedron> objects;

    objects.append(polyhedron);

    Polyhedron leftPolyhedron =
        PolyhedronFactory::create(
            PolyhedronFactory::Hexahedron
            );

    leftPolyhedron.calculateNormals();

    leftPolyhedron.applyMatrix(
        Matrix4::translation(-3.0, 0.0, 0.0)
        );

    objects.append(leftPolyhedron);

    Polyhedron rightPolyhedron =
        PolyhedronFactory::create(
            PolyhedronFactory::Octahedron
            );

    rightPolyhedron.calculateNormals();

    rightPolyhedron.applyMatrix(
        Matrix4::translation(3.0, 0.0, -0.4)
        );

    objects.append(rightPolyhedron);

    const double aspect =
        width() / double(qMax(1, height()));

    if (projectionType == ProjectionType::Perspective) {
        camera.setPerspective(
            0.82,
            aspect,
            0.1,
            100.0
            );
    } else {
        camera.setOrthographic(
            6.8,
            aspect,
            0.1,
            100.0
            );
    }

    const V3 cameraPosition = {
        camera.position.x,
        camera.position.y,
        camera.position.z
    };

    const V3 forward = normalize({
        camera.direction.x,
        camera.direction.y,
        camera.direction.z
    });

    V3 worldUp = {0.0, 1.0, 0.0};

    if (std::abs(dot(forward, worldUp)) > 0.999) {
        worldUp = {0.0, 0.0, 1.0};
    }

    const V3 right = normalize(
        cross(forward, worldUp)
        );

    const V3 up = normalize(
        cross(right, forward)
        );

    const Matrix4& projection = camera.projectionMatrix;

    QImage image(
        size(),
        QImage::Format_ARGB32
        );

    image.fill(QColor(248, 249, 251));

    QVector<double> zBuffer(
        width() * height(),
        std::numeric_limits<double>::infinity()
        );

    for (const Polyhedron& object : objects) {

        QVector<Vertex> transformedVertices;

        transformedVertices.reserve(
            object.vertices.size()
            );

        for (const Point3D& point : object.vertices) {

            const V3 relative = subtract(
                {
                    point.x,
                    point.y,
                    point.z
                },
                cameraPosition
                );

            const double cameraX = dot(relative, right);
            const double cameraY = dot(relative, up);
            const double cameraZ = dot(relative, forward);

            Vertex vertex;

            vertex.x = cameraX;
            vertex.y = cameraY;
            vertex.z = cameraZ;

            const double clipX =
                projection.m[0][0] * cameraX
                + projection.m[0][1] * cameraY
                + projection.m[0][2] * cameraZ
                + projection.m[0][3];

            const double clipY =
                projection.m[1][0] * cameraX
                + projection.m[1][1] * cameraY
                + projection.m[1][2] * cameraZ
                + projection.m[1][3];

            const double clipW =
                projection.m[3][0] * cameraX
                + projection.m[3][1] * cameraY
                + projection.m[3][2] * cameraZ
                + projection.m[3][3];

            if (std::abs(clipW) < 1e-12) {
                vertex.screen = QPointF(-10000.0, -10000.0);
            } else if (projectionType == ProjectionType::Perspective &&
                       cameraZ <= 0.05) {
                vertex.screen = QPointF(-10000.0, -10000.0);
            } else {
                const double normalizedX = clipX / clipW;
                const double normalizedY = clipY / clipW;

                vertex.screen = QPointF(
                    (normalizedX + 1.0) * width() / 2.0,
                    (1.0 - normalizedY) * height() / 2.0
                    );
            }

            transformedVertices.append(vertex);
        }

        for (const Polygon& face : object.faces) {

            if (face.indices.size() < 3) {
                continue;
            }

            bool indicesValid = true;

            for (int index : face.indices) {
                if (index < 0 ||
                    index >= transformedVertices.size()) {

                    indicesValid = false;
                    break;
                }
            }

            if (!indicesValid) {
                continue;
            }

            V3 faceCenter = {
                0.0,
                0.0,
                0.0
            };

            for (int index : face.indices) {
                const Point3D& point =
                    object.vertices[index];

                faceCenter.x += point.x;
                faceCenter.y += point.y;
                faceCenter.z += point.z;
            }

            const double faceVertexCount =
                face.indices.size();

            faceCenter.x /= faceVertexCount;
            faceCenter.y /= faceVertexCount;
            faceCenter.z /= faceVertexCount;

            V3 viewVector;

            if (projectionType == ProjectionType::Perspective) {

                viewVector = subtract(
                    cameraPosition,
                    faceCenter
                    );

            } else {

                viewVector = {
                    -camera.direction.x,
                    -camera.direction.y,
                    -camera.direction.z
                };
            }

            viewVector = normalize(viewVector);

            const V3 normal = {
                face.normal.x,
                face.normal.y,
                face.normal.z
            };

            const double visibility = dot(normal, viewVector);

            if (cullingEnabled && visibility <= 0.0) {
                continue;
            }

            for (int k = 1; k + 1 < face.indices.size(); ++k) {

                const Vertex& a =
                    transformedVertices[face.indices[0]];

                const Vertex& b =
                    transformedVertices[face.indices[k]];

                const Vertex& c =
                    transformedVertices[face.indices[k + 1]];

                if (a.screen.x() < -1000.0 ||
                    b.screen.x() < -1000.0 ||
                    c.screen.x() < -1000.0) {

                    continue;
                }

                const double x0 = a.screen.x();
                const double y0 = a.screen.y();

                const double x1 = b.screen.x();
                const double y1 = b.screen.y();

                const double x2 = c.screen.x();
                const double y2 = c.screen.y();

                const double area =
                    (x1 - x0) * (y2 - y0)
                    - (y1 - y0) * (x2 - x0);

                if (std::abs(area) < 1e-10) {
                    continue;
                }

                const int minX = qBound(
                    0,
                    static_cast<int>(
                        std::floor(std::min({x0, x1, x2}))
                        ),
                    width() - 1
                    );

                const int maxX = qBound(
                    0,
                    static_cast<int>(
                        std::ceil(std::max({x0, x1, x2}))
                        ),
                    width() - 1
                    );

                const int minY = qBound(
                    0,
                    static_cast<int>(
                        std::floor(std::min({y0, y1, y2}))
                        ),
                    height() - 1
                    );

                const int maxY = qBound(
                    0,
                    static_cast<int>(
                        std::ceil(std::max({y0, y1, y2}))
                        ),
                    height() - 1
                    );

                for (int y = minY; y <= maxY; ++y) {
                    for (int x = minX; x <= maxX; ++x) {

                        const double pixelX = x + 0.5;
                        const double pixelY = y + 0.5;

                        const double w0 =
                            (
                                (x1 - pixelX) * (y2 - pixelY)
                                - (y1 - pixelY) * (x2 - pixelX)
                                ) / area;

                        const double w1 =
                            (
                                (x2 - pixelX) * (y0 - pixelY)
                                - (y2 - pixelY) * (x0 - pixelX)
                                ) / area;

                        const double w2 =
                            1.0 - w0 - w1;

                        if (w0 < 0.0 ||
                            w1 < 0.0 ||
                            w2 < 0.0) {

                            continue;
                        }

                        double z;

                        if (projectionType == ProjectionType::Perspective) {

                            z = 1.0 / (
                                    w0 / a.z
                                    + w1 / b.z
                                    + w2 / c.z
                                    );

                        } else {

                            z =
                                w0 * a.z
                                + w1 * b.z
                                + w2 * c.z;
                        }

                        const int position =
                            y * width() + x;

                        if (!zBufferEnabled ||
                            z < zBuffer[position]) {

                            if (zBufferEnabled) {
                                zBuffer[position] = z;
                            }

                            image.setPixelColor(
                                x,
                                y,
                                face.color
                                );
                        }
                    }
                }
            }
        }
    }

    out.drawImage(0, 0, image);

    out.setPen(QColor(60, 70, 85));

    out.drawText(
        12,
        20,
        QString(
            "Камера: азимут %1°, наклон %2°, расстояние %3"
            )
            .arg(cameraYaw, 0, 'f', 0)
            .arg(cameraPitch, 0, 'f', 0)
            .arg(cameraDistance, 0, 'f', 1)
        );
}