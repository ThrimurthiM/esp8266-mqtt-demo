/*
- * COPYRIGHT (C) 2021 Electosh Solution Pvt. Ltd - All Rights Reserved
 *
 *      %name      : MQTT_Demo %
 *      Author     : Thrimurthi M
 *      Modified By: Thrimurthi M
 */

#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "DHT.h"        // including the library of DHT11 temperature and humidity sensor


/*********************** DHT11 Sensor *************************************/
#define DHTTYPE DHT11   // DHT 11
#define dht_dpin 0 // D3 
#define led_pin  2 // D4
DHT dht(dht_dpin, DHTTYPE);

/************************* WiFi Access Point *********************************/


#define WLAN_SSID       "Thrimurthi" /* User Need to update there SSID here */
#define WLAN_PASS       "thrimurthi" /* User Need to update there Password here */

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "35.158.189.129"  /* broker.mqttdashboard.com */
#define AIO_SERVERPORT  1883                   // use 8883 for SSL

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiClientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT);

/****************************** Feeds ***************************************/

// Setup a feed called 'pub_temperature' and pub_humidity for publishing.
Adafruit_MQTT_Publish pub_temperature = Adafruit_MQTT_Publish(&mqtt, "VVCE_IOT_TEMPERATURE");
Adafruit_MQTT_Publish pub_humidity = Adafruit_MQTT_Publish(&mqtt, "VVCE_IOT_HUMIDITY");

// Setup a feed called 'onoff' for subscribing to change the LED status.
Adafruit_MQTT_Subscribe sub_ledonoffbutton = Adafruit_MQTT_Subscribe(&mqtt, "VVCE_IOT_LED_CONTROL");

/*************************** Sketch Code ************************************/
/* Static Function */
static void MQTT_connect();
static char* GetHumidity();
static char* GetTemperature();

/* Static Variable */
static String stringOne;

void setup()
{
    pinMode(led_pin, OUTPUT);
    digitalWrite(led_pin, LOW);
    Serial.begin(115200);
    delay(10);

    Serial.println(F("Adafruit MQTT demo"));

    // Connect to WiFi access point.
    Serial.println(); Serial.println();
    dht.begin();
    Serial.print("Connecting to ");
    Serial.println(WLAN_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WLAN_SSID, WLAN_PASS);
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);
    }
    Serial.println();

    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    // Setup MQTT subscription for onoff feed.
    mqtt.subscribe(&sub_ledonoffbutton);
}

void loop()
{
    // Ensure the connection to the MQTT server is alive (this will make the first
    // connection and automatically reconnect when disconnected).  See the MQTT_connect
    // function definition further below.
    MQTT_connect();

    // this is our 'wait for incoming subscription packets' busy subloop
    // try to spend your time here

    Adafruit_MQTT_Subscribe *subscription;
    while ((subscription = mqtt.readSubscription(5000)))
    {
        if (subscription == &sub_ledonoffbutton)
        {
            Serial.print(F("Got: "));
            Serial.println((char *)sub_ledonoffbutton.lastread);
            stringOne = String((char *)sub_ledonoffbutton.lastread);
            if (stringOne == "ON")
            {
                Serial.println("\n ON \n");
                digitalWrite(led_pin, HIGH);
            }
            else if (stringOne == "ON\n")
            {
                Serial.println("\n ON \n");
                digitalWrite(led_pin, HIGH);
            }
            else if (stringOne == "ON\r\n")
            {
                Serial.println("\n ON \n");
                digitalWrite(led_pin, HIGH);
            }
            else if (stringOne == "ON\n\n")
            {
                Serial.println("\n ON \n");
                digitalWrite(led_pin, HIGH);
            }
            else if (stringOne == "ON\r\n\r\n")
            {
                Serial.println("\n ON \n");
                digitalWrite(led_pin, HIGH);
            }
            else
            {
                Serial.println("\n OFF \n");
                digitalWrite(led_pin, LOW);
            }
        }
    }

    char* humidity = GetHumidity();
    // Now we can publish stuff
    Serial.print(F("\n Sending "));
    Serial.print("\n");
    Serial.print(humidity);
    if (!pub_humidity.publish(humidity))
    {
        Serial.println(F("Failed"));
    }
    else
    {
        char* temperature = GetTemperature();
        Serial.print("\n");
        Serial.print(temperature);
        if (!pub_temperature.publish(temperature))
        {
            Serial.println(F("Failed"));
        }
        else
        {
            // OK
        }
    }

    // ping the server to keep the mqtt connection alive
    // NOT required if you are publishing once every KEEPALIVE seconds
    /*
    if(! mqtt.ping()) {
      mqtt.disconnect();
    }
    */
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
static void MQTT_connect()
{
    int8_t ret;

    // Stop if already connected.
    if (mqtt.connected())
    {
        return;
    }

    Serial.print("Connecting to MQTT... ");

    uint8_t retries = 3;
    while ((ret = mqtt.connect()) != 0) // connect will return 0 for connected
    {
        Serial.println(mqtt.connectErrorString(ret));
        Serial.println("Retrying MQTT connection in 5 seconds...");
        mqtt.disconnect();
        delay(5000);  // wait 5 seconds
        retries--;
        if (retries == 0)
        {
            // basically die and wait for WDT to reset me
            Serial.println("Connection Error");
            while (1);
        }
    }
    Serial.println("MQTT Connected!");
}

static char* GetHumidity()
{
    float h = dht.readHumidity();
    char cH[8];
    itoa(h, cH, 10);
    char cHumidity[30] = "Humidity    = ";
    strcat(cHumidity, cH);
    strcat(cHumidity, "%");
    return cHumidity;
}

static char* GetTemperature()
{
    float t = dht.readTemperature();
    char cT[8];
    itoa(t, cT, 10);
    char cTemperature[30] = "Temperature = ";
    strcat(cTemperature, cT);
    strcat(cTemperature, "C");
    return cTemperature;
}
