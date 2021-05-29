#include <Mercator/Area.h>
#include <Mercator/Terrain.h>
#include <Mercator/Segment.h>
#include <Mercator/Surface.h>
#include <Mercator/AreaShader.h>
#include <Mercator/FillShader.h>

#ifdef NDEBUG
#undef NDEBUG
#endif
#ifndef DEBUG
#define DEBUG
#endif

#include <cstdlib>
#include <cassert>
#include <iostream>
#include <fstream>

typedef WFMath::Point<2> Point2;

void writePGMForSurface(const std::string& fileName, int sz, Mercator::Surface* s)
{
    assert(s);
    
    std::ofstream file(fileName.c_str());
    file << "P5" << std::endl;
    file << sz << ' ' << sz << " 255" << std::endl;
    
    // now just blast out the binary
    file.write((const char*) s->getData(), sz * sz);
    file.close();
}

void testAreaShader()
{
    auto a1 = std::make_unique<Mercator::Area>(1, false);
    auto a1_ptr= a1.get();
    
    WFMath::Polygon<2> p;
    p.addCorner(p.numCorners(), Point2(3, 4));
    p.addCorner(p.numCorners(), Point2(10, 10));
    p.addCorner(p.numCorners(), Point2(14, 6));
    p.addCorner(p.numCorners(), Point2(18, 4));
    p.addCorner(p.numCorners(), Point2(17, 19));
    p.addCorner(p.numCorners(), Point2(6, 20));
    p.addCorner(p.numCorners(), Point2(-1, 18));
    p.addCorner(p.numCorners(), Point2(-8, 11));
    
    a1->setShape(p);
    
    auto a2 = std::make_unique<Mercator::Area>(1, false);
    
    WFMath::Polygon<2> p2;
    p2.addCorner(p2.numCorners(), Point2(25, 18));
    p2.addCorner(p2.numCorners(), Point2(72, 22));
    p2.addCorner(p2.numCorners(), Point2(60, 30));
    p2.addCorner(p2.numCorners(), Point2(27, 28));
    p2.addCorner(p2.numCorners(), Point2(25, 45));
    p2.addCorner(p2.numCorners(), Point2(3, 41));
    p2.addCorner(p2.numCorners(), Point2(-2, 20));
    a2->setShape(p2);
    
    Mercator::Terrain terrain(Mercator::Terrain::SHADED, 16);
    
    Mercator::FillShader base_shader{Mercator::Shader::Parameters()};
    terrain.addShader(&base_shader, 0);

    Mercator::AreaShader ashade(1);
    terrain.addShader(&ashade, 1);
    
    terrain.setBasePoint(0, 0, -1);
    terrain.setBasePoint(0, 1, 8);
    terrain.setBasePoint(1, 0, 2);
    terrain.setBasePoint(1, 1, 11);
    terrain.setBasePoint(2, 0, 2);
    terrain.setBasePoint(2, 1, 11);
    
    terrain.updateArea(1, std::move(a1));
   // terrain.addArea(a2);
    
    Mercator::Segment* seg = terrain.getSegmentAtIndex(0,0);
    assert(a1_ptr->checkIntersects(*seg));
    
    seg->populateSurfaces();
    writePGMForSurface("test1.pgm", seg->getSize(), seg->getSurfaces()[1].get());
    
    
    seg = terrain.getSegmentAtIndex(1,0);
    seg->populateSurfaces();
    writePGMForSurface("test2.pgm", seg->getSize(), seg->getSurfaces()[1].get());
}

static const unsigned int seg_size = 8;
    
void testCheckIntersect()
{
    Mercator::Area a1(1, false);
    
    WFMath::Polygon<2> p;
    p.addCorner(p.numCorners(), Point2(1, 1));
    p.addCorner(p.numCorners(), Point2(6, 1));
    p.addCorner(p.numCorners(), Point2(6, 6));
    p.addCorner(p.numCorners(), Point2(1, 6));
    
    a1.setShape(p);
    
    Mercator::Segment seg1(0,0,seg_size);

    bool success = a1.checkIntersects(seg1);
    assert(success);

    Mercator::Segment seg2 (1 * seg_size,0,seg_size);

    success = a1.checkIntersects(seg2);
    assert(!success);
}

int main(int argc, char* argv[])
{
    auto a1 = std::make_unique<Mercator::Area>(1, false);
    auto a1_ptr = a1.get();
    WFMath::Polygon<2> p;
    p.addCorner(p.numCorners(), Point2(3, 4));
    p.addCorner(p.numCorners(), Point2(10, 10));
    p.addCorner(p.numCorners(), Point2(-1, 18));
    p.addCorner(p.numCorners(), Point2(-8, 11));
    
    a1->setShape(p);
    
    Mercator::Terrain terrain(Mercator::Terrain::SHADED, seg_size);

    Mercator::AreaShader ashade(1);
    terrain.addShader(&ashade, 0);
    
    terrain.setBasePoint(-2, -1, 5);
    terrain.setBasePoint(-2, 0, 2);
    terrain.setBasePoint(-2, 1, 19);
    
    terrain.setBasePoint(-1, -1, 4);
    terrain.setBasePoint(-1, 0, 6);
    terrain.setBasePoint(-1, 1, 10);
    
    terrain.setBasePoint(0, -1, 2);
    terrain.setBasePoint(0, 0, -1);
    terrain.setBasePoint(0, 1, 8);
    terrain.setBasePoint(0, 2, 11);
    
    terrain.setBasePoint(1, -1, 7);
    terrain.setBasePoint(1, 0, 2);
    terrain.setBasePoint(1, 1, 11);
    terrain.setBasePoint(1, 2, 9);
    
    terrain.setBasePoint(2, -1, 3);
    terrain.setBasePoint(2, 0, 8);
    terrain.setBasePoint(2, 1, 2);

    terrain.setBasePoint(3, -1, 6);
    terrain.setBasePoint(3, 0, 7);
    terrain.setBasePoint(3, 1, 9);
    
    terrain.updateArea(1, std::move(a1));
    
    Mercator::Segment* seg = terrain.getSegmentAtIndex(0,0);
    assert(seg->getAreas().size() == 1);
    assert(seg->getAreas().count(1) == 1);
    assert(a1_ptr->checkIntersects(*seg));
    
    seg = terrain.getSegmentAtIndex(1,0);
    assert(seg->getAreas().empty());
    assert(seg->getAreas().count(1) == 0);
    assert(a1_ptr->checkIntersects(*seg) == false);

    WFMath::Polygon<2> clipped = a1_ptr->clipToSegment(*seg);
    assert(clipped.isValid());
    
    seg = terrain.getSegmentAtIndex(-1,0);
    assert(seg->getAreas().size() == 1);
    assert(seg->getAreas().count(1) == 1);
    assert(a1_ptr->checkIntersects(*seg));
    
    clipped = a1_ptr->clipToSegment(*seg);
    assert(clipped.isValid());
    
    seg = terrain.getSegmentAtIndex(0,1);
    assert(seg->getAreas().size() == 1);
    assert(seg->getAreas().count(1) == 1);
    assert(a1_ptr->checkIntersects(*seg));
    
    clipped = a1_ptr->clipToSegment(*seg);
    assert(clipped.isValid());

    seg = terrain.getSegmentAtIndex(2,0);
    assert(seg->getAreas().empty());
    assert(seg->getAreas().count(1) == 0);
    assert(a1_ptr->checkIntersects(*seg) == false);

    p.clear();
    p.addCorner(p.numCorners(), Point2(3 + seg_size, 4));
    p.addCorner(p.numCorners(), Point2(10 + seg_size, 10));
    p.addCorner(p.numCorners(), Point2(-1 + seg_size, 18));
    p.addCorner(p.numCorners(), Point2(-8 + seg_size, 11));

	auto a1_2 = std::make_unique<Mercator::Area>(1, false);
	auto a1_2_ptr = a1_2.get();
	a1_2->setShape(p);

    terrain.updateArea(1, std::move(a1_2));

    seg = terrain.getSegmentAtIndex(0,0);
    assert(seg->getAreas().size() == 1);
    assert(seg->getAreas().count(1) == 1);
    assert(a1_2_ptr->checkIntersects(*seg));
    
    seg = terrain.getSegmentAtIndex(1,0);
    assert(seg->getAreas().size() == 1);
    assert(seg->getAreas().count(1) == 1);
    assert(a1_2_ptr->checkIntersects(*seg));

    clipped = a1_2_ptr->clipToSegment(*seg);
    assert(clipped.isValid());
    
    seg = terrain.getSegmentAtIndex(-1,0);
    assert(seg->getAreas().empty());
    assert(seg->getAreas().count(1) == 0);
    assert(a1_2_ptr->checkIntersects(*seg) == false);
    
    seg = terrain.getSegmentAtIndex(0,1);
    assert(seg->getAreas().size() == 1);
    assert(seg->getAreas().count(1) == 1);
    assert(a1_2_ptr->checkIntersects(*seg));
    
    clipped = a1_2_ptr->clipToSegment(*seg);
    assert(clipped.isValid());

    seg = terrain.getSegmentAtIndex(2,0);
    assert(seg->getAreas().empty());
    assert(seg->getAreas().count(1) == 0);
    assert(a1_2_ptr->checkIntersects(*seg) == false);

    clipped = a1_2_ptr->clipToSegment(*seg);
    assert(clipped.isValid());

    terrain.updateArea(1, nullptr);

    seg = terrain.getSegmentAtIndex(0,0);
    assert(seg->getAreas().empty());
    assert(seg->getAreas().count(1) == 0);

    seg = terrain.getSegmentAtIndex(1,0);
    assert(seg->getAreas().empty());
    assert(seg->getAreas().count(1) == 0);

    seg = terrain.getSegmentAtIndex(-1,0);
    assert(seg->getAreas().empty());
    assert(seg->getAreas().count(1) == 0);

    seg = terrain.getSegmentAtIndex(0,1);
    assert(seg->getAreas().empty());
    assert(seg->getAreas().count(1) == 0);

    testAreaShader();

    testCheckIntersect();
    
    return EXIT_SUCCESS;
}
