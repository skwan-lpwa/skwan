# What is SkWAN?
SkWAN is a star-network protocol stack optimized for LPWAN (Low Power Wide Area Network).

Devices are divided mainly into stations and end devices when in operation. A station is equivalent to an access point, and serves as a gateway to transfer data from an end device to the Internet side. An end device periodically transmits several to several tens of bytes of sensing data to a station.

# Futures

SkWAN uses a time-division time slot system for communication control in the MAC layer. Therefore, even at low data rates, it can house many end devices, allowing stable communication. Since basically it does not cause communication collisions, end devices are free from excess power consumption associated with carrier sensing repetition or retransmission, offering superior power saving properties.
 
 A station can incorporate four wireless communication modules and one auxiliary module. This allows one station to house about 6400 end devices in the maximum configuration.

An end device can operate with a low-cost 8K-RAM 128K-Flash MCU or other similar MCUs. It operates in power save mode during most of its operating time except for the time slot for the local terminal; therefore, it is possible to easily achieve long-term battery-driven operation without complicated power-saving control by an application.

