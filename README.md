# NMEA-GPS
A basic NMEA GPS parser designed and tested with the u-Blox MAX-8 series of GPS receiver. 

https://www.beyondlogic.org/ansi-c-basic-lightweight-nmea-parser-for-gps/

MAX-8 / MAX-M8 FW3 Hardware Integration Manual:
https://www.u-blox.com/en/docs/UBX-13003221

Works on a PC with ubuntu or a Raspberry PI. 

On the Raspberry PI:
* enable the UART by adding enable_uart=1 to your /boot/config.txt 
* remove console=tty* from the /boot/cmdline.txt
* see https://www.raspberrypi.org/documentation/configuration/uart.md for your device name (ttyAMA0 or ttyS0)

# Alternatives
If you are looking for a more compatible solution tested with an extensive range of GPS, consider gpsd - a GPS service daemon.
https://gpsd.gitlab.io/gpsd/index.html

It can be used with NTPd to set the time on embedded systems without a Real Time Clock (RTC) like the Raspberry PI. 


