# NodeMcu-BME280-Weather-Station
This project was created from a combination of other projects and sources.
This core of the project came from R-B on Embedded Lab's site:
   http://embedded-lab.com/blog/making-simple-weather-web-server-using-esp8266-bme280/
I like the bones of it but wanted to expand it. I added a NTP call to get the current
time and date, which I added to the web page it produces. I also wanted to tinker with
more functions in the BME's API, so I added an altitude line. I also wanted to see the
rise of fall of the barometric pressure.

The current output of the web display looks like:

ESP8266 Weather Web Server

2017-05-29 19:52:01

Temperature = 84.1 °F

Humidity = 35.3 %

Approx. Dew Point = 60.8 °F

Pressure = 992.2 hPa ( 29.30 Inch) rising 0.22 hPa

Altitude = 680.9 feet

Wiring of the BME to the NodeMCU is caled out in the original web page.

Enjoy,
-Kirk