import paho.mqtt.client as mqtt
import json
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# MQTT Settings
BROKER = "broker.hivemq.com"
TOPIC = "home/sensors"

temps, hums, lights = [], [], []

def on_message(client, userdata, msg):
    global temps, hums, lights
    data = json.loads(msg.payload.decode())
    temps.append(data['temp'])
    hums.append(data['hum'])
    lights.append(data['light'])
    if len(temps) > 50:  # keep last 50 points
        temps, hums, lights = temps[-50:], hums[-50:], lights[-50:]

def animate(i):
    plt.clf()
    plt.subplot(3,1,1)
    plt.plot(temps, label="Temperature (°C)", color='red')
    plt.legend()

    plt.subplot(3,1,2)
    plt.plot(hums, label="Humidity (%)", color='blue')
    plt.legend()

    plt.subplot(3,1,3)
    plt.plot(lights, label="Light Level", color='green')
    plt.legend()

client = mqtt.Client()
client.on_message = on_message
client.connect(BROKER, 1883, 60)
client.subscribe(TOPIC)
client.loop_start()

ani = FuncAnimation(plt.gcf(), animate, interval=1000)
plt.show()
