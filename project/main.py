from flask import Flask, flash, request, redirect, url_for, render_template, Blueprint,request
from flask_login import login_required, current_user
from paho.mqtt import client as mqtt_client
from werkzeug.utils import secure_filename
from datetime import datetime, date
from pathlib import Path
from io import BytesIO
from PIL import Image
import numpy as np
from . import db
import os.path
import random
import base64
import pyodbc
import pickle
import time
import cv2
import os

broker = '127.0.0.1'
port = 1883
users = {}

def connection():
    # sqlcmd -S localhost,1415 -U SA -P 7u8i9o0P
    s = 'localhost,1415' #Your server name 
    d = 'TblUsers' 
    u = 'SA' #Your login
    p = '7u8i9o0P' #Your login password
    cstr = 'DRIVER={ODBC Driver 17 for SQL Server};SERVER='+s+';DATABASE='+d+';UID='+u+';PWD='+ p
    conn = pyodbc.connect(cstr)
    return conn

def subscribe(client: mqtt_client, topic):
    def on_message(client, userdata, msg):
        #print(f"Received `{msg.payload.decode()}` from `{msg.topic}` topic")
        global messageAux, im
        partner = []
        messageAux = msg.payload.decode()
        users[str(topic)] = messageAux
        conn = connection()
        cursor = conn.cursor()
        cursor.execute("SELECT * FROM dbo.TblUsers WHERE ID = ?", messageAux)
        for row in cursor.fetchall():
            partner.append({"id": row[0], "name": row[1]})
        conn.close()
        
        year = date.today().strftime("%Y")
        month = date.today().strftime("%m")
        day = date.today().strftime("%d")
        if not os.path.exists('logs/' + str(year) + "/"):
            os.mkdir('logs/' + str(year) + "/")
        if not os.path.exists('logs/' + str(year) + "/" + str(month) + "/"):
            os.mkdir('logs/' + str(year) + "/" + str(month) + "/")
        if not os.path.exists('logs/' + str(year) + "/" + str(month) + "/" + str(day) + ".pickle"):
            file = open('logs/' + str(year) + "/" + str(month) + "/" + str(day) + ".pickle", 'wb')
            file.close()
        if len(partner) == 1:
            try:
                with open('logs/' + str(year) + "/" + str(month) + "/" + str(day) + ".pickle", 'ab') as irSensorFile:
                    pickle.dump([partner[0]["name"], datetime.now().strftime('%Y-%m-%d_%H-%M-%S-%f')], irSensorFile)
                    partner = []
            except Exception as err:
                print(err)
        
        # print(datetime.now().strftime('%Y-%m-%d_%H-%M-%S-%f'))
        # print(partner)
        # print(users)
        # print(messageAux)
    
    client.subscribe(topic)
    client.on_message = on_message

def publish(client, msg):
    topic = msg.split("/")[0]
    result = client.publish(msg.split("/")[0], msg)
    status = result[0]
    if status == 0:
        print(f"Send `{msg}` to topic `{topic}`")
    else:
        print(f"Failed to send message to topic {topic}")

def connect_mqtt():
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
        else:
            print("Failed to connect, return code %d\n", rc)

    client = mqtt_client.Client(f'python-mqtt-{random.randint(0, 1000)}')
    client.on_connect = on_connect
    client.connect(broker, port)
    return client


main = Blueprint('main', __name__)

global client 
client = connect_mqtt()
subscribe(client, 'hendrix')
client.loop_start()

@main.route("/", methods=['GET', 'POST'])
@main.route('/index', methods=['GET', 'POST'])
def index():
    return render_template('index.html')

@main.route('/profile')
def profile():
    return render_template('profile.html', name=current_user.name)

@main.route('/mote_access', methods=['POST'])
def mote_access():
    mote = request.form.get('mote')
    return render_template('mote_access.html', mote=mote)

@main.route('/mote_status', methods=['GET', 'POST'])
def mote_status():
    id = current_user.rfid_id
    
    publish(client, "hendrix/" + str(id))
    time.sleep(8)
    
    return render_template('mote_status.html', info=users['hendrix'])
    
