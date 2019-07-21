# This file is distributed under the terms of the GNU General Public license.
# Copyright (C) 2006 Al Riddoch (See the file COPYING for details).

from atlas import *
from physics import *
from physics import Quaternion
from physics import Point3D
from physics import Vector3D
from rules import Location

import server


class Dig(server.Task):
    """ A task for digging soft material from the terrain."""

    materials = {1: 'sand', 2: 'earth', 3: 'silt'}

    def cut_operation(self, op):
        """ Op handler for cut op which activates this task """
        # print "Dig.cut"

        if len(op) < 1:
            sys.stderr.write("Dig task has no target in cut op")

        # FIXME Use weak references, once we have them
        self.target = server.world.get_object(op[0].id)
        self.tool = op.to

        self.pos = Point3D(op[0].pos)

    def info_operation(self, op):
        print("Dig.info")

    def tick_operation(self, op):
        """ Op handler for regular tick op """
        # print "Dig.tick"

        if self.target() is None:
            # print "Target is no more"
            self.irrelevant()
            return

        old_rate = self.rate

        self.rate = 0.1 / 1.75
        self.progress += 0.1

        if old_rate < 0.01:
            self.progress = 0

        # print "%s" % self.pos

        if self.progress < 1:
            # print "Not done yet"
            return self.next_tick(1.75)

        self.progress = 0

        surface = self.target().props.terrain.get_surface(self.pos)
        # print "SURFACE %d at %s" % (surface, self.pos)
        if surface not in Dig.materials:
            print("Not right")
            self.irrelevant()
            return

        res = Oplist()

        chunk_loc = Location(self.character.location.parent)
        chunk_loc.velocity = Vector3D()

        chunk_loc.pos = self.pos

        create = Operation("create",
                           Entity(name=Dig.materials[surface],
                                  type="pile",
                                  material=Dig.materials[surface],
                                  location=chunk_loc), to=self.target())
        create.set_serialno(0)
        res.append(create)

        res.append(self.next_tick(1.75))

        return res
