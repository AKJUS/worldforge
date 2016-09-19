
// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2009 Alistair Riddoch

#include <Mercator/Segment.h>

#include <wfmath/point.h>
#include <wfmath/axisbox.h>

#include <cstdlib>

int main(int argc, char ** argv)
{
    int iterations = 1;

    if (argc > 1) {
        iterations = strtol(argv[1], 0, 10);
    }

    Mercator::Segment s(0,0,64);

    Mercator::Matrix<2,2,Mercator::BasePoint> & points = s.getControlPoints();
    points(0, 0).roughness() = 1.85;
    points(1, 0).roughness() = 1.75;
    points(0, 1).roughness() = 1.65;
    points(1, 1).roughness() = 1.95;

    for (int i = 0; i < iterations; ++i) {

        s.populate();

    }

    return 0;
}

// stubs

#include <Mercator/Shader.h>
#include <Mercator/Surface.h>

namespace Mercator {

void Surface::populate()
{
}

Surface * Shader::newSurface(const Segment & segment) const
{
    return 0;
}

}
