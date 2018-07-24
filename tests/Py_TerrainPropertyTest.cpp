// Cyphesis Online RPG Server and AI Engine
// Copyright (C) 2009 Alistair Riddoch
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA


#ifdef NDEBUG
#undef NDEBUG
#else
#define CYPHESIS_DEBUG
#endif
#ifndef DEBUG
#define DEBUG
#endif

#include <Python.h>

#include "python_testers.h"

#include "rulesets/Python_API.h"
#include "rulesets/Entity.h"
#include "rulesets/TerrainProperty.h"

#include <cassert>

#include "external/pycxx/CXX/Extensions.hxx"
#include "rulesets/python/CyPy_Entity.h"


class TestProp : public Py::ExtensionModule<TestProp>
{
    public:
        Py::Object add_properties(const Py::Tuple& args)
        {
            auto ent = CyPy_Entity::value(args.front());

            PropertyBase * p = ent->setProperty("terrain", new TerrainProperty);
            p->install(ent, "terrain");
            p->apply(ent);
            ent->propertyApplied("terrain", *p);

            return Py::None();
        }

        TestProp() : ExtensionModule("testprop")
        {
            add_varargs_method("add_properties", &TestProp::add_properties, "");

            initialize("testprop");
        }

};

int main()
{
    PyImport_AppendInittab("testprop", [](){
        auto module = new TestProp();
        return module->module().ptr();
    });
    init_python_api("bac81904-0516-4dd0-b9d8-32e879339b96");

    run_python_string("from server import *");
    run_python_string("import testprop");
    run_python_string("t=Thing('1')");
    expect_python_error("t.terrain", PyExc_AttributeError);
    run_python_string("testprop.add_properties(t)");
    run_python_string("terrain = t.props.terrain");
    expect_python_error("terrain.foo = 1", PyExc_AttributeError);
    expect_python_error("terrain.get_height()", PyExc_TypeError);
    run_python_string("terrain.get_height(0,0)");
    expect_python_error("terrain.get_surface()", PyExc_TypeError);
    expect_python_error("terrain.get_surface('1')", PyExc_TypeError);
    run_python_string("from physics import *");
    expect_python_error("terrain.get_surface(Point3D(0,0,0))", PyExc_TypeError);
    expect_python_error("terrain.get_normal()", PyExc_TypeError);
    run_python_string("terrain.get_normal(0,0)");
    run_python_string("terrain.find_mods(Point3D(0,0,0))");

    run_python_string("points = { }");
    run_python_string("points['-1x-1'] = [-1, -1, -16.8]");
    run_python_string("points['0x-1'] = [0, -1, -3.8]");
    run_python_string("points['-1x0'] = [-1, 0, -2.8]");
    run_python_string("points['-1x1'] = [-1, 1, -1.8]");
    run_python_string("points['1x-1'] = [1, -1, 15.8]");
    run_python_string("points['0x0'] = [0, 0, 12.8]");
    run_python_string("points['1x0'] = [1, 0, 23.1]");
    run_python_string("points['0x1'] = [0, 1, 14.2]");
    run_python_string("points['1x1'] = [1, 1, 19.7]");
    run_python_string("t.props.terrain = {'points': points}");

    //No surfaces until "surfaces" is defined.
    expect_python_error("terrain.get_surface(Point3D(0,0,0))", PyExc_TypeError);

    run_python_string("surface = {'name': 'rock', 'pattern': 'fill'}");
    run_python_string("surfaces = [surface]");
    run_python_string("t.props.terrain = {'points': points, 'surfaces': surfaces}");

    run_python_string("terrain.get_surface(Point3D(0,0,0))");

    shutdown_python_api();
    return 0;
}
