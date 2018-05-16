from atlas import *

from cyphesis.Thing import Thing
from misc import set_kw

class Ribcage(Thing):
    def __init__(self, cppthing, **kw):
        self.base_init(cppthing, kw)
        set_kw(self,kw,"mass",1.0)
