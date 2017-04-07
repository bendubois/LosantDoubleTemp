/* YourDuino Multiple DS18B20 Temperature Sensors on 1 wire
  Connections:
  DS18B20 Pinout (Left to Right, pins down, flat side toward you)
  - Left   = Ground
  - Center = Signal (Pin 2):  (with 3.3K to 4.7K resistor to +5 or 3.3 )
  - Right  = +5 or +3.3 V

   Questions: terry@yourduino.com 
   V1.01  01/17/2013 ...based on examples from Rik Kretzinger
   
/*-----( Import needed libraries )-----*/
#include <ESP8266WiFi.h>
#include <Losant.h>

// Get 1-wire Library here: http://www.pjrc.com/teensy/td_libs_OneWire.html
#include <OneWire.h>

//Get DallasTemperature Library here:  http://milesburton.com/Main_Page?title=Dallas_Temperature_Control_Library
#include <DallasTemperature.h>

/*-----( Declare Constants and Pin Numbers )-----*/
#define ONE_WIRE_BUS_PIN 5

/*-----( Declare objects )-----*/
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS_PIN);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

/*-----( Declare Variables )-----*/
// Assign the addresses of your 1-Wire temp sensors.
// See the tutorial on how to obtain these addresses:
// http://www.hacktronics.com/Tutorials/arduino-1-wire-address-finder.html

DeviceAddress Probe01 = { 0x28, 0xFF, 0xC1, 0x0F, 0x03, 0x15, 0x02, 0x76 }; 
DeviceAddress Probe02 = { 0x28, 0xFF, 0xB9, 0xC6, 0x02, 0x15, 0x02, 0x24 };
/*DeviceAddress Probe03 = { 0x28, 0x4D, 0x8D, 0x40, 0x04, 0x00, 0x00, 0x78 };
DeviceAddress Probe04 = { 0x28, 0x9A, 0x80, 0x40, 0x04, 0x00, 0x00, 0xD5 };
DeviceAddress Probe05 = { 0x28, 0xE1, 0xC7, 0x40, 0x04, 0x00, 0x00, 0x0D };*/

// WiFi credentials.
const char* WIFI_SSID = "--";
const char* WIFI_PASS = "--";

// Losant credentials.
const char* LOSANT_DEVICE_ID = "--";
const char* LOSANT_ACCESS_KEY = "--";
const char* LOSANT_ACCESS_SECRET = "--";

float Temp1 = 0;
float Temp2 = 0;
int dataDelay = 0;

WiFiClientSecure wifiClient;

LosantDevice device(LOSANT_DEVICE_ID);

void connect() {

  // Connect to Wifi.
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Connect to Losant.
  Serial.println();
  Serial.print("Connecting to Losant...");

  device.connectSecure(wifiClient, LOSANT_ACCESS_KEY, LOSANT_ACCESS_SECRET);

  while(!device.connected()) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Connected!");
  Serial.println();
  Serial.println("This device is now ready for use!");
}

void tempJSON() {
  Serial.println("Sending temp.");

  // Losant uses a JSON protocol. Construct the simple state object.
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["Temperature1"] = Temp1;
  root["Temperature2"] = Temp2;

  // Send the state to Losant.
  device.sendState(root);
}

void setup()   /****** SETUP: RUNS ONCE ******/
{
  // start serial port to show results
  Serial.begin(115200);
  Serial.print("Initializing Temperature Control Library Version ");
  Serial.println(DALLASTEMPLIBVERSION);
  
  // Initialize the Temperature measurement library
  sensors.begin();
  
  // set the resolution to 10 bit (Can be 9 to 12 bits .. lower is faster)
  sensors.setResolution(Probe01, 10);
  sensors.setResolution(Probe02, 10);
  /*sensors.setResolution(Probe03, 10);
  sensors.setResolution(Probe04, 10);
  sensors.setResolution(Probe05, 10);*/

  connect();
}//--(end setup )---

void loop()   /****** LOOP: RUNS CONSTANTLY ******/
{
  bool toReconnect = false;

  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("Disconnected from WiFi");
    toReconnect = true;
  }

  if(!device.connected()) {
    Serial.println("Disconnected from Losant");
    Serial.println(device.mqttClient.state());
    toReconnect = true;
  }

  if(toReconnect) {
    connect();
  }
  device.loop();
  
  delay(1000);
  Serial.println();
  Serial.print("Number of Devices found on bus = ");  
  Serial.println(sensors.getDeviceCount());   
  Serial.print("Getting temperatures... ");  
  Serial.println();   
  
  // Command all devices on bus to read temperature  
  sensors.requestTemperatures();  
  
  Temp1 = sensors.getTempF(Probe01);
  Temp2 = sensors.getTempF(Probe02);
  
  Serial.print("Probe 01 temperature is:   ");
  printTemperature(Probe01);
  Serial.println();

  Serial.print("Probe 02 temperature is:   ");
  printTemperature(Probe02);
  Serial.println();
 
 /* Serial.print("Probe 03 temperature is:   ");
  printTemperature(Probe03);
  Serial.println();
   
  Serial.print("Probe 04 temperature is:   ");
  printTemperature(Probe04);
  Serial.println();
  
  Serial.print("Probe 05 temperature is:   ");
  printTemperature(Probe05);
  Serial.println();*/

  if(dataDelay >=20){
    tempJSON();
    dataDelay = 0;
  }

  dataDelay ++;
  //delay(20000); //delay between sending data points
}//--(end main loop )---

/*-----( Declare User-written Functions )-----*/
void printTemperature(DeviceAddress deviceAddress)
{

float tempC = sensors.getTempC(deviceAddress);

   if (tempC == -127.00) 
   {
   Serial.print("Error getting temperature  ");
   } 
   else
   {
   Serial.print("C: ");
   Serial.print(tempC);
   Serial.print(" F: ");
   Serial.print(DallasTemperature::toFahrenheit(tempC));
   }
}// End printTemperature
//*********( THE END )***********
