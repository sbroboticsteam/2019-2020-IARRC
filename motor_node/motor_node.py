from board import SCL, SDA
import busio
from adafruit_pca9685 import PCA9685
from time import sleep
from Node import Node
import numpy

node = Node("motor_node.json")
i2c_bus=busio.I2C(SCL, SDA)
pca = PCA9685(i2c_bus)

pca.frequency = 2000
print("node started")

while True:
	msg = str(node.recv_simple("motor"))
	mag = int(hex(int(msg.split(" ")[1])), 16)
	pca.channels[0].duty_cycle = mag
	print("mag:", mag)
