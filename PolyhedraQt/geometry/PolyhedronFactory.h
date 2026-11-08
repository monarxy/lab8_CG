#pragma once

#include <QString>

#include "Polyhedron.h"

class PolyhedronFactory {
public:
    enum Type {
        Tetrahedron,
        Hexahedron,
        Octahedron,
        Icosahedron,
        Dodecahedron
    };

    static Polyhedron create(Type type);
    static QString name(Type type);
};
