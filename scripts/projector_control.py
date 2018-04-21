
from ola.ClientWrapper import ClientWrapper
from projectorLib import Projector
import time

inUniverse  = 1
inAddr      = 430

proj1 = Projector("gymproj1.pius.org",1024,"gym","gym")
proj2 = Projector("gymproj2.pius.org",1024,"gym","gym")

curState = [proj1.getShutter(), proj2.getShutter()]
curPower = [proj1.getPower(), proj2.getPower()]
curTime = time.time()

print("Current shutter state = {}".format(curState))
#print("Current power   state = {}".format(curPower))

def rxData(data):
    global curState
    global curTime 

    newState = (data[inAddr-1] < 127)
    newTime = time.time()

    if newTime - curTime > 60:
        curState = [proj1.getShutter(), proj2.getShutter()]
        curTime = newTime
        print("Current shutter state = {}".format(curState))

    if newState != curState:
        curState = newState
        proj1.setShutter(curState)
        proj2.setShutter(curState)
        print("New shutter state = {}".format(curState))


wrapper = ClientWrapper()
client = wrapper.Client()

client.RegisterUniverse(inUniverse,client.REGISTER, rxData)

wrapper.Run()

