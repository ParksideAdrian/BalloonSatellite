#include <SPI.h>
#include <SD.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#define DHTPIN 2 //Analog 2
#define DHTTYPE DHT22
#include<Wire.h>
#define Addr 0x60

DHT dht (DHTPIN, DHTTYPE);

int tempPin = 0;
int i;
float samplest[10];
float samplesh[10];
const float e = 2.71828;
const int chipSelect = 10;

//const int motor = 9;
//int seconds = 65;

void setup() {

    // Initialise I2C communication
  Wire.begin();
  // Initialise Serial Communication, set baud rate = 9600
  Serial.begin(9600);

  // Start I2C transmission
  Wire.beginTransmission(Addr);
  // Select control register
  Wire.write(0x26);
  // Active mode, OSR = 128, altimeter mode
  Wire.write(0xB9);
  // Stop I2C transmission
  Wire.endTransmission();

  // Start I2C transmission
  Wire.beginTransmission(Addr);
  // Select data configuration register
  Wire.write(0x13);
  // Data ready event enabled for altitude, pressure, temperature
  Wire.write(0x07);
  // Stop I2C transmission
  Wire.endTransmission();
  delay(300);

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
  //pinMode(motor, OUTPUT);
}

void loop() {

  //****************************************************

  //timer(seconds);

  unsigned int data[6];

  // Start I2C transmission
  Wire.beginTransmission(Addr);
  // Select control register
  Wire.write(0x26);
  // Active mode, OSR = 128, altimeter mode
  Wire.write(0xB9);
  // Stop I2C transmission
  Wire.endTransmission();
  delay(1000);

  // Start I2C transmission
  Wire.beginTransmission(Addr);
  // Select data register
  Wire.write(0x00);
  // Stop I2C transmission
  Wire.endTransmission();

  // Request 6 bytes of data
  Wire.requestFrom(Addr, 6);

  // Read 6 bytes of data from address 0x00(00)
  // status, tHeight msb1, tHeight msb, tHeight lsb, temp msb, temp lsb
  if (Wire.available() == 6)
  {
    data[0] = Wire.read();
    data[1] = Wire.read();
    data[2] = Wire.read();
    data[3] = Wire.read();
    data[4] = Wire.read();
    data[5] = Wire.read();
  }


  // Convert the data to 20-bits
  int tHeight = (((long)(data[1] * (long)65536) + (data[2] * 256) + (data[3] & 0xF0)) / 16);
  int temp = ((data[4] * 256) + (data[5] & 0xF0)) / 16;
  float altitude = tHeight / 16.0;
  float altitudeft = (altitude * 3.28084);
  float cTemp = (temp / 16.0);
  float fTemp = cTemp * 1.8 + 32;

  // Start I2C transmission
  Wire.beginTransmission(Addr);
  // Select control register
  Wire.write(0x26);
  // Active mode, OSR = 128, barometer mode
  Wire.write(0x39);
  // Stop I2C transmission
  Wire.endTransmission();
  delay(1000);

  // Start I2C transmission
  Wire.beginTransmission(Addr);
  // Select data register
  Wire.write(0x00);
  // Stop I2C transmission
  Wire.endTransmission();

  // Request 4 bytes of data
  Wire.requestFrom(Addr, 4);

  // Read 4 bytes of data
  // status, pres msb1, pres msb, pres lsb
  if (Wire.available() == 4)
  {
    data[0] = Wire.read();
    data[1] = Wire.read();
    data[2] = Wire.read();
    data[3] = Wire.read();
  }


  // Convert the data to 20-bits
  long pres = (((long)data[1] * (long)65536) + (data[2] * 256) + (data[3] & 0xF0)) / 16;
  float pressure = (pres / 4.0) / 1000.0;

  float altitude1 = (1-pow((pressure*10)/1013.25,0.190284)) * 145366.45;


  //****************************************************

  float voltage, sensorValue, PPM;
  float h = dht.readHumidity();
  float t = dht.readTemperature(); //measure temp in C
  float f = dht.readTemperature() * (9.0/5.0) + 32.0;

  // make a string for assembling the data to log:
  String dataString1 = "";
  String dataString2 = "";
  String dataString3 = "";

  // read three sensors and append to the string:
  //for (int analogPin = 0; analogPin < 3; analogPin++) {
    int analogPin = 2;
    sensorValue = analogRead(analogPin);
    voltage = analogRead(2)*(0.00080586);
    PPM = 1.938*e*(1.7742*voltage);
    dataString1 += "Methane Values: Sensor Reading:  ";
    dataString1 += String(sensorValue);
    dataString1 += ", Voltage: ";
    dataString1 += String(voltage);
    dataString1 += ", PPM: ";
    dataString1 += String(PPM);

//Altitude 1 reads altitude in meters from the MPL3115 sensor
//Altitude 2 reads altitude in ft as a function of pressure
    dataString3 += "Altitude1 : ";
    dataString3 += String(altitude);
    dataString3 += " m \n";
    dataString3 += "Altitude2 : ";
    dataString3 += String(altitude1);
    dataString3 += " ft \n";
    dataString3 += "Pressure : ";
    dataString3 += String(pressure);
    dataString3 += " kPa \n";
    dataString3 += "Temperature in Celcius : ";
    dataString3 += String(cTemp);
    dataString3 += " C \n";
    dataString3 += "Temperature in Fahrenheit : ";
    dataString3 += String(fTemp);
    dataString3 += " F \n";

    //if (analogPin < 2) {
      //dataString += ",";
    //}
  //}

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString1);
    //dataFile.println(dataString2);
    dataFile.println(dataString3);
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString1);
    //Serial.println(dataString2);//
    Serial.println(dataString3);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }
  delay (3000);

//Timer code for usable DC motor

/*
Serial.println(seconds);
  if (seconds == 0) {
            for (int x = 0; x < 100000; x++) {
                digitalWrite(motor, 450);
            }
  }
  delay(1000);
  seconds--;
}


void timer(int seconds) {
    for (int i = seconds; i >= 0; i--) {
        Serial.print(i);
        if (i == 0) {
            for (int x = 0; x < 100000; x++) {
                digitalWrite(motor, 450);
            }
        }
        delay(1000);
    }*/
}
