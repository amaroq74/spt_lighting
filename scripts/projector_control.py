
from ola.ClientWrapper import ClientWrapper
from projectorLib import Projector
import time

inUniverse  = 1
inAddr      = 430

#projs = [ Projector("gymproj1.pius.org",1024,"gym","gym"),
#          Projector("gymproj2.pius.org",1024,"gym","gym") ]

projs = [ Projector("gymproj1.pius.org",1024,"gym","gym") ]

curState = all([proj.getShutter() for proj in projs])
curTime = time.time()

print("Current shutter state = {}".format(curState))

def rxData(data):
    global curState
    global curTime 

    newState = (data[inAddr-1] < 127)
    newTime = time.time()

    if newTime - curTime > 60:
        curState = all( [proj.getShutter() for proj in projs])
        curTime = time.time()
        print("Current shutter state = {}".format(curState))

    if newState != curState:
        curState = newState
        for proj in projs:
            proj.setShutter(curState)
        print("New shutter state = {}".format(curState))


wrapper = ClientWrapper()
client = wrapper.Client()

client.RegisterUniverse(inUniverse,client.REGISTER, rxData)

wrapper.Run()

