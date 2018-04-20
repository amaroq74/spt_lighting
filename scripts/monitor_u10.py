from ola.ClientWrapper import ClientWrapper

def newData(data):
    #print(len(data))
    print(data)
    #print(data[0])

universe = 1

wrapper = ClientWrapper()
client = wrapper.Client()
client.RegisterUniverse(universe,client.REGISTER, newData)
wrapper.Run()

