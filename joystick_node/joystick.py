import ctypes
import time
from sdl2 import *
from numpy import interp


class Joystick():

	def __init__(self):

		SDL_Init(SDL_INIT_JOYSTICK)
		self.axis = {1: "0", 4: "0"}
		self.button = {5: False, 1: False, 0: False}

	def update(self):
		event = SDL_Event()
		while SDL_PollEvent(ctypes.byref(event)) != 0:
			if event.type == SDL_JOYDEVICEADDED:
				self.device = SDL_JoystickOpen(event.jdevice.which)
			elif event.type == SDL_JOYAXISMOTION:
				yval = event.jaxis.value
				#yinter = int(interp(yval, [-32768, 32767], [0,255]))
				#yfinal = interp(yinter, [0, 255], [-1.0, 1.0]) * -1
				yinter = int(yval) + 32768
				if -1000 < yval < 1000:
					yinter = 32767
				#if yinter > 65535:
				#	yinter = 65535
				#if yinter < -65535:
				#	yinter = -65535
				yfinal = yinter
				self.axis[event.jaxis.axis] = str(yfinal)
			elif event.type == SDL_JOYBUTTONDOWN:
				self.button[event.jbutton.button] = True
			elif event.type == SDL_JOYBUTTONUP:
				self.button[event.jbutton.button] = False


if __name__ == "__main__":
	joystick = Joystick()
	while True:
		joystick.update()
		time.sleep(0.1)
		print("Axis", joystick.axis)
		print("Button", joystick.button)

		if joystick.button[5]:
			print("pressed")
