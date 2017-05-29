/*********
  Project: BME Weather Web server using NodeMCU
  Implements Adafruit's sensor libraries.
  Complete project is at: http://embedded-lab.com/blog/making-a-simple-weather-web-server-using-esp8266-and-bme280/
  
  Modified code from Rui Santos' Temperature Weather Server posted on http://randomnerdtutorials.com  
*********/

#include <Wire.h>
#include <ESP8266WiFi.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <WiFiUdp.h>
#include <stdio.h>
#include <TimeLib.h>

Adafruit_BME280 bme; // I2C

// Replace with your network details
const char* ssid = "Gitli";
const char* password = "Barabajagal1950";
float h, t, p, pin, dp, a, af;
char temperatureFString[6];
char dpString[6];
char humidityString[6];
char pressureString[7];
char pressureInchString[6];
char altitudeMetersString[6];
char altitudeFeetString[6];
char timeDateAndTimeString[40];
int  pressureArray[100];
int  pressureArrayMax = 100;
int  pressureSamples = 0;
int  pressureArrayIndex = 0;
char pressureTrend[10];
float pressureAverage = 0.0;
float pressureTotal = 0.0;
float pressureDifference = 0.0;

// Web Server on port 80
WiFiServer server(80);

// local port to listen for UDP packets
unsigned int localPort = 2390;

/* Don't hardwire the IP address or we won't get the benefits of the pool.
 *  Lookup the IP address for the host name instead */
//IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
IPAddress timeServerIP; // time.nist.gov NTP server address
const char* ntpServerName = "time.nist.gov";

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

// only runs once on boot
void setup() {
  // Initializing serial port for debugging purposes
  Serial.begin(115200);
  delay(10);
  Wire.begin(D3, D4);
  Wire.setClock(100000);
  // Connecting to WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Starting the web server
  server.begin();
  Serial.println("Web server running. Waiting for the ESP IP...");
  delay(10000);
  
  // Printing the ESP IP address
  Serial.println(WiFi.localIP());
  Serial.println(F("BME280 test"));

  if (!bme.begin()) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  // start up the UDP port for SNTP
  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());
}

void getTimeFromNist() {
  time_t     now;
  TimeElements tm;
  int localTime;
  
  //get a random server from the pool
  WiFi.hostByName(ntpServerName, timeServerIP); 

  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);
  
  int cb = udp.parsePacket();
  if (!cb) {
    Serial.println("no packet yet");
    strncpy(timeDateAndTimeString, "Queried NIST NTP server, no packet yet", 40);
  }
  else {
    Serial.print("packet received, length=");
    Serial.println(cb);
    // We've received a packet, read the data from it
    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    Serial.print("Seconds since Jan 1 1900 = " );
    Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // print Unix time:
    Serial.println(epoch);

     // Set the system time to the given time t
    now = secsSince1900 - seventyYears;
    setTime(now);
    makeTime(tm);
    localTime = hour(now) - 5; //Central time
    if (localTime < 0) {
      localTime +=24;
    }
    sprintf(timeDateAndTimeString, "%04d-%02d-%02d %02d:%02d:%02d", year(now), month(now), day(now), localTime, minute(now), second(now));
    

    // print the hour, minute and second:
    Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
    Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
    Serial.print(':');
    if ( ((epoch % 3600) / 60) < 10 ) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
    Serial.print(':');
    if ( (epoch % 60) < 10 ) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.println(epoch % 60); // print the second

    // build our timestamp string
    int hour   = ((epoch  % 86400L) / 3600) -5; // for Central time
    int minute = (epoch % 3600) / 60;
    int second = epoch % 60;
    //sprintf(timeDateAndTimeString,"%02d:%02d:%02d", hour, minute, second);

  }

}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}


void getWeather() {
    
    // get data from BME
    h = bme.readHumidity();
    t = bme.readTemperature();
    t = t*1.8+32.0;
    dp = t-0.36*(100.0-h);
    a = bme.readAltitude(1017);
    af = a * 3.28;
    
    p = bme.readPressure()/100.0F;
    pin = 0.02953*p;
    dtostrf(t, 5, 1, temperatureFString);
    dtostrf(h, 5, 1, humidityString);
    dtostrf(p, 6, 1, pressureString);
    dtostrf(pin, 5, 2, pressureInchString);
    dtostrf(dp, 5, 1, dpString);
    dtostrf(a, 5, 1, altitudeMetersString);
    dtostrf(af, 5, 1, altitudeFeetString);
    delay(100);

    // add a pressure sample to the array and check if we've filled the arrray. The pressureArrayIndex was initialized to 0
    pressureArray[pressureArrayIndex++] = p;
    if (pressureArrayIndex >= 100) {  //reset the index if we reached the end
      pressureArrayIndex = 0;
    }

    pressureTotal = 0.0;
    pressureSamples = 0;
    for (int i=0; i < pressureArrayMax; i++) {  //accumulate the total
      if ((pressureArray[i] > 500.0) && (pressureArray[i] < 2000.0)) {  // ... if it's sensible
        pressureTotal += pressureArray[i];
        pressureSamples++;
      }
    }
    // get our average
    pressureAverage = pressureTotal/pressureSamples;
    Serial.println(pressureTotal);
    Serial.println(pressureSamples);
    Serial.println(pressureAverage);
    Serial.println(p);
    
    if (p < pressureAverage) {
      sprintf(pressureTrend, "%s ", "falling");
    } else if (p == pressureAverage) {
      sprintf(pressureTrend, "%s ", "level");
    } else  {
      sprintf(pressureTrend, "%s ", "rising");
    }
    pressureDifference = p - pressureAverage;
 
}

// runs over and over again
void loop() {
  // Listenning for new clients
  //Serial.println("Listening for new clients");
  WiFiClient client = server.available();
  
  if (client) {
    Serial.println("New client");
    // bolean to locate when the http request ends
    boolean blank_line = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        
        if (c == '\n' && blank_line) {
            // read our BME
            getWeather();
            // get our current time
            getTimeFromNist();
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();
            // your actual web page that displays temperature
            client.println("<!DOCTYPE HTML>");
            client.println("<html>");
            client.println("<head><META HTTP-EQUIV=\"refresh\" CONTENT=\"15\"></head>");
            client.println("<body><h1>ESP8266 Weather Web Server</h1>");
            client.println("<table border=\"2\" width=\"456\" cellpadding=\"10\"><tbody><tr><td>");
            client.println("<h3>");
            client.println(timeDateAndTimeString);
            client.println("</h3>");
            client.println("<h3>Temperature = ");
            client.println(temperatureFString);
            client.println("&deg;F</h3>");
            client.println("<h3>Humidity = ");
            client.println(humidityString);
            client.println("%</h3>");
            client.println("<h3>Approx. Dew Point = ");
            client.println(dpString);
            client.println("&deg;F</h3>");
            client.println("<h3>Pressure = ");
            client.println(pressureString);
            client.println("hPa (");
            client.println(pressureInchString);
            client.println("Inch) ");
            client.println(pressureTrend);
            client.println(pressureDifference);
            client.println(" hPa");
            client.println("</h3>");
            client.println("<h3>Altitude = ");
            client.println(altitudeFeetString);
            client.println(" feet</h3>");
            client.println("</td></tr></tbody></table></body></html>");   
            break;
        }
        if (c == '\n') {
          // when starts reading a new line
          blank_line = true;
        }
        else if (c != '\r') {
          // when finds a character on the current line
          blank_line = false;
        }
      }
    }  
    // closing the client connection
    delay(1);
    client.stop();
    Serial.println("Client disconnected.");
  }
} 
