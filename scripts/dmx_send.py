from array import array

from ola.ClientWrapper import ClientWrapper

outUniverse = 3
client = None

def txData(status):
  if not status.Succeeded():
    print('Error: %s' % status.message)


wrapper = ClientWrapper()
client = wrapper.Client()

data = array(512)
data [14] = 255

client.SendDmx(outUniverse,data,txData)

wrapper.Run()

