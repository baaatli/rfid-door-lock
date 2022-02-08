# rfid-door-lock
RFID door lock using Arduino Pro Mini and MFRC522 module. 

All I learned:
The interface with MFRC522 is very tricky, hope this one works with you. 
Also, there are plenty of Chinese counterfeits of the module that don't work. I got mine from: https://robu.in/product/mifare-rfid-readerwriter-13-56mhz-rc522-spi-s50-fudan-card-and-keychain/
This one works on 13.56 MHz but RFID has a couple of other frequencies like 125 kHz as well which will not work with this module, make sure you have tag of 13.56 MHz only. 
I have implemented Arduino power saving which increases battery life to a great extent. The sleep power consumption is close to 4 mA and in active mode it is around 15 mA at 3.3V.


Here's a link to article: https://medium.com/@vardhann/arduino-rfid-door-lock-but-better-a6d2329b2dcc
