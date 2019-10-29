// libraries
#include <Wire.h> //I2C Arduino Library

// compass sensor
// Analog input 4 I2C SDA
// Analog input 5 I2C SCL
#define address 0x1E //0011110b, I2C 7bit address of HMC5883

int startingDegreeX = -5000;
int startingDegreeY = -5000;

// ultrasonic sensor
// defines pins numbers
const int trigPin = 9;
const int echoPin = 10;
// defines variables
long duration;
int distance;

void setup(){
  //Initialize Serial and I2C communications
  Serial.begin(9600);
  Wire.begin();
  
  //Put the HMC5883 IC into the correct operating mode
  Wire.beginTransmission(address); //open communication with HMC5883
  Wire.write(0x02); //select mode register
  Wire.write(0x00); //continuous measurement mode
  Wire.endTransmission();

  // setup pinmode
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
}

void loop(){
  compass();
  ultrasonic();
  delay(250);
}

void compass() {
  int x,y,z; //triple axis data

  //Tell the HMC5883 where to begin reading data
  Wire.beginTransmission(address);
  Wire.write(0x03); //select register 3, X MSB register
  Wire.endTransmission();
  
  //Read data from each axis, 2 registers per axis
  Wire.requestFrom(address, 6);
  if(6<=Wire.available()){
    x = Wire.read()<<8; //X msb
    x |= Wire.read(); //X lsb
    z = Wire.read()<<8; //Z msb
    z |= Wire.read(); //Z lsb
    y = Wire.read()<<8; //Y msb
    y |= Wire.read(); //Y lsb
  }
  
  //Print out values of each axis
  Serial.print("x: ");
  Serial.print(x);
  Serial.print("\t");
  Serial.println(startingDegreeX);
  Serial.print("y: ");
  Serial.print(y);
  Serial.print("\t");
  Serial.println(startingDegreeY);

  if(startingDegreeX == -5000 && startingDegreeY == -5000) {
    startingDegreeX = x;
    startingDegreeY = y;
  }

  if(x <= startingDegreeX + 20 && x >= startingDegreeX - 20 && y <= startingDegreeY + 20 && y >= startingDegreeY - 20) {
    Serial.println("At starting position");
  }
}

void ultrasonic() {
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance= duration*0.034/2;
  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.println(distance);
}
