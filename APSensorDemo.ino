#include <WiFiManager.h>

#include <DoubleResetDetector.h>

#include "ESP8266WiFi.h"

#include <Ticker.h>  //Ticker Library
 
Ticker blinker;
// Number of seconds after reset during which a 
// subseqent reset will be considered a double reset.
#define DRD_TIMEOUT 10

// RTC Memory Address for the DoubleResetDetector to use
#define DRD_ADDRESS 0

#define AP_NAME "DontUseAccessPoint"

#define DEFAULT_PASSWORD "nopassword"

// Set web server port number to 80
WiFiServer server(80);

// Auxiliar variables to store the current output state
String outputState = "far";
// Variable to store the HTTP request
String header;

int inputPin = 13; // choose input pin (for Infrared sensor) 
int val = 0; // variable for reading the pin status 

DoubleResetDetector drd(DRD_TIMEOUT, DRD_ADDRESS);

//=======================================================================
void changeState()
{
  digitalWrite(LED_BUILTIN, !(digitalRead(LED_BUILTIN)));  //Invert Current State of LED  
}

void stopBlinking(){
  blinker.detach();
  digitalWrite(LED_BUILTIN, HIGH); 
}

void setup() 
{
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  pinMode(inputPin, INPUT);
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //exit after config instead of connecting
  wifiManager.setBreakAfterConfig(true);

 

  if (drd.detectDoubleReset()) {
    Serial.println("Double Reset Detected");
     wifiManager.resetSettings();
    Serial.println("failed to connect, we should reset as see if it connects");
    delay(3000);
    ESP.reset();
    delay(5000);
   
  } else {
    Serial.println("No Double Reset Detected");
  }
 
  //Initialize Ticker every 0.5s
  blinker.attach(0.5, changeState); //Use <strong>attach_ms</strong> if you need time in ms
 
   if (!wifiManager.autoConnect(AP_NAME, DEFAULT_PASSWORD)) {
    Serial.println("failed to connect, we should reset as see if it connects");
    delay(3000);
    ESP.reset();
    delay(5000);
  }

   //if you get here you have connected to the WiFi
  Serial.println("Successfully wifi connect with local network");


  Serial.println("local ip");
  Serial.println(WiFi.localIP());

  server.begin();
  stopBlinking();
  
}
void loop() 
{
   drd.loop();
   hostStatusWebpage();
}
void hostStatusWebpage()
{
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off

            val = digitalRead(inputPin); // read input value 
           if (val == HIGH)
           {
            Serial.println("Output on");
              outputState = "far";
             
            } else {
              Serial.println("Output on");
              outputState = "near";
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>ESP8266 Web Server</h1>");
            
            // Display current state, and ON/OFF buttons for the defined GPIO  
            client.println("<p>Output - State " + outputState + "</p>");
            // If the outputState is off, it displays the ON button       
                      
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

