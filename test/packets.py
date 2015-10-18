import sys
import json
import socket
import threading
import select
import time
import base64

class TcpClient(object):
	def __init__(self):
		self._client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self._alive = False

	def connect(self, hostname, port):
		self._client.connect((hostname, port))
		self._thread_listener = threading.Thread(target = self._listener)
		self._thread_listener.setDaemon(True)
		self._alive = True
		self._thread_listener.start()

	def disconnect(self):
		self._alive = False;
		self._client.close()

	def _listener(self):
		while self._alive:
			input,output,err = select.select([self._client],[],[]) 
			for rd in input: 
				if self._alive is True:
					data = rd.recv(1024)
					print "rx:", data

	def send(self, data):
		self._client.send(data)

def main():
	client = TcpClient()
	client.connect("localhost", 1234)
	
	pkt_dict = {
	"search": [0x00, 0x00, 0x01, 0x06]
	}

	print "Available keys:"
	print pkt_dict.keys()
	print "Type a key or 'quit' to quit:"

	while True:
		inputText = raw_input()
		if inputText == "quit":
			break
		else:
			if inputText in pkt_dict.keys():
				buf = ""
				buf += chr(0x55)
				for b in pkt_dict[inputText]:
					buf += chr(b)
				buf += chr(0x55)
				data_to_send = json.dumps({ "pkt": {
					"type": "data",
					"format": "serial",
					"data": base64.b64encode(buf)

				}})
				print data_to_send
				#for i in range(1,100):
				client.send(data_to_send)

			else:
				print "unknown key"

	client.disconnect()
	print "done!"
		
if __name__ == "__main__":
	main()

