#pragma once
#include "Polyhedron.h"
#include <QString>

class ObjIO {
public:
    static bool load(const QString& fileName, Polyhedron& model, QString* error = nullptr);
    static bool save(const QString& fileName, const Polyhedron& model, QString* error = nullptr);
};
