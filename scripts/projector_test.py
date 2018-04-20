
from projectorLib import Projector

inUniverse  = 1
inAddr      = 430

proj1 = Projector("gymproj1.pius.org",1024,"gym","gym")
proj2 = Projector("gymproj2.pius.org",1024,"gym","gym")

curState = [proj1.getShutter(), proj2.getShutter()]

print("Current shutter state = {}".format(curState))

curState = False
proj1.setShutter(curState)
proj2.setShutter(curState)


