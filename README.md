# NodeMcu-BME280-Weather-Station
This project was created from a combination of other projects and sources.
This core of the project came from R-B on Embedded Lab's site:
   http://embedded-lab.com/blog/making-simple-weather-web-server-using-esp8266-bme280/
I like the bones of it but wanted to expand it. I added a NTP call to get the current
time and date, which I added to the web page it produces. I also wanted to tinker with
more functions in the BME's API, so I added an altitude line. I also wanted to see the
rise of fall of the barometric pressure.

I tried it on a Wemos D1 mini (clone) and found that it works without any modification,
but you may find it will need different port drivers. I also learned that not all USB
cables work for programming; I have one that works great for power but the driver did
not see the board when I plugged it in. The wiring for the BME280 is identical.

Here's a good place to find drivers (there are others, Google Arduino NodeMCU drivers):
  http://kig.re/2014/12/31/how-to-use-arduino-nano-mini-pro-with-CH340G-on-mac-osx-yosemite.html

If you want to see the differences in these, I found this insightful: 
   https://frightanic.com/iot/comparison-of-esp8266-nodemcu-development-boards/

I cloned this same file on my Mac; so I have one for NodeMCU and another for Wemos.
This gave me the opportunity to customize the name so I can tell these apart on the LAN.

The current output of the web display looks like:

     ESP8266 Weather Web Server

     2017-05-29 19:52:01

     Temperature = 84.1 °F

     Humidity = 35.3 %

     Approx. Dew Point = 60.8 °F

     Pressure = 992.2 hPa ( 29.30 Inch) rising 0.22 hPa

     Altitude = 680.9 feet

Wiring of the BME to the NodeMCU is shown on the original web page.

Enjoy,
-Kirk
