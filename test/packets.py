import sys
import json
import socket
import threading
import select
import time

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
	
	pkt_search = [0x00, 0x00, 0x01, 0x06]
	
	pkt_dict = {}
	pkt_dict["search"] = pkt_search
	
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
				client.send(buf)

			else:
				print "unknown key"

	client.disconnect()
	print "done!"
		
if __name__ == "__main__":
	main()

