# Author: Miriam Ortega and Marco Guarachi 
# Year: 2024
# Description: Server file for "LSE Practica 1 (UPM)"

import sys
import select
import socket
from datetime import datetime
from datetime import date

  
# Create the socket for the server
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# Enables the socket
s.bind(('',8080))
# Make the socket start listening
s.listen(5)

input = [s, sys.stdin]
running = 1
while running:
  inputready, outputready, exceptready = select.select(input, [], [])
  for s_ in inputready:
    if s_ == s:
      #handle the server socket
      # Wait for client
      client, address = s.accept()
      input.append(client)
      
    elif s_ == sys.stdin:
      #handle standard input
      junk = sys.stdin.readline()
      running = 0
      
    else:
      #handle all other sockets
      # Receive request from client
      rcv_data = s_.recv(1024)
      if rcv_data:
        # Send requested data
        if rcv_data == "date":
  	  # Date
	  today = date.today()
	  answer = today.strftime("%B %d, %Y") 

        elif rcv_data == "time":
	  # Datetime
	  today = datetime.now()
	  answer = today.strftime('%H:%M:%S')
	  
	s_.send(answer)
      else:
        s_.close()
        input.remove(s_)

# Close socket
s.close()

