from ola.ClientWrapper import ClientWrapper

inUniverse  = 2
outUniverse = 3
client = None

def txData(status):
  if not status.Succeeded():
    print('Error: %s' % status.message)

def rxData(data):
    for i in range(len(data)):
        if data[i] < 25:
            data[i] = 0
        elif data[i] > 200:
            data[i] = 255
        else:
            data[i] = int(((float(data[i])-25.0)/200.0) * 255.0)

    client.SendDmx(outUniverse,data,txData)

wrapper = ClientWrapper()
client = wrapper.Client()

client.RegisterUniverse(inUniverse,client.REGISTER, rxData)

wrapper.Run()

