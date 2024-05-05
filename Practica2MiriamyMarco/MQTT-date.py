import paho.mqtt.client as mqtt # Import the MQTT library
import time # The time library is useful for delays
from datetime import datetime
from datetime import date

#from itertools import cycle
#myIterator = cycle(['HOLA','ADIOS'])
client_topic = 'MQTT_NETWORK/DATE'

# Our "on message" event
def messageFunction (client, userdata, message):
	topic = str(message.topic)
	message = str(message.payload.decode("utf-8"))
	print(message)
 
 
ourClient = mqtt.Client("TIME") # Create a MQTT client object
ourClient.connect("10.8.42.100", 1885) # Connect to the test MQTT broker
#ourClient.subscribe("MQTT_NETWORK/RASPBERRY") # Subscribe to the topic AC_unit
#ourClient.on_message = messageFunction # Attach the messageFunction to subscription
ourClient.loop_start() # Start the MQTT client
 
 
# Main program loop
while(1):
	today = date.today()
	answer = today.strftime("%B %d, %Y")
	ourClient.publish(client_topic, client_topic +'{\"DATE\": \"' + answer + '\"}') # Publish message to MQTT broker
	time.sleep(3) # Sleep for a second

