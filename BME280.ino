#include "Wire.h"
// Include the SparkFun Phant library.
// Include SparkFun BME280 library
#include "SparkFunBME280.h"
#include "ESP8266WiFi.h"
#include <Phant.h>

BME280 mySensor;


const char WiFiSSID[] = "Swapnil's iPhone";
const char WiFiPSK[] = "8s5esfhw0ng8m";

const char PhantHost[] = "data.sparkfun.com";
const char PublicKey[] = "bGJbO8GMjzFNbzA0YwVY";
const char PrivateKey[] = "VpMm09pkRAsEzK7JdZnd";

const unsigned long postRate = 1000 * 60 * 30;
unsigned long lastPost = 0;

void setup()
{
  // initHardware(); // Setup input/output I/O pins
  connectWiFi(); // Connect to WiFi

  //For I2C, enable the following and disable the SPI section
  mySensor.settings.commInterface = I2C_MODE;
  mySensor.settings.I2CAddress = 0x77;
  //***Operation settings*****************************//
  mySensor.settings.runMode = 3; // 3, Normal mode
  mySensor.settings.tStandby = 0; // 0, 0.5ms
  mySensor.settings.filter = 2; // 0, filter off
  //tempOverSample can be:
  // 0, skipped
  // 1 through 5, oversampling *1, *2, *4, *8, *16 respectively
  mySensor.settings.tempOverSample = 1;
  //pressOverSample can be:
  // 0, skipped
  // 1 through 5, oversampling *1, *2, *4, *8, *16 respectively
  mySensor.settings.pressOverSample = 1;
  //humidOverSample can be:
  // 0, skipped
  // 1 through 5, oversampling *1, *2, *4, *8, *16 respectively
  mySensor.settings.humidOverSample = 1;


  Serial.begin(115200);
  Serial.print("Program Started\n");

  Serial.print("Program Started\n");
  Serial.print("Starting BME280... result of .begin(): 0x");

  //Calling .begin() causes the settings to be loaded
  delay(10);  //Make sure sensor had enough time to turn on. BME280 requires 2ms to start up.
  Serial.println(mySensor.begin(), HEX);

  Serial.print("Displaying ID, reset and ctrl regs\n");

  Serial.print("ID(0xD0): 0x");
  Serial.println(mySensor.readRegister(BME280_CHIP_ID_REG), HEX);
  Serial.print("Reset register(0xE0): 0x");
  Serial.println(mySensor.readRegister(BME280_RST_REG), HEX);
  Serial.print("ctrl_meas(0xF4): 0x");
  Serial.println(mySensor.readRegister(BME280_CTRL_MEAS_REG), HEX);
  Serial.print("ctrl_hum(0xF2): 0x");
  Serial.println(mySensor.readRegister(BME280_CTRL_HUMIDITY_REG), HEX);

  Serial.print("\n\n");

  connectWiFi();
}
void loop()
{
  
  Serial.print("Temperature: ");
  Serial.print(mySensor.readTempC(), 2);
  Serial.println(" degrees C");

  Serial.print("Temperature: ");
  Serial.print(mySensor.readTempF(), 3);
  Serial.println(" degrees F");

  Serial.print("Pressure: ");
  Serial.print(mySensor.readFloatPressure(), 0);

  Serial.print("Altitude: ");
  Serial.print(mySensor.readFloatAltitudeMeters(), 3);
  Serial.println("m");

  Serial.print("Altitude: ");
  Serial.print(mySensor.readFloatAltitudeFeet(), 3);
  Serial.println("ft");

  Serial.print("%RH: ");
  Serial.print(mySensor.readFloatHumidity(), 0);
  Serial.println(" %");

  Serial.println();

  delay(1000);
  unsigned int delaytime;
  Serial.println("Posting to Phant!");
  for (int i = 0; i < 10; i++)
  {
    if (postToPhant())
    {
      lastPost = millis();
      Serial.println("Post Suceeded!");
    }
    else // If the Phant post failed
    {
      Serial.println("Post failed, will try again.");
    }
    delaytime = postRate;
    delay(3000); // Short delay, then next post
  }

}
void connectWiFi()
{
  delay(1);
  Serial.println();
  Serial.println("Connecting to: " + String(WiFiSSID));
  // Set WiFi mode to station (as opposed to AP or AP_STA)
  WiFi.mode(WIFI_STA);
  // WiFI.begin([ssid], [passkey]) initiates a WiFI connection
  // to the stated [ssid], using the [passkey] as a WPA, WPA2,
  // or WEP passphrase.
  WiFi.begin(WiFiSSID, WiFiPSK);
  // Use the WiFi.status() function to check if the ESP8266
  // is connected to a WiFi network.
  while (WiFi.status() != WL_CONNECTED)
  {

    delay(15000);
    Serial.print(".");
    // Potentially infinite loops are generally dangerous.
    // Add delays -- allowing the processor to perform other
    // tasks -- wherever possible.
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
void initHardware()
{
  Serial.begin(115200);
}
int postToPhant()
{

  // Declare an object from the Phant library - phant
  Phant phant(PhantHost, PublicKey, PrivateKey);
  // Add the three field/value pairs defined by our stream:

  phant.add("temp_f", mySensor.readTempF());
  phant.add("humidity", mySensor.readFloatHumidity());
  phant.add("pressure_kpa", mySensor.readFloatPressure() / 1000);

  // Now connect to data.sparkfun.com, and post our data:
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(PhantHost, httpPort))
  {
    // If we fail to connect, return 0.
    return 0;
  }
  // If we successfully connected, print our Phant post:
  client.print(phant.post());
  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    //Serial.print(line); // Trying to avoid using serial
  }
  //Print each row in the loop
  //Start with temperature, as that data is needed for accurate compensation.
  //Reading the temperature updates the compensators of the other functions
  //in the background.

  Serial.print(mySensor.readTempC(), 2);
  Serial.print(",");
  Serial.print(mySensor.readTempF(), 3);
  Serial.print(",");
  Serial.print(mySensor.readFloatPressure(), 0);
  Serial.print(",");
  Serial.print(mySensor.readFloatAltitudeMeters(), 3);
  Serial.print(",");
  Serial.print(mySensor.readFloatAltitudeFeet(), 3);
  Serial.print(",");
  Serial.print(mySensor.readFloatHumidity(), 0);
  Serial.print(",");

  // Before we exit, turn the LED off.
 
  return 1; // Return success
}
void printError(byte error)
// If there's an I2C error, this function will
// print out an explanation.
{
  Serial.print("I2C error: ");
  Serial.print(error, DEC);
  Serial.print(", ");
  switch (error)
  {
    case 0:
      Serial.println("success");
      break;
    case 1:
      Serial.println("data too long for transmit buffer");
      break;
    case 2:
      Serial.println("other error");
      break;
    default:
      Serial.println("unknown error");
  }
}
