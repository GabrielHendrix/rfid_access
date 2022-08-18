from paho.mqtt import client as mqtt_client
import random
import pyodbc
import time
import paho.mqtt.client as mqtt
def connection():
    # sqlcmd -S localhost,1415 -U SA -P 7u8i9o0P
    s = 'localhost,1415' #Your server name 
    d = 'TblUsers' 
    u = 'SA' #Your login
    p = '7u8i9o0P' #Your login password
    cstr = 'DRIVER={ODBC Driver 17 for SQL Server};SERVER='+s+';DATABASE='+d+';UID='+u+';PWD='+ p
    conn = pyodbc.connect(cstr)
    return conn

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("hendrix")

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    #print(f"Received `{msg.payload.decode()}` from `{msg.topic}` topic")
    global messageAux, im
    partners = []
    messageAux = msg.payload.decode()
    messageAux
    conn = connection()
    cursor = conn.cursor()
    cursor.execute("SELECT * FROM dbo.TblUsers")
    for row in cursor.fetchall():
        partners.append({"id": row[0], "name": row[1]})
    conn.close()
    print(partners)
    # print(messageAux) 

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect('143.244.159.171', 1883, 60)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client.loop_forever()
