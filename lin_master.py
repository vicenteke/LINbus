from machine import Pin, UART
from utime import sleep

class LINMaster:

    cs 	 = Pin(3, Pin.OUT)								# Pin connected to MC2003B's Chip Select
    uart = UART(1, 9600, parity=None, stop=1, bits=8, tx=4, rx=5)	# UART connected to MCP2003B
        
	def __init__(self):
        self.initialization()

	# Configures what's necessary to turn MCP2003B on operating mode
	def initialization():
		self.cs.on()

	# Asks data from 'slaveID' node
	# return data if Checksum/CRC OK, otherwise return -1 after three attempts
	def send_header(slaveID):
		
		if slaveID > 59 or slaveID < 2:
			print("Forbidden Operation: attempt to use a reserved ID.")
			return -1
		
		attempts = 0 # Checksum/CRC: if wrong, retry 3 times, return -1 if fail
		success = False
		
		while not success and attempts < 3:
		
			print("Trying to get data from", slaveID, "...")
		
			parity = 0
			for x in range(6):
				parity = parity + ((slaveID >> x) & 0x01)
			parity = parity % 2
			parity = (parity << 1) & 0x03
			
				
			uart.sendbreak() # send BREAK
			uart.write(0x55) # send SYNC (0x55) 
			uart.write(((slaveID & 0x3F << 2) + parity) & 0xFF) # send slaveID + parity
			# Is parity required?
				
			received = b''
			# byte = b''	
			while not uart.any():
				#sleep(0.1)
			while uart.any():
				byte = uart.read(1)
				received = received + byte
				
			data 		= received[0:7]
			crc 		= received[8:15]
			checksum 	= received[16:23]
			
			attempts += 1
			success = True
			
			
			if checksum != self.checksum(data):
				print("Checksum fail")
				success = False
				
			if crc != self.CRC(data):
				print("CRC fail")
				success = False
			
		if not success:
			return -1
			
		return data
			
	# Asks for data from 'slaveID' node every 'period' seconds for 'repetitions' times
	def start_periodic(slaveID, period = 20, repetitions = -1):
		if repetitions == -1:
			while True:
				if(self.send_header(slaveID) == -1):
					return
				sleep(period)
		i = 0
		while i < repetitions:
			i = i + 1
			if(self.send_header(slaveID) == -1):
				return
			sleep(period)
			
	# Calculates CRC
	def CRC(data):
		return 0
		
	# Calculates Checksum
	def checksum(data):
		return 0
		
		
def main():
	print(">>>>>> LIN Bus Master >>>>>>")
	try:
		lin = LINMaster()
		lin.start_periodic(2)
		print("The End: press Ctrl+C to quit...")
	except KeyboardInterrupt:
		raise
		
		
class LINSlave:

	def __init__(self, id):
        self.id = id
	
	def send_data(data):
		# send 2 bytes: 1 byte data + 1 byte CRC
		# send Checksum (1 byte)
		# --------------------------------------
		
		# calculates CRC and Checksum
		# send data
		# send CRC
		# send Checksum
		
		# return 1 if right,
		#		 0 if ignore data,
		#		-1 if parity error
		
		return 1
		
	def wait_request(): # --->> use rising edge interrupt to get after break (maybe one 1 still in message)
		# wait something in bus
		# get only data
		# check id
			# if not: ignore_data
		# check parity
			# if wrong: send something wrong so master will try again
		
		return send_data(0x55)
		
	def ignore_data(size):
		return 0
		
	def slave_main():
		# create class
		# while True: wait_request()
		# try/except KeyboardInterrupt
		return
		
	


