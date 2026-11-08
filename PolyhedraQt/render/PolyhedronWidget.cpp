
#include "PolyhedronWidget.h"

#include "../geometry/PolyhedronFactory.h"
#include "../math/Matrix4.h"

#include <QPainter>
#include <QImage>
#include <QPaintEvent>
#include <QSizePolicy>
#include <QString>

#include <algorithm>
#include <cmath>
#include <limits>

namespace {

struct V3 {
    double x;
    double y;
    double z;
};

V3 sub(V3 a, V3 b) {
    return {
        a.x - b.x,
        a.y - b.y,
        a.z - b.z
    };
}

V3 cross(V3 a, V3 b) {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

double dot(V3 a, V3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

V3 norm(V3 a) {
    double length = std::sqrt(dot(a, a));

    if (length > 1e-12) {
        return {
            a.x / length,
            a.y / length,
            a.z / length
        };
    }

    return {0.0, 0.0, 0.0};
}

struct Vertex {
    double x;
    double y;
    double z;

    QPointF screen;
};

} // namespace


PolyhedronWidget::PolyhedronWidget(QWidget* parent)
    : QWidget(parent) {

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


void PolyhedronWidget::setPolyhedron(const Polyhedron& p) {
    polyhedron = p;

    polyhedron.calculateNormals();

    update();
}


void PolyhedronWidget::setProjectionType(ProjectionType type) {
    projectionType = type;
    update();
}


void PolyhedronWidget::setBackFaceCulling(bool enabled) {
    cullingEnabled = enabled;
    update();
}


void PolyhedronWidget::setZBufferEnabled(bool enabled) {
    zBufferEnabled = enabled;
    update();
}


void PolyhedronWidget::setCamera(
    double yaw,
    double pitch,
    double distance
    ) {
    cameraYaw = yaw;
    cameraPitch = pitch;
    cameraDistance = distance;

    camera.orbit(
        cameraYaw,
        cameraPitch,
        cameraDistance
        );

    update();
}


void PolyhedronWidget::paintEvent(QPaintEvent*) {

    QPainter out(this);

    out.setRenderHint(QPainter::Antialiasing, true);

    out.fillRect(
        rect(),
        QColor(248, 249, 251)
        );

    if (polyhedron.vertices.isEmpty()) {
        return;
    }


    QVector<Polyhedron> objects;

    objects.append(polyhedron);

    Polyhedron leftPolyhedron =
        PolyhedronFactory::create(
            PolyhedronFactory::Hexahedron
            );

    leftPolyhedron.applyMatrix(
        Matrix4::translation(-3.0, 0.0, 0.0)
        );

    objects.append(leftPolyhedron);

    Polyhedron rightPolyhedron =
        PolyhedronFactory::create(
            PolyhedronFactory::Octahedron
            );

    rightPolyhedron.applyMatrix(
        Matrix4::translation(3.0, 0.0, -0.4)
        );

    objects.append(rightPolyhedron);



    camera.setPerspective(
        0.82,
        width() / double(qMax(1, height())),
        0.1,
        100.0
        );

    const V3 cameraPos{
        camera.position.x,
        camera.position.y,
        camera.position.z
    };

    const V3 forward = norm({
        -cameraPos.x,
        -cameraPos.y,
        -cameraPos.z
    });

    const V3 worldUp{0.0, 1.0, 0.0};

    const V3 right = norm(
        cross(forward, worldUp)
        );

    const V3 up = norm(
        cross(right, forward)
        );

    const double focal =
        std::min(width(), height())
        * camera.projectionMatrix.m[1][1]
        * 0.5;


    QImage image(
        size(),
        QImage::Format_ARGB32
        );

    image.fill(QColor(248, 249, 251));

    QVector<double> zbuf(
        width() * height(),
        std::numeric_limits<double>::infinity()
        );


    for (const Polyhedron& object : objects) {

        QVector<Vertex> verts;

        verts.reserve(object.vertices.size());

        for (const Point3D& p : object.vertices) {

            V3 rel = sub(
                {p.x, p.y, p.z},
                cameraPos
                );

            const double cx = dot(rel, right);
            const double cy = dot(rel, up);
            const double cz = dot(rel, forward);

            Vertex v;

            v.x = cx;
            v.y = cy;
            v.z = cz;

            if (projectionType == ProjectionType::Perspective) {

                if (cz <= 0.05) {

                    v.screen = QPointF(-10000, -10000);

                } else {

                    v.screen = QPointF(
                        width() / 2.0 + focal * cx / cz,
                        height() / 2.0 - focal * cy / cz
                        );
                }

            } else {

                v.screen = QPointF(
                    width() / 2.0 + focal * cx / cameraDistance,
                    height() / 2.0 - focal * cy / cameraDistance
                    );
            }

            verts.append(v);
        }


        for (const Polygon& face : object.faces) {

            if (face.indices.size() < 3) {
                continue;
            }

            bool indicesValid = true;

            for (int index : face.indices) {

                if (index < 0 || index >= verts.size()) {
                    indicesValid = false;
                    break;
                }
            }

            if (!indicesValid) {
                continue;
            }


            V3 normal{
                face.normal.x,
                face.normal.y,
                face.normal.z
            };

            V3 faceCenter{0.0, 0.0, 0.0};

            for (int index : face.indices) {

                faceCenter.x += object.vertices[index].x;
                faceCenter.y += object.vertices[index].y;
                faceCenter.z += object.vertices[index].z;
            }

            double count = double(face.indices.size());

            faceCenter.x /= count;
            faceCenter.y /= count;
            faceCenter.z /= count;

            V3 viewDirection = sub(
                cameraPos,
                faceCenter
                );

            if (
                cullingEnabled
                && dot(normal, viewDirection) <= 0.0
                ) {
                continue;
            }


            for (
                int k = 1;
                k + 1 < face.indices.size();
                ++k
                ) {

                const Vertex a = verts[face.indices[0]];
                const Vertex b = verts[face.indices[k]];
                const Vertex c = verts[face.indices[k + 1]];

                if (
                    a.screen.x() < -1000
                    || b.screen.x() < -1000
                    || c.screen.x() < -1000
                    ) {
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
                    int(std::floor(std::min({x0, x1, x2}))),
                    width() - 1
                    );

                const int maxX = qBound(
                    0,
                    int(std::ceil(std::max({x0, x1, x2}))),
                    width() - 1
                    );

                const int minY = qBound(
                    0,
                    int(std::floor(std::min({y0, y1, y2}))),
                    height() - 1
                    );

                const int maxY = qBound(
                    0,
                    int(std::ceil(std::max({y0, y1, y2}))),
                    height() - 1
                    );

                for (int y = minY; y <= maxY; ++y) {

                    for (int x = minX; x <= maxX; ++x) {

                        const double px = x + 0.5;
                        const double py = y + 0.5;


                        const double w0 =
                            (
                                (x1 - px) * (y2 - py)
                                - (y1 - py) * (x2 - px)
                                ) / area;

                        const double w1 =
                            (
                                (x2 - px) * (y0 - py)
                                - (y2 - py) * (x0 - px)
                                ) / area;

                        const double w2 = 1.0 - w0 - w1;
                        if (
                            w0 < 0.0
                            || w1 < 0.0
                            || w2 < 0.0
                            ) {
                            continue;
                        }

                        double z;

                        if (
                            projectionType
                            == ProjectionType::Perspective
                            ) {

                            const double inverseZ =
                                w0 / a.z
                                + w1 / b.z
                                + w2 / c.z;

                            if (inverseZ <= 0.0) {
                                continue;
                            }

                            z = 1.0 / inverseZ;

                        } else {

                            z =
                                w0 * a.z
                                + w1 * b.z
                                + w2 * c.z;
                        }

                        const int position = y * width() + x;
                        if (
                            !zBufferEnabled
                            || z < zbuf[position]
                            ) {

                            if (zBufferEnabled) {
                                zbuf[position] = z;
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
