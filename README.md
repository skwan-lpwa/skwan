# What is SkWAN?
SkWAN is a star-network protocol stack optimized for LPWAN (Low Power Wide Area Network).

Devices are divided mainly into stations and end devices when in operation. A station is equivalent to an access point, and serves as a gateway to transfer data from an end device to the Internet side. An end device periodically transmits several to several tens of bytes of sensing data to a station.

## Features

SkWAN uses a time-division time slot system for communication control in the MAC layer. Therefore, even at low data rates, it can house many end devices, allowing stable communication. Since basically it does not cause communication collisions, end devices are free from excess power consumption associated with carrier sensing repetition or retransmission, offering superior power saving properties.
 
 ![tdma](https://user-images.githubusercontent.com/11895675/71439027-4c026300-273b-11ea-9d3d-f3019f81e3af.jpg)

 A station can incorporate four wireless communication modules and one auxiliary module. This allows one station to house about 6400 end devices in the maximum configuration.

An end device can operate with a low-cost 8K-RAM 128K-Flash MCU or other similar MCUs. It operates in power save mode during most of its operating time except for the time slot for the local terminal; therefore, it is possible to easily achieve long-term battery-driven operation without complicated power-saving control by an application.

## Usage: Station

First, specify an operating frequency.

Select channel 0x1A (26):
 ```
 SKSREG␣S08␣1A
 ```
Next, set the station ID.

Set the station ID to 0x12345678:
 ```
 SKSREG␣S05␣12345678
 ```
A station ID is a unique number to identify every station. You need to take care not to give the same ID to different stations.

Next, set the pre-shared key for connection.

Set the PSK:
 ```
 SKSETPSK␣11111111222222223333333344444444
 ```
Finally, change the operation type to station to start beacon transmission.

Set the operation type to 0:
 ```
 SKSREG␣S02␣0
 ```
If the station ID is the initial value (0xFFFFFFFF), setting S02 to 0 does not start beacon transmission.

## Usage: End Device

Also for the end device, first, specify an operating frequency.

Select channel 0x1A (26):

 ```
SKSREG␣S08␣1A
 ```
 
Next, set the pre-shared key for connection.

Set the PSK:

 ```
SKSETPSK␣11111111222222223333333344444444
 ```
 
Start operation as an end device:

 ```
SKSREG␣S02␣1
 ```
 
After you wait for a while in this state, a beacon reception event occurs.

 ```
ERXBCN␣00␣01␣12345678␣50
 ```
 
If more than one station is in operation, a beacon reception event occurs the same number of times as the number of stations. Select the station ID of the connection destination among these.

Set the station ID to 0x12345678:
 
 ```
SKSREG␣S05␣12345678
 ```
 
After the station ID is set, time slot synchronization starts the next time a beacon is received from the station. After the start of synchronization, the time slot for the local terminal is used to automatically perform connection.

Upon completion of the connection, an EJOIN event occurs with Status = 0 (SUCCESS). This completes the connection, and now it is possible to transmit and receive data.

## Protocol specification
For more details about the SkWAN protocol, please refer to the wiki:

https://github.com/skwan-lpwa/skwan/wiki

## Sample Application

http://www.skyley.com/wiki/index.php?LoRa_Tutorial (in Japanese)


