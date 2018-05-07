#This file is distributed under the terms of the GNU General Public license.
#Copyright (C) 1999 Aloril (See the file COPYING for details).

from atlas import *

import server

class Garment(server.Thing):
    """This is base class for clothing. This mechanism is just a quick hacky
       way to modify the guise attribute."""
    def wear_operation(self, op):
        wearer = server.world.get_object(op.from_)
        guise = wearer.guise
        # return Operation("set",op[0],Entity(op.from_, ),to=to_)
        print(type(guise))
        print(guise)
        print(guise['mesh'])
        print(guise['mesh'].dress)
        print(guise['mesh'].head)
        print(guise['mesh'].pants)
        print(guise['material'])
        print(guise['material'].dress)
        print(guise['material'].head)
        print(guise['material'].pants)
