
from ola.ClientWrapper import ClientWrapper
from projectorLib import Projector
import time
import dlipower

inUniverse  = 1
inAddr      = 509

ps1 = dlipower.PowerSwitch(hostname="gymps1.pius.org", userid="root", password="root")

curState = [None, None, None, None]

def rxData(data):
    global curState


    for i in range(len(curState)):
        newState[i] = (data[inAddr-1+i] < 127)

        if newState[i] != curState[i]:
            curState[i] = newState[i]
            if newState[i]:
                switch.on(i)
            else:
                switch.off(i)
        print("New ps {} state = {}".format(i,curState))


wrapper = ClientWrapper()
client = wrapper.Client()

client.RegisterUniverse(inUniverse,client.REGISTER, rxData)

wrapper.Run()

