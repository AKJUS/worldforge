#This file is distributed under the terms of the GNU General Public license.
#Copyright (C) 1999 Aloril (See the file COPYING for details).

from atlas import *

from cyphesis import probability
from common import log,const

from physics import Vector3D
from physics import Point3D

import server

class House(server.Thing):
    """This base class for houses, building material is wood"""
    def tick_operation(self, op):
        """check if we should self-combust
           in any case send ourself next tick"""
        #print `self`,"Got tick operation:\n"
        opTick=Operation("tick",to=self)
        opTick.setFutureSeconds(const.basic_tick)
        if probability.does_it_happen(probability.fire_probability):
            fireEntity=Entity(name='fire',type=['fire'],status=0.0,
                              location=Location(self,Point3D(0.0,0.0,0.0)))
            opCreate=Operation("create",fireEntity,to=self)
            if const.debug_level>=2:
                print("Fire! "*30)
            return Oplist(opCreate,opTick)
        return opTick
