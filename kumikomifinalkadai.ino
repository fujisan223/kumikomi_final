#include <WiFi.h>
#include <Servo.h>
#include <FastLED.h>
 
#define numberOfLEDs 60 //Quantity of RGB LEDs
#define controlPin 23

const char* ssid     = "your ssid";
const char* password = "your password";

static const int servoPin1 = 2;
static const int servoPin2 = 4;
bool aircon = false; //state of air conditioner(1=ON,0=OFF)
int wifiLED = 5; //LED to confirm wifi conection
int ans , temp , tv ;

Servo servo1;
Servo servo2;
CRGB leds[numberOfLEDs];
WiFiServer server(80);

void setup()
{
    Serial.begin(115200);
    pinMode(wifiLED, OUTPUT);
    servo1.attach(servoPin1);
    servo2.attach(servoPin2);
    FastLED.addLeds<WS2812B, controlPin, GRB>(leds, numberOfLEDs);

    delay(10);

    // We start by connecting to a WiFi network

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    
    server.begin();
    digitalWrite(wifiLED, HIGH); //after conected wifi, wifiLED lights
    servo1.write(0); //servos set to origin
    servo2.write(90);

}

int value = 0;

void measuretemp() {
     ans = analogRead(33) ;             // read sensor value
     tv  = map(ans,0,4096,0,3600) ;     // convert sensor value to voltage
     temp = map(tv,174,1205,-40,125) ;  // convert voltage to temp
}

void RLED(){ //RGB LEDs grow red
  for (int thisLED = 0; thisLED < numberOfLEDs; thisLED++) {
    leds[thisLED].r = 128;
    leds[thisLED].g = 0;
    leds[thisLED].b = 0;
    FastLED.show();
  }
}

void GamingLED(){ //RGB LEDs grow rainbow
  static uint8_t hue;
  for(int thisLED = 0; thisLED < numberOfLEDs; thisLED++) {   

    //set an led value
    leds[thisLED] = CHSV(hue++,255,255);
    FastLED.show();
    delay(20);
  }
}

void loop(){
 delay(1000);
 
 WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("Click <a href=\"/H\">here</a> to turn on the air conditioner.<br>");
            client.print("Click <a href=\"/L\">here</a> to turn off the air conditioner.<br>");
            client.print(" <br>");
            client.print((String)"ariconditioner <br>" + aircon);

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) { //if client get H
          servo1.write(90);
          delay(1000);
          servo1.write(0);
          delay(500);
          aircon = true; //Turn on the air conditioner
        }
        
        else if (currentLine.endsWith("GET /L")) { //if client get L
          servo2.write(0);
          delay(1000);
          servo2.write(90);
          delay(500);
          aircon = false; //Turn off the air conditioner
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }

  measuretemp();
  Serial.println((String)"temp = " + temp);
  Serial.println((String)"aircon = " + aircon); //temp and state of air conditioner send to PC

  if (temp >= 16 && temp <= 17) { //if sensor shows suitable temperature
          GamingLED();
  }else if (temp > 17 && aircon == true) { //if sensor shows too hot temperature and air conditioner is turned on
          servo2.write(0);
          delay(1000);
          servo2.write(90);
          delay(500);
          aircon = false;
      RLED();
  }else if (temp > 17 && aircon == false) { //if sensor shows too hot temperature and air conditioner is turned off
      RLED();
  }else{ //if sensor shows the other state
   for (int thisLED = 0; thisLED < numberOfLEDs; thisLED++) {
     leds[thisLED].r = 0;
     leds[thisLED].g = 0;
     leds[thisLED].b = 0;
    FastLED.show(); //LEDs turn off
    }
  }
}
