# Not used

from machine import Pin, UART
from time import sleep

class LINMaster:

	cs 	 = Pin(3, Pin.OUT)								# Pin connected to MC2003B's Chip Select
	uart = UART(1, 9600, parity=None, stop=1, bits=8)	# UART connected to MCP2003B

	led = Pin(25, Pin.OUT) # LED signaling proper operation
        
	def __init__(self):
		self.initialization()

	# Configures what's necessary to turn MCP2003B on operating mode
	def initialization(self):
		self.cs.on()

	# Asks data from 'slaveID' node
	# return data if Checksum/CRC OK, otherwise return -1 after three attempts
	def send_header(self, slaveID):
		
		if slaveID > 59 or slaveID < 2:
			print("Forbidden Operation: attempt to use a reserved ID.")
			return -2
		
		attempts = 0 # Checksum/CRC: if wrong, retry 3 times, return -1 if fail
		success = False

		self.led.on()
		
		while not success and attempts < 3:
		
			print("Trying to get data from", slaveID, "...")
		
			parity = 0
			for x in range(6):
				parity = parity + ((slaveID >> x) & 0x01)
			parity = parity % 2
			parity = (parity << 1) & 0x03
			
			uart_buf = [0x55, (((slaveID & 0x3F << 2) + parity) & 0xFF)]
			
			self.uart.sendbreak() # send BREAK
			self.uart.write(bytes([uart_buf[0]])) # send SYNC (0x55) 
			self.uart.write(bytes([uart_buf[1]])) # send slaveID + parity
			# Is parity required?
				
			received = b''
			# byte = b''	
			while not self.uart.any():
				#sleep(0.1)
				pass
			while self.uart.any():
				byte = self.uart.read(1)
				received = received + byte
				
			data 		= received[0]
			codeword	= received[0:2]
			
			attempts += 1
			success = True
			
			
			if not self.checksum(received):
				print(">> Checksum fail")
				success = False
				
			if not self.CRCDecode(codeword):
				print(">> CRC fail")
				success = False
			
		self.led.off()

		if not success:
			return -1
			
		return data
			
	# Asks for data from 'slaveID' node every 'period' seconds for 'repetitions' times
	def start_periodic(self, slaveID, period = 8, repetitions = -1):
		if repetitions == -1:
			while True:
				if(self.send_header(slaveID) < 0):
					return
				sleep(period)

		i = 0
		data = 0
		while i < repetitions:
			i = i + 1
			data = self.send_header(slaveID)
			if data < 0:
				return

			print("Received: ", data)
			sleep(period)
			
	# Checks CRC
	def CRCDecode(self, data):
	    data = int.from_bytes(data, "big")
	    res = (data >> 8) & 0xFF

	    for i in range(8):
	        res = (res << 1) & 0x1FF
	        res += ((data >> (7 - i)) & 0x1) & 0x1FF
	        res ^= 0xD5 * int(res / 256)
	        
	    if res == 0:
	        return True

	    return False
		
	# Calculates Checksum
	def checksum(self, data):
		soma = sum(data)
		res = (soma & 0xFF) + ((soma >> 8) & 0xFF)
		res = (~(res) & 0xFF)

		if res == 0:
			return True

		return False
		
		
def main():
	sleep(2)
	print(">>>>>> LIN Bus Master >>>>>>")
	try:
		lin = LINMaster()
		lin.start_periodic(2)
		print("The End: press Ctrl+C to quit...")
	except KeyboardInterrupt:
		raise
		
if __name__ == "__main__":
    main()
		
# class LINSlave:

# 	def __init__(self, id):
#         self.id = id
	
# 	def send_data(data):
# 		# send 2 bytes: 1 byte data + 1 byte CRC
# 		# send Checksum (1 byte)
# 		# --------------------------------------
		
# 		# calculates CRC and Checksum
# 		# send data
# 		# send CRC
# 		# send Checksum(data + CRC)
		
# 		# return 1 if right,
# 		#		 0 if ignore data,
# 		#		-1 if parity error
		
# 		return 1
		
# 	def wait_request(): # --->> use rising edge interrupt to get after break (maybe one 1 still in message)
# 		# wait something in bus
# 		# get only data
# 		# check id
# 			# if not: ignore_data
# 		# check parity
# 			# if wrong: send something wrong so master will try again
		
# 		return send_data(0x55)
		
# 	def ignore_data(size):
# 		return 0
		
# 	def slave_main():
# 		# create class
# 		# while True: wait_request()
# 		# try/except KeyboardInterrupt
# 		return

# 	def CRC(data):
# 	    data = int.from_bytes(data, "big")
# 	    for i in range(8):
# 	        data = (data << 1) & 0x1FF
# 	        data ^= 0xD5 * int(data / 256)
	        
# 	    return data

# 	def checksum(data):
# 		soma = sum(data)
# 	    res = (soma & 0xFF) + ((soma >> 8) & 0xFF)
# 	    return (~(res) & 0xFF)