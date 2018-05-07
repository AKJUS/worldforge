#This file is distributed under the terms of the GNU General Public license.
#Copyright (C) 1999 Aloril (See the file COPYING for details).

class Knowledge:
    def __init__(self):
        self.place={}
        self.location={}
        self.goal={}
        self.importance={}
    def add(self, what, key, value):
        if not hasattr(self, what):
            setattr(self, what, {})
        d=getattr(self,what)
        d[key]=value
    def remove(self, what, key):
        if not hasattr(self, what):
            return
        d=getattr(self,what)
        if key in d:
            del d[key]
        if len(d)==0:
            delattr(self,what)
    def __str__(self):
        s="<know: "
        s=s+"place: "+str(self.place)+"\n"
        s=s+"location: "+str(self.location)+"\n"
        s=s+"goal: "+str(self.goal)+"\n"
        s=s+"importance: "+str(self.importance)+"\n"
        return s+">\n"

