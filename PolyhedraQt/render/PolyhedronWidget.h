#pragma once
#include <QWidget>
#include <QVector>
#include "../geometry/Polyhedron.h"
#include "../math/Projection.h"
#include "Camera.h"

class PolyhedronWidget : public QWidget {
    Q_OBJECT
public:
    explicit PolyhedronWidget(QWidget* parent = nullptr);
    void setPolyhedron(const Polyhedron& p);
    void setProjectionType(ProjectionType type);
    void setBackFaceCulling(bool enabled);
    void setZBufferEnabled(bool enabled);
    void setCamera(double yawDegrees, double pitchDegrees, double distance);
protected:
    void paintEvent(QPaintEvent*) override;
private:
    Polyhedron polyhedron;
    ProjectionType projectionType = ProjectionType::Perspective;
    bool cullingEnabled = true;
    bool zBufferEnabled = true;
    double cameraYaw = 35.0, cameraPitch = 22.0, cameraDistance = 8.0;
    Camera camera;
};
