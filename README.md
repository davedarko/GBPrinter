GBPrinter
=========
This library for Arduino is based on the works and efforts of the guy(s) from furrtek.org and Miles Burton from milesburton.com 
To use it, simply copy the folder in to the libraries folder in your sketchbook. Let me know if something is not working, I will try to help.

See the references here: 
http://milesburton.com/Gameboy_Printer_with_Arduino
http://furrtek.free.fr/index.php?p=crea&a=gbpcable&i=2

# What it does
- it should work with furrteks picture send but I am not able to tell because of my linux / mac setup
- hardcoded blocks of 640bytes are printable - to see in the hadCon() function

# Where does one get those blocks?
To get those you can go to http://davedarko.de/gameboy.php and try out my picture preparator - upload or draw something, print and wait for that buggy Java Application - refresh the site, klick on your link and add "&format=bin" to it, you should see the generated HEX stuff. This should generate 8 printable rows, which I could not test cause my chinese arduino atmega328 boards only have 6500-ish bytes - I guess that is why they are so cheap. I tested 4 and all crashed. Try connecting RX and TX before you start the arduino!

# ToDo
- A character mode, so you can send text and it will generate the print
- a "cleaned up" version of the Picture transmitting
- a processing sketch as an interface via bluetooth (as another github project, maybe)
- an attiny45/85 Sketch / hex files for minification

![alt text](https://raw.github.com/davedarko/GBPrinter/master/res/img/wire.png "Wires")

[![ScreenShot](https://raw.github.com/davedarko/GBPrinter/master/res/img/yt.png)](http://youtu.be/Zd2LHVKtdGM)