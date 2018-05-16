#This file is distributed under the terms of the GNU General Public license.
#Copyright (C) 1999 Aloril (See the file COPYING for details).

from atlas import *
try:
  from random import *
except ImportError:
  from whrandom import *
from mind.panlingua import interlinguish
il=interlinguish
from .world import probability
from editor import editor
import time

#goal priority
#1) eating: certain times
#2) market/tavern: certain times, certain probability
#3) sleeping: nights
#4) chop trees: winter (when not doing anything else)
#4) other similar tasks: seasonal (-"-)

#'Breakfast' goal is type of 'eating'.

village_height=0
gallows_pos=(7,7,village_height)

knowledge=[('gallows',gallows_pos)]

execute=(il.execute,"elect()")

count=[(il.trade,"count_players()"),
       (il.trade,"decount_players()")]

hangman_goals=[execute]

#observer calls this
def default(mapeditor):
#   general things

    m=editor(mapeditor)

# a wall around the world, suitable for all games.

    m.make('wall',type='wall',pos=(-151,-101,village_height),bbox=[1,102,2.5])
    m.make('wall',type='wall',pos=(-151,-101,village_height),bbox=[152,1,2.5])
    m.make('wall',type='wall',pos=(-151,100,village_height),bbox=[152,1,2.5])
    m.make('wall',type='wall',pos=(100,-101,village_height),bbox=[1,102,2.5])

    m.make('weather',type='weather',desc='object that describes the weather',
           pos=(0,1,0), rain=0.0)

#   the gallows
    gallows = m.make('gallows', type='gallows', pos=gallows_pos)

#   the mayor, who runs the game
    mayor=m.make('mayor', type='mayor', pos=(1, 1, village_height))

#   the hangman, who lynches the villagers
    hangman=m.make('hangman', type='hangman', pos=(5, 5, village_height))
    m.learn(hangman,execute)
    m.know(hangman,knowledge)
    m.own(hangman,gallows)
