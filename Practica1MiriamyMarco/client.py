# Author: Miriam Ortega and Marco Guarachi 
# Year: 2024
# Description: Client file for "LSE Practica 1 (UPM)"

import socket


# Ask the user server ip, port, and desired data 
ip = raw_input("Enter server ip: ")
port = int(raw_input("Enter server port: "))
data = raw_input("Select date or current time [date/time]: ")


# Create the client socket
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# Connect to server
s.connect((ip,port))
# Send date or time to server
s.send(data)
# Receive date or time from server
rcv_data = s.recv(1024)
# Close socket
s.close()


# Print date or time from server
print("Current " + data + " is: " + rcv_data)

