#I2C Test Code 9/13/19 - AG
import jetsonI2C
import time
#setting the ultrasonic I2C address
ULTRASONIC_ADDRESS = 0x11  
Ultrasonic = jetsonI2C.jetsonI2C(1,ULTRASONIC_ADDRESS)

#confirm the device is ready
#print(Ultrasonic.read_bytes(0x00,1))
#Ultrasonic.write_byte(0x00,ULTRASONIC_ADDRESS)
#print(Ultrasonic.read_bytes(0x00,1))

#Configure Ultrasonic
CONFIG_REG = 0x07
DATA_REG = 0x03
#In the data sheet the ultrasonic has two modes, passive 0x00 (on command) or automatic 0x80
# ^ MSbits in the upper nibble
#We must add 0x40 to both of these to enable data recording
Passive = 0x40
Automatic = 0xA0
#Ultrasonic Data Range is determined by the by the LSbits in the upper nibble of the hex number
D_500 = 0x20
D_300 = 0x10
D_150 = 0x00
# Buffer for writing to the config register
Buffer = []
# Logical OR our settings in a convient form (since mode is upper nibble, Range is lower nibble)
Buffer.append(Automatic | D_300)
Ultrasonic.write_bytes(CONFIG_REG, Buffer)

#Run forever
while 1:
	#Read Data Reg
	Buffer = Ultrasonic.read_bytes(DATA_REG,2)
	#Bit shiting data into proper order
	Distance = (Buffer[0] << 8) | Buffer[1]
	#debug printing	
	print(Distance)
	

