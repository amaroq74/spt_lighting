#!/usr/bin/env python3

import paho.mqtt.client as mqtt

ListenTopics = {'stat/fc/door1' : {'name':'East_Door', 'last':0 },
                'stat/fc/door2' : {'name':'West_Door', 'last':0 },
                'stat/fc/wifi'  : None }

def on_connect(client, userdata, flags, rc):
    print("Connected to mqtt server")
    for k,v in ListenTopics.items():
        client.subscribe(k)

def on_message(client, userdata, msg):
    try:
        data = ListenTopics[msg.topic]

        if data is None:
            print("WIFI = state = {}".format(msg.payload.decode('utf-8')))
        else:
            door  = ListenTopics[msg.topic]['name']
            state   = 1 if str(msg.payload.decode('utf-8')) == 'Open' else 0
            changed = 0 if state == ListenTopics[msg.topic]['last'] else 1

            print(f"door = {door}, state = {state}, changed = {changed}")

    except Exception as e:
        print('*** Got Error ({})***'.format(e))
        exit()

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("aliska.amaroq.net", 1883, 60)

client.loop_forever()

