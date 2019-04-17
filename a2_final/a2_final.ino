
// https://io.adafruit.com/willayang/dashboards/a2
// this sketch grabs humidity data from openweatherapi.org and reads humidity locally.
// it draws a line chart on Adafruit IO to show the trend and difference of both
// local humidity reading and the city we are interested in.
// This sketch can also control the LED on the board by clicking on the switch

/************************** Configuration ***********************************/

#include "config.h"

/************************ Example Starts Here *******************************/
#include <Adafruit_Sensor.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "Adafruit_Si7021.h"

const char* ssid = "University of Washington";
const char* pass = "";
const String key = "b226ebd2ec194b655cb23e4e46a41e10"; // the key to get IP address and locations
const String weatherKey = "7bc7661254d77ecdaf02640767252c88"; // the key to get weather info

String city = "Chicago";
float local_reading;
float api_reading;

// instantiate the sensor 
Adafruit_Si7021 sensor = Adafruit_Si7021();

// set up the 'local' and 'cityread' feeds
AdafruitIO_Feed *local = io.feed("local_hum");
AdafruitIO_Feed *cityread = io.feed("city_hum");

// digital pin 5 to control LED
#define LED_PIN 5

// set up the 'digital' feed for LED
AdafruitIO_Feed *digital = io.feed("digital");

void setup() {

  pinMode(LED_PIN, OUTPUT);
  
  // start the serial connection
  Serial.begin(115200);

  // wait for serial monitor to open
  while(! Serial);
  
  delay(10); // delay 10 miliseconds 
  // print wifi name
  Serial.print("Connecting to "); 
  Serial.println(ssid); 
  WiFi.mode(WIFI_STA); // start connecting to wifi
  WiFi.begin(ssid, pass); // initialize the wifi name and pw
  while (WiFi.status() != WL_CONNECTED) {   // keep connecting until the wifi is connected
    delay(500); // wait for 0.5 sec
    Serial.print(".");  // print dots
  }
  // inform the wifi is connected and print the IP address
  Serial.println(); Serial.println("WiFi connected"); Serial.println();
  Serial.print("Your ESP has been assigned the internal IP address ");
  Serial.println(WiFi.localIP());
  
  Serial.print("Connecting to Adafruit IO");
  io.connect();
 
  // set up a message handler for the 'digital' feed.
  // the handleMessage function (defined below)
  // will be called whenever a message is
  // received from adafruit io.
  digital->onMessage(handleMessage);

  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }    
  
  // we are connected
  Serial.println();
  Serial.println(io.statusText());
  digital->get();
  
  sensor = Adafruit_Si7021(); // initialize the humidity sensor
  sensor.begin(); // turn on the sensor
  Serial.println("Si7021 test!");
  if (!sensor.begin()) {
    Serial.println("Did not find Si7021 sensor!");
    while (true);
  }  

}

void loop() {

  local_reading = sensor.readHumidity();

  delay(1000);

  // calls the function to grab the humidity data from Chicago
  getHumidity(city);
  Serial.print("Local:    "); Serial.println(local_reading);
  Serial.print("City:    "); Serial.println(api_reading);

  io.run();
  // save the reading from local to Adafruit IO
  local->save(local_reading);
  // save the reading from MPL to Adafruit IO
  cityread->save(api_reading);
    
}

void getHumidity(String city) { // this method gets the wind speed in the given city from the weather API 
  HTTPClient theClient; // initializes browser
  String url = "http://api.openweathermap.org/data/2.5/weather?q=" + city 
  + "&units=imperial&appid=" + weatherKey; 
  theClient.begin(url); // connect to the api
  int httpCode = theClient.GET(); // gets http response code
  if (httpCode > 0) { 

    if (httpCode == HTTP_CODE_OK) { // sees if the code is 200 so that it works
      String payload = theClient.getString(); // gets the payload from the website (json format String)
      DynamicJsonBuffer jsonBuffer; // jsonbuffer will parse the payload
      JsonObject& root = jsonBuffer.parseObject(payload); // jsonObject contains the json data
      if (!root.success()) {
        Serial.println("parseObject() failed in getHumidity().");
        return;
      }
      api_reading = root["main"]["humidity"]; // updates the api reading
    }
  } else {
    Serial.printf("Something went wrong with connecting to the endpoint in getHumidity().");
  }
}

// this function is called whenever an 'digital' feed message
// is received from Adafruit IO. it was attached to
// the 'digital' feed in the setup() function above.
void handleMessage(AdafruitIO_Data *data) {

  Serial.print("received <- ");

  if(data->toPinLevel() == HIGH)
    Serial.println("HIGH");
  else
    Serial.println("LOW");

  digitalWrite(LED_PIN, data->toPinLevel());
  
}
