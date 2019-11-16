from board import SCL, SDA
import busio
from adafruit_pca9685 import PCA9685
from time import sleep

i2c_bus=busio.I2C(SCL, SDA)
pca = PCA9685(i2c_bus)

pca.frequency = 2000

while True:
	pwm = input("Give hex: ")
	print(pwm)
	pca.channels[0].duty_cycle = int(pwm, 16)
	sleep(1)
	pca.channels[0].duty_cycle = 0x0000
	print("Resetting")
