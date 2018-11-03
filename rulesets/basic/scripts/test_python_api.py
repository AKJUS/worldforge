# This file is distributed under the terms of the GNU General Public license.
# Copyright (C) 2006 Al Riddoch (See the file COPYING for details).

import sys
import atlas
import Vector3D
import Point3D


def default(mapeditor):
    print("Test logging")
    sys.stdout.write("Test stdout\n")
    sys.stderr.write("Test stderr\n")

    print("Test operation")
    operation = atlas.Operation("move")
    assert (len(operation) == 0)
    operation = atlas.Operation("move", atlas.Entity("1"))
    assert (len(operation) == 1)
    assert (operation[0].id == "1")
    operation = atlas.Operation("move", atlas.Entity("2"), atlas.Entity("3"))
    assert (len(operation) == 2)
    assert (operation[0].id == "2")
    assert (operation[1].id == "3")
    operation = atlas.Operation("move", atlas.Entity("4"), atlas.Entity("5"), atlas.Entity("6"))
    assert (len(operation) == 3)
    assert (operation[0].id == "4")
    assert (operation[1].id == "5")
    assert (operation[2].id == "6")
    operation = atlas.Operation("move", atlas.Entity("7"), to="8", from_="9")
    assert (len(operation) == 1)
    assert (operation[0].id == "7")
    assert (operation.to == "8")
    assert (operation.from_ == "9")

    assert (operation.get_serialno() != 452)
    assert (operation.get_refno() != 567)
    assert (operation.get_from() != "10")
    assert (operation.get_to() != "11")
    assert (operation.get_seconds() != 577)
    assert (operation.get_future_seconds() != 345)

    operation.set_serialno(452)
    operation.set_refno(567)
    operation.set_from("10")
    operation.set_to("11")
    operation.set_seconds(577.5)
    operation.set_future_seconds(345.5)
    operation.set_args([atlas.Operation("set")])

    assert (operation.get_serialno() == 452)
    assert (operation.get_refno() == 567)
    assert (operation.get_from() == "10")
    assert (operation.get_to() == "11")
    assert (operation.get_seconds() == 577.5)
    assert (operation.get_future_seconds() == 345.5)
    assert (len(operation.get_args()) == 1)

    print("Test location")
    a = 1
    if atlas.isLocation(a):
        raise AssertionError("atlas.isLocation returned true on an integer")
    location = rules.Location()
    if not atlas.isLocation(location):
        raise AasertionError("atlas.isLocation returned false on a Location")

    # FIXME No current way to create an entity
    # location=rules.Location('23')
    # location=rules.Location('42', Vector3D(1,0,0))

    print("Test entity")
    entity = atlas.Entity()
    entity = atlas.Entity("1")
    assert (entity.id == "1")
    # FIXME other methods

    print("Test message")
    message = atlas.Oplist()
    assert (len(message) == 0)
    message.append(atlas.Operation("move"))
    assert (len(message) == 1)
    message = atlas.Oplist(atlas.Operation("move"))
    assert (len(message) == 1)
    # FIXME other methods

    print("Test vector")
    vector = Vector3D.Vector3D()
    vector = Vector3D.Vector3D([0, 0, 1])
    vector = Vector3D.Vector3D([0.1, 0.1, 1.1])
    vector = Vector3D.Vector3D(0, 0, 1)
    vector = Vector3D.Vector3D(0.1, 0.1, 1.1)
    # FIXME other methods

    print("Test point")
    vector = Point3D.Point3D()
    vector = Point3D.Point3D([0, 0, 1])
    vector = Point3D.Point3D([0.1, 0.1, 1.1])
    vector = Point3D.Point3D(0, 0, 1)
    vector = Point3D.Point3D(0.1, 0.1, 1.1)
    # FIXME other methods
