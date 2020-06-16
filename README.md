# AccessPointExample
Creating an Access Point on an ESP32 that will allow the user to select an available WIFI network they want the ESP to connect to.

## How it works
1. The device starts up and initialises a Access Point.
2. The device waits for 30 seconds for a connection from a client
3. If a connection is received the count down stops, and a network scan is performed (to see what WIFI networks are available)
4. Default Access Point URL is 172.16.1.1
5. Select the Network SSID you wish to connect to from the drop down list
6. Enter the password and press submit
7. The ESP32 will now store the SSID and password in EEPROM and try to connect to the network
8. Each time the ESP starts up, if no connection is received within 30 seconds, it will connect using the details stored in EEPROM
9. While connected to the network, the ESP32 will check to see that its network connection is okay, if it loses connection, it will try to connect for 30 seconds, if the connection fails again it will wait for 30 seconds before trying again.
