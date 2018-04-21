from ola.ClientWrapper import ClientWrapper
import time

inUniverse  = 1
outUniverse = 3
client = None
last = 0
tme  = int(time.time())

def txData(status):
  if not status.Succeeded():
    print('Error: %s' % status.message)

def rxData(data):
    print("got data")
    global last
    global tme

    for i in range(len(data)):
        data[i] = 0

    cur = int(time.time())

    if cur != tme:
        if last == 0:
            last = 255
        else:
            last = 0

        data[0] = last
        print("Sending: {}".format(last))
        tme = cur

    client.SendDmx(outUniverse,data,txData)

wrapper = ClientWrapper()
client = wrapper.Client()

client.RegisterUniverse(inUniverse,client.REGISTER, rxData)

wrapper.Run()

