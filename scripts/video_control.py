
from ola.ClientWrapper import ClientWrapper
import time

import pyautogui
pyautogui.PAUSE = 0.5
pyautogui.FAILSAFE = True

inUniverse  = 1
inAddr      = 430

curState = False
curTime = time.time()

def rxData(data):
    global curState
    global curTime 

    newState = (data[inAddr-1] > 127)
    newTime = time.time()

    if newTime - curTime > 60:
        curTime = time.time()
        print("Current video state = {}".format(curState))

    if newState != curState:
        curState = newState
        pyautogui.hotkey(' ')
        print("New video state = {}".format(curState))


wrapper = ClientWrapper()
client = wrapper.Client()

client.RegisterUniverse(inUniverse,client.REGISTER, rxData)

wrapper.Run()

