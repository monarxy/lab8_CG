#include "MainWindow.h"
#include "../geometry/ObjIO.h"
#include "../math/Transformations.h"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QGroupBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QMessageBox>
#include <QFileDialog>
#include <QPlainTextEdit>
#include <QTextStream>
#include <QStatusBar>
#include <QSignalBlocker>
#include <QCheckBox>

#include <cmath>

namespace {

constexpr double kPi = 3.14159265358979323846;

QDoubleSpinBox* numberBox(double value = 0.0,
                          double minimum = -1000.0,
                          double maximum = 1000.0,
                          int decimals = 2)
{
    auto* spinBox = new QDoubleSpinBox;
    spinBox->setRange(minimum, maximum);
    spinBox->setDecimals(decimals);
    spinBox->setSingleStep(0.1);
    spinBox->setValue(value);
    return spinBox;
}

QPushButton* actionButton(const QString& text)
{
    auto* button = new QPushButton(text);
    button->setMinimumHeight(29);
    return button;
}

} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      view(new PolyhedronWidget),
      current(PolyhedronFactory::create(PolyhedronFactory::Hexahedron))
{
    setWindowTitle("3D модели — отсечение, Z-буфер и камера");
    resize(1200, 760);
    setMinimumSize(900, 600);

    auto* central = new QWidget;
    auto* rootLayout = new QHBoxLayout(central);
    rootLayout->setContentsMargins(8, 8, 8, 8);
    rootLayout->setSpacing(8);

    auto* scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setMinimumWidth(290);
    scrollArea->setMaximumWidth(360);

    auto* controls = new QWidget;
    auto* controlsLayout = new QVBoxLayout(controls);

    // Model selection and reset.
    shapeBox = new QComboBox;
    for (int i = 0; i < 5; ++i) {
        shapeBox->addItem(PolyhedronFactory::name(
            static_cast<PolyhedronFactory::Type>(i)));
    }

    auto* shapeGroup = new QGroupBox("Модель");
    auto* shapeLayout = new QFormLayout(shapeGroup);
    shapeLayout->addRow("Многогранник:", shapeBox);

    auto* resetButton = actionButton("Сбросить преобразования");
    shapeLayout->addRow(resetButton);
    controlsLayout->addWidget(shapeGroup);

    // Projection selection.
    projectionBox = new QComboBox;
    projectionBox->addItems({"Аксонометрическая", "Перспективная"});

    auto* projectionGroup = new QGroupBox("Проекция");
    auto* projectionLayout = new QFormLayout(projectionGroup);
    projectionLayout->addRow("Тип:", projectionBox);
    auto* cullingCheck = new QCheckBox("Отсекать нелицевые грани"); cullingCheck->setChecked(true);
    auto* zBufferCheck = new QCheckBox("Использовать Z-буфер"); zBufferCheck->setChecked(true);
    projectionLayout->addRow(cullingCheck);
    projectionLayout->addRow(zBufferCheck);
    controlsLayout->addWidget(projectionGroup);

    // Параметры орбитальной камеры: объект остаётся неподвижным.
    auto* cameraGroup = new QGroupBox("Камера (вращение вокруг объекта)");
    auto* cameraLayout = new QFormLayout(cameraGroup);
    auto* cameraYawBox = numberBox(35.0, -180.0, 180.0, 0);
    auto* cameraPitchBox = numberBox(22.0, -80.0, 80.0, 0);
    auto* cameraDistanceBox = numberBox(8.0, 3.0, 30.0, 1);
    cameraLayout->addRow("Азимут:", cameraYawBox);
    cameraLayout->addRow("Наклон:", cameraPitchBox);
    cameraLayout->addRow("Расстояние:", cameraDistanceBox);
    controlsLayout->addWidget(cameraGroup);

    // Parameters used by transformations.
    xBox = numberBox();
    yBox = numberBox();
    zBox = numberBox();
    angleBox = numberBox(30.0, -360.0, 360.0);
    scaleBox = numberBox(1.2, 0.05, 10.0);

    auto* transformationGroup = new QGroupBox("Параметры преобразований");
    auto* transformationLayout = new QFormLayout(transformationGroup);
    transformationLayout->addRow("X:", xBox);
    transformationLayout->addRow("Y:", yBox);
    transformationLayout->addRow("Z:", zBox);
    transformationLayout->addRow("Угол (градусы):", angleBox);
    transformationLayout->addRow("Коэффициент масштаба:", scaleBox);
    controlsLayout->addWidget(transformationGroup);

    auto* moveButton = actionButton("Сместить по X / Y / Z");
    controlsLayout->addWidget(moveButton);

    auto* rotationRow = new QHBoxLayout;
    auto* rotateXButton = actionButton("Поворот X");
    auto* rotateYButton = actionButton("Поворот Y");
    auto* rotateZButton = actionButton("Поворот Z");
    rotationRow->addWidget(rotateXButton);
    rotationRow->addWidget(rotateYButton);
    rotationRow->addWidget(rotateZButton);
    controlsLayout->addLayout(rotationRow);

    auto* scaleButton = actionButton("Масштаб относительно центра");
    controlsLayout->addWidget(scaleButton);

    // Reflection across a coordinate plane.
    planeBox = new QComboBox;
    planeBox->addItems({"XY", "XZ", "YZ"});
    auto* reflectButton = actionButton("Отразить относительно плоскости");

    auto* reflectionGroup = new QGroupBox("Отражение");
    auto* reflectionLayout = new QHBoxLayout(reflectionGroup);
    reflectionLayout->addWidget(planeBox);
    reflectionLayout->addWidget(reflectButton);
    controlsLayout->addWidget(reflectionGroup);

    // Rotation around an axis through the polyhedron center.
    axisBox = new QComboBox;
    axisBox->addItems({"X", "Y", "Z"});
    auto* centerRotateButton = actionButton("Вращать вокруг центральной оси");

    auto* axisGroup = new QGroupBox("Вращение через центр");
    auto* axisLayout = new QHBoxLayout(axisGroup);
    axisLayout->addWidget(axisBox);
    axisLayout->addWidget(centerRotateButton);
    controlsLayout->addWidget(axisGroup);

    // Rotation around an arbitrary line through points A and B.
    axBox = numberBox(-1.0);
    ayBox = numberBox();
    azBox = numberBox();
    bxBox = numberBox(1.0);
    byBox = numberBox();
    bzBox = numberBox();

    auto* lineGroup = new QGroupBox("Произвольная прямая через точки A и B");
    auto* lineLayout = new QFormLayout(lineGroup);

    auto* aRow = new QWidget;
    auto* aLayout = new QHBoxLayout(aRow);
    aLayout->setContentsMargins(0, 0, 0, 0);
    aLayout->addWidget(axBox);
    aLayout->addWidget(ayBox);
    aLayout->addWidget(azBox);
    lineLayout->addRow("A (x,y,z):", aRow);

    auto* bRow = new QWidget;
    auto* bLayout = new QHBoxLayout(bRow);
    bLayout->setContentsMargins(0, 0, 0, 0);
    bLayout->addWidget(bxBox);
    bLayout->addWidget(byBox);
    bLayout->addWidget(bzBox);
    lineLayout->addRow("B (x,y,z):", bRow);

    auto* arbitraryRotateButton = actionButton("Повернуть вокруг прямой");
    lineLayout->addRow(arbitraryRotateButton);
    controlsLayout->addWidget(lineGroup);
    // OBJ import/export for the current model.
    auto* fileGroup = new QGroupBox("Файл OBJ");
    auto* fileLayout = new QHBoxLayout(fileGroup);
    auto* openButton = actionButton("Открыть OBJ");
    auto* saveButton = actionButton("Сохранить OBJ");
    fileLayout->addWidget(openButton);
    fileLayout->addWidget(saveButton);
    controlsLayout->addWidget(fileGroup);

    // Surface of revolution settings. Each line contains x, y, z of one profile point.
    auto* revolutionGroup = new QGroupBox("Фигура вращения");
    auto* revolutionLayout = new QFormLayout(revolutionGroup);
    profileEdit = new QPlainTextEdit;
    profileEdit->setMaximumHeight(85);
    profileEdit->setPlainText("1, 0, -1\n1, 0, 1");
    revolutionAxisBox = new QComboBox;
    revolutionAxisBox->addItems({"X", "Y", "Z"});
    revolutionAxisBox->setCurrentIndex(2);
    revolutionSegmentsBox = new QDoubleSpinBox;
    revolutionSegmentsBox->setDecimals(0);
    revolutionSegmentsBox->setRange(3, 200);
    revolutionSegmentsBox->setValue(24);
    auto* buildRevolutionButton = actionButton("Построить фигуру вращения");
    revolutionLayout->addRow("Точки образующей (x, y, z):", profileEdit);
    revolutionLayout->addRow("Ось:", revolutionAxisBox);
    revolutionLayout->addRow("Разбиения:", revolutionSegmentsBox);
    revolutionLayout->addRow(buildRevolutionButton);
    controlsLayout->addWidget(revolutionGroup);

    // Function surface settings; same interval and number of divisions for X and Y.
    auto* surfaceGroup = new QGroupBox("График двух переменных z = f(x, y)");
    auto* surfaceLayout = new QFormLayout(surfaceGroup);
    functionBox = new QComboBox;
    functionBox->addItems({"x^2 + y^2", "sin(x) * cos(y)", "x * y", "sin(sqrt(x^2 + y^2))"});
    surfaceMinBox = numberBox(-2.0, -20.0, 20.0);
    surfaceMaxBox = numberBox(2.0, -20.0, 20.0);
    surfaceDivisionsBox = numberBox(30.0, 2.0, 150.0, 0);
    auto* buildSurfaceButton = actionButton("Построить поверхность");
    surfaceLayout->addRow("Функция:", functionBox);
    surfaceLayout->addRow("Начало диапазона:", surfaceMinBox);
    surfaceLayout->addRow("Конец диапазона:", surfaceMaxBox);
    surfaceLayout->addRow("Разбиения по X и Y:", surfaceDivisionsBox);
    surfaceLayout->addRow(buildSurfaceButton);
    controlsLayout->addWidget(surfaceGroup);

    controlsLayout->addStretch(1);

    scrollArea->setWidget(controls);
    rootLayout->addWidget(scrollArea);
    rootLayout->addWidget(view, 1);
    setCentralWidget(central);

    view->setPolyhedron(current);

    connect(cullingCheck, &QCheckBox::toggled, view, &PolyhedronWidget::setBackFaceCulling);
    connect(zBufferCheck, &QCheckBox::toggled, view, &PolyhedronWidget::setZBufferEnabled);
    const auto updateCamera = [this, cameraYawBox, cameraPitchBox, cameraDistanceBox]() {
        view->setCamera(cameraYawBox->value(), cameraPitchBox->value(), cameraDistanceBox->value());
    };
    connect(cameraYawBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [updateCamera](double){updateCamera();});
    connect(cameraPitchBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [updateCamera](double){updateCamera();});
    connect(cameraDistanceBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [updateCamera](double){updateCamera();});

    connect(openButton, &QPushButton::clicked, this, &MainWindow::openObj);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveObj);
    connect(buildRevolutionButton, &QPushButton::clicked, this, &MainWindow::buildRevolution);
    connect(buildSurfaceButton, &QPushButton::clicked, this, &MainWindow::buildSurface);

    connect(shapeBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
                loadShape(index);
            });

    connect(projectionBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
                const ProjectionType type = index == 0
                    ? ProjectionType::Axonometric
                    : ProjectionType::Perspective;
                view->setProjectionType(type);
            });

    connect(resetButton, &QPushButton::clicked, this, [this]() {
        int index = shapeBox->currentIndex();
        if (index < 0) index = 1;
        { QSignalBlocker blocker(shapeBox); shapeBox->setCurrentIndex(index); }
        loadShape(index);
    });

    connect(moveButton, &QPushButton::clicked, this, [this]() {
        apply(Matrix4::translation(xBox->value(), yBox->value(), zBox->value()));
    });

    const auto angle = [this]() {
        return angleBox->value() * kPi / 180.0;
    };

    connect(rotateXButton, &QPushButton::clicked, this, [this, angle]() {
        apply(Matrix4::rotationX(angle()));
    });
    connect(rotateYButton, &QPushButton::clicked, this, [this, angle]() {
        apply(Matrix4::rotationY(angle()));
    });
    connect(rotateZButton, &QPushButton::clicked, this, [this, angle]() {
        apply(Matrix4::rotationZ(angle()));
    });

    connect(scaleButton, &QPushButton::clicked, this, [this]() {
        apply(Transformations::scaleAroundCenter(current.center(), scaleBox->value()));
    });

    connect(reflectButton, &QPushButton::clicked, this, [this]() {
        const int index = planeBox->currentIndex();
        const Matrix4 reflection = index == 0
            ? Matrix4::reflectionXY()
            : (index == 1 ? Matrix4::reflectionXZ() : Matrix4::reflectionYZ());
        apply(reflection);
    });

    connect(centerRotateButton, &QPushButton::clicked, this, [this, angle]() {
        apply(Transformations::rotateAroundCenterAxis(
            current.center(), axisBox->currentIndex(), angle()));
    });

    connect(arbitraryRotateButton, &QPushButton::clicked, this, [this, angle]() {
        const Point3D a(axBox->value(), ayBox->value(), azBox->value());
        const Point3D b(bxBox->value(), byBox->value(), bzBox->value());

        if (std::abs(a.x - b.x) + std::abs(a.y - b.y) + std::abs(a.z - b.z) < 1e-9) {
            QMessageBox::warning(this, "Неверная прямая",
                                 "Точки A и B должны различаться.");
            return;
        }

        apply(Transformations::rotateAroundLine(a, b, angle()));
    });
}

void MainWindow::loadShape(int index)
{
    current = PolyhedronFactory::create(
        static_cast<PolyhedronFactory::Type>(index));
    view->setPolyhedron(current);
}

void MainWindow::apply(const Matrix4& matrix)
{
    current.applyMatrix(matrix);
    view->setPolyhedron(current);
}


void MainWindow::openObj()
{
    const QString fileName = QFileDialog::getOpenFileName(
        this, "Открыть модель OBJ", QString(), "Wavefront OBJ (*.obj)");
    if (fileName.isEmpty()) return;
    Polyhedron loaded;
    QString error;
    if (!ObjIO::load(fileName, loaded, &error)) {
        QMessageBox::warning(this, "Ошибка загрузки OBJ", error);
        return;
    }
    current = loaded;
    view->setPolyhedron(current);
    { QSignalBlocker blocker(shapeBox); shapeBox->setCurrentIndex(-1); }
}

void MainWindow::saveObj()
{
    const QString fileName = QFileDialog::getSaveFileName(
        this, "Сохранить модель OBJ", "model.obj", "Wavefront OBJ (*.obj)");
    if (fileName.isEmpty()) return;
    QString error;
    if (!ObjIO::save(fileName, current, &error))
        QMessageBox::warning(this, "Ошибка сохранения OBJ", error);
    else
        statusBar()->showMessage("Модель сохранена в OBJ", 4000);
}

void MainWindow::buildRevolution()
{
    QVector<Point3D> profile;
    const QStringList lines = profileEdit->toPlainText().split(QChar('\n'), QString::SkipEmptyParts);
    for (const QString& line : lines) {
        QString normalized = line;
        normalized.replace(';', ',');
        const QStringList values = normalized.split(',', QString::SkipEmptyParts);
        if (values.size() != 3) {
            QMessageBox::warning(this, "Образующая", "Каждая строка должна содержать x, y, z через запятую.");
            return;
        }
        bool okX = false, okY = false, okZ = false;
        const double x = values[0].trimmed().toDouble(&okX);
        const double y = values[1].trimmed().toDouble(&okY);
        const double z = values[2].trimmed().toDouble(&okZ);
        if (!okX || !okY || !okZ) {
            QMessageBox::warning(this, "Образующая", "Не удалось прочитать координаты точки.");
            return;
        }
        profile.append(Point3D(x, y, z));
    }
    Polyhedron generated;
    QString error;
    if (!ModelGenerators::revolution(profile, revolutionAxisBox->currentIndex(),
                                      static_cast<int>(revolutionSegmentsBox->value()),
                                      generated, &error)) {
        QMessageBox::warning(this, "Построение фигуры вращения", error);
        return;
    }
    current = generated;
    view->setPolyhedron(current);
    { QSignalBlocker blocker(shapeBox); shapeBox->setCurrentIndex(-1); }
}

void MainWindow::buildSurface()
{
    Polyhedron generated;
    QString error;
    if (!ModelGenerators::surface(functionBox->currentIndex(), surfaceMinBox->value(),
                                  surfaceMaxBox->value(),
                                  static_cast<int>(surfaceDivisionsBox->value()),
                                  generated, &error)) {
        QMessageBox::warning(this, "Построение поверхности", error);
        return;
    }
    current = generated;
    view->setPolyhedron(current);
    { QSignalBlocker blocker(shapeBox); shapeBox->setCurrentIndex(-1); }
}
