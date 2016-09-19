// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2003 Alistair Riddoch, Damien McGinnes

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "iround.h"

#include "Terrain.h"

#include "Matrix.h"
#include "Segment.h"
#include "TerrainMod.h"
#include "Shader.h"
#include "Area.h"
#include "Surface.h"

#include <iostream>
#include <algorithm>

#include <cstdio>

namespace Mercator {

const unsigned int Terrain::DEFAULT;
const unsigned int Terrain::SHADED;
constexpr float Terrain::defaultLevel;

/// \brief Construct a new Terrain object with optional options and resolution.
///
/// @param options a bitfield of option flags. Defaults to no flags set.
/// - DEFAULT value provided for no flags set.
/// - SHADED is set if shaders are going to be used on this terrain.
/// @param resolution the spacing between adjacent base points. Defaults to 64.
Terrain::Terrain(unsigned int options, unsigned int resolution) : m_options(options),
                                                                  m_res(resolution),
                                                                  m_spacing(resolution)
{

}

/// \brief Destroy Terrain object, deleting contained objects.
///
/// All Segment objects are deleted, but Shader objects are not yet deleted.
/// Probably need to be fixed.
Terrain::~Terrain()
{
    Segmentstore::const_iterator I = m_segments.begin(); 
    Segmentstore::const_iterator Iend = m_segments.end();
    for (; I != Iend; ++I) {
        Segmentcolumn::const_iterator J = I->second.begin(); 
        Segmentcolumn::const_iterator Jend = I->second.end(); 
        for (; J != Jend; ++J) {
            Segment * s = J->second;
            if (s) delete s;
        }
    }
}

/// \brief Add a new Shader to the list for this terrain.
///
/// As each shader is added, surfaces are added to all existing segments
/// to store the result of the shader.
void Terrain::addShader(const Shader * t, int id)
{
    if (m_shaders.count(id)) {
        std::cerr << "WARNING: duplicate use of shader ID " << id << std::endl;
    }
    
    m_shaders[id] = t;
    
    Segmentstore::const_iterator I = m_segments.begin(); 
    Segmentstore::const_iterator Iend = m_segments.end(); 
    for (; I != Iend; ++I) {
        Segmentcolumn::const_iterator J = I->second.begin(); 
        Segmentcolumn::const_iterator Jend = I->second.end(); 
        for (; J != Jend; ++J) {
            Segment *seg=J->second;
            if (!t->checkIntersect(*seg)) {
                continue;
            }

            Segment::Surfacestore & sss = seg->getSurfaces();
            sss[id] = t->newSurface(*seg);
        }
    }
}

/// \brief remove a Shader from the list for this terrain.
///
/// As each shader is removed, surfaces are removed from existing segments
void Terrain::removeShader(const Shader * t, int id)
{

    m_shaders.erase(m_shaders.find(id));

    // Delete all surfaces for this shader
    Segmentstore::const_iterator I = m_segments.begin();
    Segmentstore::const_iterator Iend = m_segments.end();
    for (; I != Iend; ++I) {
        Segmentcolumn::const_iterator J = I->second.begin();
        Segmentcolumn::const_iterator Jend = I->second.end();
        for (; J != Jend; ++J) {
            Segment *seg=J->second;

            Segment::Surfacestore & sss = seg->getSurfaces();
            Segment::Surfacestore::iterator K = sss.find(id);
            if (K != sss.end()) {
                delete K->second;
                sss.erase(K);
            }
        }
    }
}



/// \brief Add the required Surface objects to a Segment.
///
/// If shading is enabled, each Segment has a set of Surface objects
/// corresponding to the Shader objects available for this terrain.
/// This function creates the necessary Surface objects, and adds them
/// to the list in the Segment object. At this point the Segment is
/// not yet populated with heightfield data, so the Surface cannot
/// be populated. A newly constructed surface does not allocate its
/// buffer memory, so there is very little overhead to creating it.
void Terrain::addSurfaces(Segment & seg)
{
    Segment::Surfacestore & sss = seg.getSurfaces();
    if (!sss.empty()) {
        std::cerr << "WARNING: Adding surfaces to a terrain segment which has surfaces"
                  << std::endl << std::flush;
        sss.clear();
    }
    
    Shaderstore::const_iterator I = m_shaders.begin();
    Shaderstore::const_iterator Iend = m_shaders.end();
    for (; I != Iend; ++I) {
        // shader doesn't touch this segment, skip
        if (!I->second->checkIntersect(seg)) {
            continue;
        }
        
        sss[I->first] = I->second->newSurface(seg);
    }
}

/// \brief Populate the Surface objects associated with a Segment.
///
/// This is called after a Segment has been populated with heightfield
/// data. The actual responsibility for populating the Surface objects
/// is in Segment::populateSurfaces().
void Terrain::shadeSurfaces(Segment & seg)
{
    seg.populateSurfaces();
}

/// \brief Get the height value at a given coordinate x,y.
///
/// This is a convenience function provided to quickly get a height
/// value at a given point. It always succeeds, as if no height data
/// is available it just returns the default value. If a Segment does
/// exist in the right place and is populated, the coords within that
/// Segment are determined, and the heightfield queried. This function
/// does not cause any data to be populated, and does not attempt to
/// do any interpolation to get an accurate height figure. For more
/// accurate results see Terrain::getHeightAndNormal.
float Terrain::get(float x, float y) const
{
    int ix = I_ROUND(std::floor(x / m_spacing));
    int iy = I_ROUND(std::floor(y / m_spacing));

    Segment * s = getSegment(ix, iy);
    if ((s == 0) || (!s->isValid())) {
        return Terrain::defaultLevel;
    }
    return s->get(I_ROUND(x) - (ix * m_res), I_ROUND(y) - (iy * m_res));
}

/// \brief Get an accurate height and normal vector at a given coordinate
/// x,y.
///
/// This is a more expensive function that Terrain::get() for getting an
/// accurate height value and surface normal at a given point. The main
/// body of functionality is in the Segment::getHeightAndNormal() function
/// called from here.
/// The height and normal are interpolated based on a model where each
/// tile of the heightfield comprises two triangles. If no heightfield data
/// is available for the given location, this function returns false, and
/// no data is returned.
/// @param x coordinate of point to be returned.
/// @param y coordinate of point to be returned.
/// @param h reference to variable which will be used to store the resulting
/// height value.
/// @param n reference to variable which will be used to store the resulting
/// normal value.
/// @return true if heightdata was available, false otherwise.
bool Terrain::getHeightAndNormal(float x, float y, float & h,
                                  WFMath::Vector<3> & n) const
{
    int ix = I_ROUND(std::floor(x / m_spacing));
    int iy = I_ROUND(std::floor(y / m_spacing));

    Segment * s = getSegment(ix, iy);
    if ((s == 0) || (!s->isValid())) {
        return false;
    }
    s->getHeightAndNormal(x - (ix * m_res), y - (iy * m_res), h, n);
    return true;
}

/// \brief Get the BasePoint at a given base point coordinate.
///
/// Get the BasePoint value for the given coordinate on the base
/// point grid.
/// @param x coordinate on the base point grid.
/// @param y coordinate on the base point grid.
/// @param z reference to varaible which will be used to store the
/// BasePoint data.
/// @return true if a BasePoint is defined at the given coordinate, false
/// otherwise.
bool Terrain::getBasePoint(int x, int y, BasePoint& z) const
{
    Pointstore::const_iterator I = m_basePoints.find(x);
    if (I == m_basePoints.end()) {
        return false;
    }
    Pointcolumn::const_iterator J = I->second.find(y);
    if (J == I->second.end()) {
        return false;
    }
    z = J->second;
    return true;
}

/// \brief Set the BasePoint value at a given base point coordinate.
///
/// Set the BasePoint value for the given coordinate on the base
/// point grid.
/// If inserting this BasePoint completes the set of points required
/// to define one or more Segment objects which were not yet defined,
/// new Segment objects are created. If this replaces a point for one
/// or more Segment objects that were already defined, the contents of
/// those Segment objects are invalidated.
/// @param x coordinate on the base point grid.
/// @param y coordinate on the base point grid.
/// @param z BasePoint value to be used at the given coordinate.
void Terrain::setBasePoint(int x, int y, const BasePoint& z)
{
    m_basePoints[x][y] = z;
    bool pointIsSet[3][3];
    BasePoint existingPoint[3][3];
    for(int i = x - 1, ri = 0; i < x + 2; ++i, ++ri) {
        for(int j = y - 1, rj = 0; j < y + 2; ++j, ++rj) {
            pointIsSet[ri][rj] = getBasePoint(i, j, existingPoint[ri][rj]);
        }
    }
    for(int i = x - 1, ri = 0; i < x + 1; ++i, ++ri) {
        for(int j = y - 1, rj = 0; j < y + 1; ++j, ++rj) {
            Segment * s = getSegment(i, j);
            if (s == 0) { 
                bool complete = pointIsSet[ri][rj] &&
                                pointIsSet[ri + 1][rj + 1] &&
                                pointIsSet[ri + 1][rj] &&
                                pointIsSet[ri][rj + 1];
                if (!complete) {
                    continue;
                }
                s = new Segment(i * m_res, j * m_res, m_res);
                Matrix<2, 2, BasePoint> & cp = s->getControlPoints();
                float min = existingPoint[ri][rj].height();
                float max = min;
                for(unsigned int k = 0; k < 2; ++k) {
                    for(unsigned int l = 0; l < 2; ++l) {
                        cp(k, l) = existingPoint[ri + k][rj + l];
                        min = std::min(cp(k, l).height(), min);
                        max = std::max(cp(k, l).height(), max);
                    }
                }
                s->setMinMax(min, max);

                for (auto& entry : m_terrainMods) {
                    const TerrainMod* terrainMod = std::get<0>(entry.second);
                    if (terrainMod->checkIntersects(*s)) {
                        s->updateMod(entry.first, terrainMod);
                    }
                }

                // apply shaders last, after all other data is in place
                if (isShaded()) {
                    addSurfaces(*s);
                }
                
                m_segments[i][j] = s;
                continue;
            }
            s->setCornerPoint(ri ? 0 : 1, rj ? 0 : 1, z);
        }
    }
}

/// \brief Get the Segment at a given base point coordinate.
///
/// Get the Segment pointer for the given coordinate on the base
/// point grid. The Segment in question may not have been populated
/// with heightfield or surface data.
/// @param x coordinate on the base point grid.
/// @param y coordinate on the base point grid.
/// @return a valid pointer if a Segment is defined at the given coordinate,
/// zero otherwise.
Segment * Terrain::getSegment(int x, int y) const
{
    Segmentstore::const_iterator I = m_segments.find(x);
    if (I == m_segments.end()) {
        return 0;
    }
    Segmentcolumn::const_iterator J = I->second.find(y);
    if (J == I->second.end()) {
        return 0;
    }
    return J->second;
}

Terrain::Rect Terrain::updateMod(long id, const TerrainMod * mod)
{
    std::set<Segment*> removed, added, updated;

    auto I = m_terrainMods.find(id);

    Rect old_box;
    if (I != m_terrainMods.end()) {
        std::tuple<const TerrainMod *, Rect>& entry = I->second;

        old_box = std::get<1>(entry);



        int lx=I_ROUND(std::floor((old_box.lowCorner()[0] - 1.f) / m_spacing));
        int ly=I_ROUND(std::floor((old_box.lowCorner()[1] - 1.f) / m_spacing));
        int hx=I_ROUND(std::ceil((old_box.highCorner()[0] + 1.f) / m_spacing));
        int hy=I_ROUND(std::ceil((old_box.highCorner()[1] + 1.f) / m_spacing));

        for (int i=lx;i<hx;++i) {
           for (int j=ly;j<hy;++j) {
               Segment *s=getSegment(i,j);
               if (!s) {
                   continue;
               }

               removed.insert(s);

           } // of y loop
        } // of x loop

        if (mod) {
            std::get<0>(entry) = mod;
            std::get<1>(entry) = mod->bbox();
        } else {
            m_terrainMods.erase(id);
        }
    } else if (mod) {
        m_terrainMods.emplace(id, std::make_tuple(mod, mod->bbox()));
    }

    if (mod) {
        int lx=I_ROUND(std::floor((mod->bbox().lowCorner()[0] - 1.f) / m_spacing));
        int ly=I_ROUND(std::floor((mod->bbox().lowCorner()[1] - 1.f) / m_spacing));
        int hx=I_ROUND(std::ceil((mod->bbox().highCorner()[0] + 1.f) / m_spacing));
        int hy=I_ROUND(std::ceil((mod->bbox().highCorner()[1] + 1.f) / m_spacing));

        for (int i=lx;i<hx;++i) {
            for (int j=ly;j<hy;++j) {
                Segment *s=getSegment(i,j);
                if (!s) {
                    continue;
                }

                std::set<Segment*>::iterator J = removed.find(s);
                if (J == removed.end()) {
                    added.insert(s);
                } else {
                    updated.insert(s);
                    removed.erase(J);
                }
            } // of y loop
        } // of x loop
    }

    for (auto& segment : removed) {
        segment->updateMod(id, nullptr);
    }
    for (auto& segment : added) {
        if (mod->checkIntersects(*segment)) {
            segment->updateMod(id, mod);
        }
    }
    for (auto& segment : updated) {
        if (mod->checkIntersects(*segment)) {
            segment->updateMod(id, mod);
        } else {
            segment->updateMod(id, nullptr);
        }
    }

    return old_box;
}

bool Terrain::hasMod(long id) const
{
    return m_terrainMods.find(id) != m_terrainMods.end();
}


/// \brief Add an area modifier to the terrain.
///
/// Add a new Area object to the terrain, which defines a modification
/// to the surface.
void Terrain::addArea(const Area * area)
{
    int layer = area->getLayer();

    Shaderstore::const_iterator I = m_shaders.find(layer);
    if (I != m_shaders.end()) {
        area->setShader(I->second);
    }
    
    //work out which segments are overlapped by this effector
    //note that the bbox is expanded by one grid unit because
    //segments share edges. this ensures a mod along an edge
    //will affect both segments.

    m_terrainAreas.emplace(area, area->bbox());

    int lx=I_ROUND(std::floor((area->bbox().lowCorner()[0] - 1.f) / m_spacing));
    int ly=I_ROUND(std::floor((area->bbox().lowCorner()[1] - 1.f) / m_spacing));
    int hx=I_ROUND(std::ceil((area->bbox().highCorner()[0] + 1.f) / m_spacing));
    int hy=I_ROUND(std::ceil((area->bbox().highCorner()[1] + 1.f) / m_spacing));

    for (int i=lx;i<hx;++i) {
        for (int j=ly;j<hy;++j) {
            Segment *s=getSegment(i,j);
            if (s) {
                if (area->checkIntersects(*s)) {
                    s->addArea(area);
                }
            }
        } // of y loop
    } // of x loop
}

/// \brief Apply changes to an area modifier to the terrain.
Terrain::Rect Terrain::updateArea(const Area * area)
{
    std::set<Segment*> removed, added, updated;

     auto I = m_terrainAreas.find(area);

     Rect old_box;
     if (I != m_terrainAreas.end()) {

         old_box = I->second;

         int lx=I_ROUND(std::floor((old_box.lowCorner()[0] - 1.f) / m_spacing));
         int ly=I_ROUND(std::floor((old_box.lowCorner()[1] - 1.f) / m_spacing));
         int hx=I_ROUND(std::ceil((old_box.highCorner()[0] + 1.f) / m_spacing));
         int hy=I_ROUND(std::ceil((old_box.highCorner()[1] + 1.f) / m_spacing));

         for (int i=lx;i<hx;++i) {
            for (int j=ly;j<hy;++j) {
                Segment *s=getSegment(i,j);
                if (!s) {
                    continue;
                }

                removed.insert(s);

            } // of y loop
         } // of x loop

         I->second = area->bbox();

     } else {
         m_terrainAreas.emplace(area, area->bbox());
     }



     int lx=I_ROUND(std::floor((area->bbox().lowCorner()[0] - 1.f) / m_spacing));
     int ly=I_ROUND(std::floor((area->bbox().lowCorner()[1] - 1.f) / m_spacing));
     int hx=I_ROUND(std::ceil((area->bbox().highCorner()[0] + 1.f) / m_spacing));
     int hy=I_ROUND(std::ceil((area->bbox().highCorner()[1] + 1.f) / m_spacing));

     for (int i=lx;i<hx;++i) {
         for (int j=ly;j<hy;++j) {
             Segment *s=getSegment(i,j);
             if (!s) {
                 continue;
             }

             std::set<Segment*>::iterator J = removed.find(s);
             if (J == removed.end()) {
                 added.insert(s);
             } else {
                 updated.insert(s);
                 removed.erase(J);
             }
         } // of y loop
     } // of x loop

     for (auto& segment : removed) {
         segment->removeArea(area);
     }
     for (auto& segment : added) {
         if (area->checkIntersects(*segment)) {
             segment->addArea(area);
         }
     }
     for (auto& segment : updated) {
         if (area->checkIntersects(*segment)) {
             if (segment->updateArea(area) != 0) {
                 segment->addArea(area);
             }
         } else {
             segment->removeArea(area);
         }
     }

     return old_box;
}

/// \brief Remove an area modifier from the terrain.
///
/// Remove an existing Area object from the terrain, and mark all the
/// affected terrain surfaces as invalid.
void Terrain::removeArea(const Area * area)
{
    m_terrainAreas.erase(area);

    const Rect & eff_box = area->bbox();

    int lx=I_ROUND(std::floor((eff_box.lowCorner()[0] - 1.f) / m_spacing));
    int ly=I_ROUND(std::floor((eff_box.lowCorner()[1] - 1.f) / m_spacing));
    int hx=I_ROUND(std::ceil((eff_box.highCorner()[0] + 1.f) / m_spacing));
    int hy=I_ROUND(std::ceil((eff_box.highCorner()[1] + 1.f) / m_spacing));

    for (int i=lx;i<hx;++i) {
        for (int j=ly;j<hy;++j) {
            Segment *s=getSegment(i,j);
            if (s) {
                s->removeArea(area);
            }
        } // of y loop
    } // of x loop
}

bool Terrain::hasArea(const Area* a) const
{
    return m_terrainAreas.find(a) != m_terrainAreas.end();
}


} // namespace Mercator
