
from ola.ClientWrapper import ClientWrapper
from projectorLib import Projector
import time
import dlipower

inUniverse  = 1
inAddr      = 509

ps1 = dlipower.PowerSwitch(hostname="172.16.50.120", userid="admin", password="1234")

curState = [None, None, None, None]

def rxData(data):
    global curState
    newState = [None, None, None, None]

    for i in range(len(curState)):
        newState[i] = (data[inAddr-1+i] > 127)

        if newState[i] != curState[i]:
            curState[i] = newState[i]
            if curState[i]:
                ps1.on(i+1)
            else:
                ps1.off(i+1)
            print("New ps {} state = {}".format(i,curState))

wrapper = ClientWrapper()
client = wrapper.Client()

client.RegisterUniverse(inUniverse,client.REGISTER, rxData)

wrapper.Run()

