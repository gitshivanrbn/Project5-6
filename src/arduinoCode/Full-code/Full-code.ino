// libraries
#include <Wire.h> //I2C Arduino Library

// defines
#define slow 0
#define medium 175
#define fast 255

// compass sensor
 //Analog input 4 I2C SDA
 //Analog input 5 I2C SCL
#define address 0x1E //0011110b, I2C 7bit address of HMC5883

/*
 * startup
 */
const int reedSwitch = 8;
int reedState = 0;

/*
 * compass
 */ 
int x,y,z; //triple axis data

float startingDegree;
float heading;

bool checkAllowed = false;

/* 
 * 2 ultrasonic
 */
const int trigPinS = 3; // Front
const int echoPinS = 9; // Front
const int trigPinF = 10; // Side
const int echoPinF = 11; // Side

//links
const int motor1A = 4; // 
const int motor1B = 5; // 
const int motor2A = 7; // 
const int motor2B = 6; // 


// defines variables
long durationF;
int distanceF = 0;
long durationS;
int distanceS = 0;

// distances 
int startingDistanceF = 200;
int startingDistanceS = 10000;

int diverDistanceF;
int diverDistanceS;
   
int distanceFAvg = 0;
int distanceSAvg = 0;
int front = 0;
int side = 0;

void setup() {
  Serial.begin(9600);
  
  Wire.begin();
  
  //Put the HMC5883 IC into the correct operating mode
  Wire.beginTransmission(address); //open communication with HMC5883
  Wire.write(0x02); //select mode register
  Wire.write(0x00); //continuous measurement mode
  Wire.endTransmission();
  
  // setup pinmode
  pinMode(trigPinF, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPinF, INPUT); // Sets the echoPin as an Input
  pinMode(trigPinS, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPinS, INPUT); // Sets the echoPin as an Input

  pinMode(motor1A, OUTPUT);
  pinMode(motor1B, OUTPUT);
  pinMode(motor2A, OUTPUT);
  pinMode(motor2B, OUTPUT);

  pinMode(reedSwitch, INPUT);

  Serial.println("reset");

  ini();
}

void(* resetFunc) (void) = 0; // declare reset function @ address 0

void loop() {
  // put your main code here, to run repeatedly:
  
  ultrasonic();

  compass();

  roundBath();
  if(!(distanceS > startingDistanceS/2)) {
    //Serial.println("Motor");
    motor();
  }
  else {
    //stopSequence();
    //Serial.println("Stop");
  }
}

void compass() {
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
  
  heading = atan2(z, x) * 180 / PI; // calculate heading direction in degree (-180 , 180)
}

int calcDiv(int num) { // calculate from -180 to +180
  if(startingDegree + num > 180) {
    return -180 + (startingDegree + num - 180);
  }
  else if(startingDegree + num < -180) {
    return 180 + (startingDegree + num + 180);
  }
  else {
    return startingDegree + num;
  }
}

void roundBath() { // pathfinding for a round bath
  if(checkAllowed) {
    if(calcDiv(20) < 0 && startingDegree >= 0 || calcDiv(-20) >= 0 && startingDegree < 0) {
      if(heading <= calcDiv(20) || heading >= calcDiv(-20)) {
        Serial.println("At starting position");
        checkAllowed = false;
        // verder naar binnen
        diverDistanceS += 50;
        Serial.println(startingDistanceS);
      }
    }
    else {
      if(heading <= calcDiv(20) && heading >= calcDiv(-20)) {
        Serial.println("At starting position");
        checkAllowed = false;
        // verder naar binnen
        diverDistanceS += 50;
        Serial.println(startingDistanceS);
      }
    }
  }
  else {
    if(calcDiv(110) < 0 && startingDegree >= 0 || calcDiv(70) >= 0 && startingDegree < 0) {
      if(heading <= calcDiv(110) && heading >= calcDiv(70)) {
        Serial.println("Reverse");
        checkAllowed = true;
      }
    }
    else {
      if(heading <= calcDiv(110) && heading >= calcDiv(70)) {
        Serial.println("Reverse");
        checkAllowed = true;
      }
    }
  }
}

void ultrasonic() { // reading ultrasonic
  distanceSAvg = 0;
  distanceFAvg = 0;
  front = 0;
  side = 0;

      
    // Clears the trigPin
    digitalWrite(trigPinF, LOW);
    delayMicroseconds(2);
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPinF, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPinF, LOW);
    // Reads the echoPin, returns the sound wave travel time in microseconds
    durationF = pulseIn(echoPinF, HIGH);
    // Calculating the distance
    distanceF = durationF*0.034/2;
    // Prints the distance on the Serial Monitor
    
    // Clears the trigPin
    digitalWrite(trigPinS, LOW);
    delayMicroseconds(2)  ;
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPinS, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPinS, LOW);
    // Reads the echoPin, returns the sound wave travel time in microseconds
    durationS = pulseIn(echoPinS, HIGH);
    // Calculating the distance
    distanceS = durationS*0.034/2;
    // Prints the distance on the Serial Monitor

    avg(); // avarage values
}

void avg() { // avarage values
  for (int i = 0; i < 3; i++){
    if(distanceF < startingDistanceF + 20) {
      front++;
      distanceFAvg += distanceF;
  
    }
  }

  for (int i = 0; i < 3; i++){
    if(distanceS < diverDistanceS + 30) {
      side++;
      distanceSAvg += distanceS;
    }
  }

  if(front != 0) {
    distanceF = distanceFAvg / front;

    //Serial.print("DistanceF: ");
    //Serial.println(distanceF);
  }
//  else {
//    distanceF = startingDistanceF;
//  }

  if(side != 0) {
    distanceS = distanceSAvg / side;
    //Serial.print("DistanceS: ");
    //Serial.println(distanceS);
  }
//  else {
//    distanceS = startingDistanceS;
//  }
}

void motor() {
  //if robot is parallel to the wall and no obstacles at the front
  if(distanceS >= diverDistanceS - 5 && distanceS <= diverDistanceS + 5) {
    analogWrite(motor1A, LOW);
    analogWrite(motor1B, fast);
    analogWrite(motor2A, LOW);
    analogWrite(motor2B, fast);
  }
  else if(distanceS > diverDistanceS + 10) {
    analogWrite(motor1A, LOW);
    analogWrite(motor1B, fast);
    analogWrite(motor2A, LOW);
    analogWrite(motor2B, slow);
    delay(100);
    analogWrite(motor1A, LOW);
    analogWrite(motor1B, fast);
    analogWrite(motor2A, LOW);
    analogWrite(motor2B, medium);
    delay(20);
  }    
  //if robot is too far from the wall
  else if(distanceS > diverDistanceS + 5) {
    analogWrite(motor1A, LOW);
    analogWrite(motor1B, fast);
    analogWrite(motor2A, LOW);
    analogWrite(motor2B, medium);
  }    
  else if(distanceS > diverDistanceS - 10) {
    analogWrite(motor1A, LOW);
    analogWrite(motor1B, slow);
    analogWrite(motor2A, LOW);
    analogWrite(motor2B, fast);
    delay(100);
    analogWrite(motor1A, LOW);
    analogWrite(motor1B, medium);
    analogWrite(motor2A, LOW);
    analogWrite(motor2B, fast);
    delay(20);
  }    
  //if robot is too close to the wall and no obstacles at the front
  else if(distanceS < diverDistanceS -5) {
    analogWrite(motor1A, LOW);
    analogWrite(motor1B, medium);
    analogWrite(motor2A, LOW);
    analogWrite(motor2B, fast);
  }
}


void ini() {
  do {
    reedState = digitalRead(reedSwitch); // read reed switch
    //Serial.println("switch");
  } while(reedState == 1);

  //delay(10000); // wait for user to place in right spot

  // indicator light???
  
  compass(); // get values for begin value
  startingDegree = heading;

  do {
    ultrasonic();

    //startingDistanceF = distanceF; ???
    //startingDistanceS = distanceS;
    //Serial.println("Test");
  }
  while(startingDistanceS == 0 && startingDistanceF ==0); 
  diverDistanceF = 40; 
  diverDistanceS = 40; 
}

void stopSequence() {
  analogWrite(motor1A, LOW);
  analogWrite(motor1B, fast);
  analogWrite(motor2A, LOW);
  analogWrite(motor2B, fast);
  while(distanceF > 25) {
    Serial.println(distanceF);
    ultrasonic();
  }
  
  analogWrite(motor1A, LOW);
  analogWrite(motor1B, LOW);
  analogWrite(motor2A, LOW);
  analogWrite(motor2B, LOW);

  resetFunc();  //call reset
}
