#pragma once

#include <QMainWindow>

#include "../geometry/PolyhedronFactory.h"
#include "../geometry/ModelGenerators.h"
#include "../render/PolyhedronWidget.h"

class QComboBox;
class QDoubleSpinBox;
class QPlainTextEdit;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    PolyhedronWidget* view;
    Polyhedron current;

    QComboBox* shapeBox;
    QComboBox* projectionBox;
    QComboBox* axisBox;
    QComboBox* planeBox;

    QDoubleSpinBox* xBox;
    QDoubleSpinBox* yBox;
    QDoubleSpinBox* zBox;
    QDoubleSpinBox* angleBox;
    QDoubleSpinBox* scaleBox;

    QDoubleSpinBox* axBox;
    QDoubleSpinBox* ayBox;
    QDoubleSpinBox* azBox;
    QDoubleSpinBox* bxBox;
    QDoubleSpinBox* byBox;
    QDoubleSpinBox* bzBox;

    QPlainTextEdit* profileEdit;
    QComboBox* revolutionAxisBox;
    QDoubleSpinBox* revolutionSegmentsBox;
    QComboBox* functionBox;
    QDoubleSpinBox* surfaceMinBox;
    QDoubleSpinBox* surfaceMaxBox;
    QDoubleSpinBox* surfaceDivisionsBox;

    void loadShape(int index);
    void apply(const Matrix4& matrix);
    void openObj();
    void saveObj();
    void buildRevolution();
    void buildSurface();
};
