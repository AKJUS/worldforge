// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2003 Alistair Riddoch, Damien McGinnes

#include <Mercator/TerrainMod_impl.h>

namespace Mercator {

TerrainMod::~TerrainMod()
{
}

template class LevelTerrainMod<WFMath::Ball<2> >;
template class LevelTerrainMod<WFMath::RotBox<2> >;

CraterTerrainMod::~CraterTerrainMod()
{
}
    
WFMath::AxisBox<2> CraterTerrainMod::bbox() const
{
    return ab;
}

void CraterTerrainMod::apply(float &point, int x, int y) const
{
    if (Contains(m_shape,WFMath::Point<3>(x,y,point),true)) {
        float d = m_shape.radius() * m_shape.radius() -
                  (m_shape.getCenter()[0] - x) * (m_shape.getCenter()[0] - x) -
                  (m_shape.getCenter()[1] - y) * (m_shape.getCenter()[1] - y); 

        if (d >= 0.0)
            point = m_shape.getCenter()[2] - sqrt(d);
    }
}
    
TerrainMod * CraterTerrainMod::clone() const
{
    return new CraterTerrainMod(m_shape);
}

} // namespace Mercator
