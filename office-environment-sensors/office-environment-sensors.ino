/******************************************************************************
  Office Environment Sensors Example (Rosie Hub Compartible)
  Brandon Satrom
  Original Creation Date: July 26, 2016
  Updated July 26th, 2016

  This sketch measures the temperature, humidity, and barometric pressure and
  logs those readings via MQTT to a node-red instance running as a Smart Home
  controller. The controller then forwards that information to an InfluxDB
  instance for collection and vizualization.

  The Shield library used in this example can be found here:
  https://github.com/sparkfun/SparkFun_Photon_Weather_Shield_Particle_Library

  The MQTT library can be found here:
  https://github.com/hirotakaster/MQTT

  Hardware Connections:
	This sketch was written specifically for the Photon Weather Shield,
	which connects the HTU21D and MPL3115A2 to the I2C bus by default.
  If you have an HTU21D and/or an MPL3115A2 breakout,	use the following
  hardware setup:
      HTU21D ------------- Photon
      (-) ------------------- GND
      (+) ------------------- 3.3V (VCC)
       CL ------------------- D1/SCL
       DA ------------------- D0/SDA

    MPL3115A2 ------------- Photon
      GND ------------------- GND
      VCC ------------------- 3.3V (VCC)
      SCL ------------------ D1/SCL
      SDA ------------------ D0/SDA

  Development environment specifics:
  	IDE: Particle Dev
  	Hardware Platform: Particle Photon
                       SparkFun Photon Weather Shield
*******************************************************************************/
#include "SparkFun_Photon_Weather_Shield_Library.h"
#include "MQTT.h"

// Configure MQTT Server
byte server[] = { 10, 171, 5, 102 }; //Replace with local network MQTT Server

 MQTT client(server, 1883, mqtt_callback);
// If you want to use domain name or server name,
// MQTT client("www.sample.com", 1883, callback)git statu

float humidity = 0;
float tempf = 0;
float pascals = 0;
float baroTemp = 0;

long lastPrint = 0;

//Create Instance of HTU21D or SI7021 temp and humidity sensor and MPL3115A2 barometric sensor
Weather sensor;

//---------------------------------------------------------------
void setup()
{
    Serial.begin(9600);   // open serial over USB at 9600 baud

    sensor.begin();
    sensor.setModeBarometer();//Set to Barometer Mode
    //These are additional MPL3115A2 functions that MUST be called for the sensor to work.
    sensor.setOversampleRate(7); // Set Oversample rate
    //Call with a rate from 0 to 7. See page 33 for table of ratios.
    //Sets the over sample rate. Datasheet calls for 128 but you can set it
    //from 1 to 128 samples. The higher the oversample rate the greater
    //the time between data samples.

    sensor.enableEventFlags(); //Necessary register calls to enble temp, baro and alt

    //Init MQTT client
    //connect to the server
    client.connect("Home_Office_Client");
}
//---------------------------------------------------------------

void mqtt_callback(char* topic, byte* payload, unsigned int length)
{
    char p[length + 1];
    memcpy(p, payload, length);
    p[length] = NULL;
    String message(p);

    Serial.println("Ack: ");
    Serial.println(message);
}

void loop()
{
      if(millis() - lastPrint > 5000)
      {
        // Record when you published
        lastPrint = millis();

        getWeather();

        logWeather(); //Log to NodeRed
        printInfo(); //Print to Serial
      }
}
//---------------------------------------------------------------
void getWeather()
{
  // Measure Relative Humidity from the HTU21D or Si7021
  humidity = sensor.getRH();

  // Measure Temperature from the HTU21D or Si7021
  tempf = sensor.getTempF();
  // Temperature is measured every time RH is requested.
  // It is faster, therefore, to read it from previous RH
  // measurement with getTemp() instead with readTemp()

  //Measure the Barometer temperature in F from the MPL3115A2
  baroTemp = sensor.readBaroTempF();

  //Measure Pressure from the MPL3115A2
  pascals = sensor.readPressure();

  //If in altitude mode, you can get a reading in feet with this line:
  //float altf = sensor.readAltitudeFt();
}
//---------------------------------------------------------------

void logWeather()
{
  Serial.print("Connected: ");
  Serial.println(client.isConnected());

  if (client.isConnected()) {
        // get messageid parameter at 4.
        uint16_t messageid;

        // string of concatenated environmentalValues
        String values = "{\"environment\": \"office\", \"temp\": " + String(tempf) + ", \"humidity\": " + String(humidity) + ", \"baro\": " + String(baroTemp) + ", \"pressure\": " + String(pascals/100) + "}";

        client.publish("/environment", values, MQTT::QOS1, &messageid);

        Serial.print("MessageId: ");
        Serial.println(messageid);

        client.subscribe("/environment_ack");
    } else {
      //Try to reconnect
      client.connect("Home_Office_Client");
    }
}

void printInfo()
{
//This function prints the weather data out to the default Serial Port

  Serial.print("Temp:");
  Serial.print(tempf);
  Serial.print("F, ");

  Serial.print("Humidity:");
  Serial.print(humidity);
  Serial.print("%, ");

  Serial.print("Baro_Temp:");
  Serial.print(baroTemp);
  Serial.print("F, ");

  Serial.print("Pressure:");
  Serial.print(pascals/100);
  Serial.println("hPa");
}
