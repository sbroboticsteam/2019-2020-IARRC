from joystick import Joystick
from Node import Node
import time

joystick = Joystick()
node = Node("joystick_node.json")
print("node started!")

while True:
	joystick.update()
	print("sending message!")
	node.send("motor", joystick.axis[1])
	time.sleep(0.1)
	
