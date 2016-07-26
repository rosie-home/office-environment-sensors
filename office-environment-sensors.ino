/******************************************************************************
  Office Environment Sensors Example (Rosie Hub Compartible)
  Brandon Satrom
  Original Creation Date: July 26, 2016
  Updated July 26th, 2016

  This sketch measures the temperature, humidity, and barometric pressure and
  logs those readings via MQTT to a node-red instance running as a Smart Home
  controller. The controller then forwards that information to an InfluxDB
  instance for collection and vizualization.

  The library used in this example can be found here:
  https://github.com/sparkfun/SparkFun_Photon_Weather_Shield_Particle_Library

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
    Particle.publish("Init");

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
}
//---------------------------------------------------------------
void loop()
{
      if(millis() - lastPrint > 5000)
      {
        // Record when you published
        lastPrint = millis();

        getWeather();

        //logInfo(); //Log to NodeRed
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
void printInfo()
{
//This function prints the weather data out to the default Serial Port

  Particle.publish("Temp", String(tempf));

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
  Serial.print("hPa, ");
  Serial.print((pascals/100) * 0.0295300);
  Serial.println("in.Hg");
  //The MPL3115A2 outputs the pressure in Pascals. However, most weather stations
  //report pressure in hectopascals or millibars. Divide by 100 to get a reading
  //more closely resembling what online weather reports may say in hPa or mb.
  //Another common unit for pressure is Inches of Mercury (in.Hg). To convert
  //from mb to in.Hg, use the following formula. P(inHg) = 0.0295300 * P(mb)
  //More info on conversion can be found here:
  //www.srh.noaa.gov/images/epz/wxcalc/pressureConversion.pdf
}
