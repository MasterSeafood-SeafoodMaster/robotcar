#include <WiFi.h>
#include <Wire.h>

// robot car pin
#define PIN_IN1  7 
#define PIN_IN2  8 
#define PIN_ENA  14 
#define PIN_IN3  9 
#define PIN_IN4  10 
#define PIN_ENB  15

// distance cencer pin
#define PIN_FRONT 20
#define PIN_BACK 26

// 3axis adxl345 pin
#define PIN_ADXL_SDA 37
#define PIN_ADXL_SCL 36
#define Register_2D 0x2D
#define Register_X0 0x32
#define Register_X1 0x33
#define Register_Y0 0x34
#define Register_Y1 0x35
#define Register_Z0 0x36
#define Register_Z1 0x37
int ADXAddress = 83; // ADXL345 device address
double Xg,Yg,Zg;
double Roll = 0.00, Pitch = 0.00;   //Roll & Pitch are the angles which rotate by the axis X and Y

const char* ssid = "DESKTOP-9F1K6GA 3362";  //網路熱點名稱 想電腦控制車子就必需用筆電開熱點給車子連接 必須是2.4Hz
const char* password = "8*33v7J3";          //網路熱點密碼
WiFiServer wifiServer(8000);
// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pins as outputs.
  // pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_ENA, OUTPUT);
  pinMode(PIN_IN3, OUTPUT);
  pinMode(PIN_IN4, OUTPUT);
  pinMode(PIN_ENB, OUTPUT);
  connectWIFI();
  setupADXL();
}

void connectWIFI() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    Serial.println("Connecting to WiFi..");
    digitalWrite(LED_BUILTIN, LOW);
  }
  Serial.println("Connected to the WiFi network");
  Serial.println(WiFi.localIP());
  wifiServer.begin();  
}

void speedup(int mSpeed) {
  for (int speed = 0; speed <= mSpeed; speed+=5) {
    analogWrite(PIN_ENA, speed); // speed up
    analogWrite(PIN_ENB, speed); 
    delay(1);
 } 
}

void speeddown(int mSpeed) {
  for (int speed = mSpeed; speed >= 0; speed-=5) {
    analogWrite(PIN_ENA, speed); // speed down
    analogWrite(PIN_ENB, speed); 
    delay(1);
  }
}
int Speed = 50;
void motor(int dir) {
  if (dir==0) { //FORWARD
      analogWrite(PIN_ENA, Speed);
      analogWrite(PIN_ENB, Speed); 
      digitalWrite(PIN_IN1, HIGH); // A
      digitalWrite(PIN_IN2, LOW);  // A
      digitalWrite(PIN_IN3, HIGH); // B
      digitalWrite(PIN_IN4, LOW);  // B
  }
  else if (dir==1){ //BACKWARD
      analogWrite(PIN_ENA, Speed);
      analogWrite(PIN_ENB, Speed); 
      digitalWrite(PIN_IN1, LOW); // A
      digitalWrite(PIN_IN2, HIGH);  // A
      digitalWrite(PIN_IN3, LOW); // B
      digitalWrite(PIN_IN4, HIGH);  // B
  }
  else if (dir==3){ //RIGHT
      analogWrite(PIN_ENA, Speed);
      analogWrite(PIN_ENB, Speed); 
      digitalWrite(PIN_IN1, LOW); // A
      digitalWrite(PIN_IN2, HIGH);  // A
      digitalWrite(PIN_IN3, HIGH); // B
      digitalWrite(PIN_IN4, LOW);  // B
  }
  else if (dir==2){ //LEFT
      analogWrite(PIN_ENA, Speed);
      analogWrite(PIN_ENB, Speed); 
      digitalWrite(PIN_IN1, HIGH); // A
      digitalWrite(PIN_IN2, LOW);  // A
      digitalWrite(PIN_IN3, LOW); // B
      digitalWrite(PIN_IN4, HIGH);  // B
  }
  else if (dir==4) {
      analogWrite(PIN_ENA, 0);
      analogWrite(PIN_ENB, 0); 
      digitalWrite(PIN_IN1, LOW); // A
      digitalWrite(PIN_IN2, LOW);  // A
      digitalWrite(PIN_IN3, LOW); // B
      digitalWrite(PIN_IN4, LOW);  // B
  }
}

char last;

// the loop function runs over and over again forever
void loop() {
  long front, back;
  char barrier = 'n';
  WiFiClient client = wifiServer.available();
  if (client) {
    while (client.connected()) {
      // read user keyboard control data from client
      char cutup = client.read(); 
      Serial.println(cutup);
      // 獲取車子當前傾斜角度
      detectCarAngle();
      // showData();
      // angle = detectCarPosicion(cutup);
      // 距離危險偵測
      front = detectDistance(PIN_FRONT);
      back = detectDistance(PIN_BACK);
      Serial.println("back"+String(back));
      Serial.println("front"+String(front));
      if((back < 10) && (front < 10)){
        Serial.println("stop");
        cutup = 'n';
      }else if(back < 10){
        Serial.println("auto move to front");
        cutup = 'w';
        barrier = 'y';
        Serial.println(barrier);
      }else if(front < 10){
        Serial.println("auto move to back");
        cutup = 's';
        barrier = 'y';
        Serial.println(barrier);
      }
      // 根據client key遙控車子
      if (cutup=='w') {
        // digitalWrite(LED_BUILTIN, HIGH);     
        motor(0);
        //if (last=='n') {speedup(Speed/2);}     
        last='w';
      }
      else if (cutup=='a') {
       // digitalWrite(LED_BUILTIN, HIGH);
       motor(3);
       //if (last=='n') {speedup(Speed/2);} 
       last='a';
      }
      else if (cutup=='s') {
        // digitalWrite(LED_BUILTIN, HIGH);
        motor(1);
        //if (last=='n') {speedup(Speed/2);} 
        last='s';
      }
      else if (cutup=='d') {
        // digitalWrite(LED_BUILTIN, HIGH);
        motor(2);
        //if (last=='n') {speedup(Speed/2);}
        last='d';
      }
      else if (cutup=='n') {
        //if (last!='n') {speeddown(Speed/2);}
        motor(4);
        last='n';               
      }
      String s = "Hello from ESP32!";
      String axis = String(Xg)+','+String(Yg)+','+String(Zg);
      String tilt = String(Roll)+','+String(Pitch);
      client.print(axis);
      client.print(barrier);
      client.print(cutup);
      barrier = 'n';
    }
    
    //client.stop();
    //Serial.println("Client disconnected");
    // digitalWrite(LED_BUILTIN, LOW);
  }
}


long detectDistance(int pingPin){
  long duration, cm;
  // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  pinMode(pingPin, OUTPUT);
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pingPin, LOW);

  // The same pin is used to read the signal from the PING))): a HIGH pulse
  // whose duration is the time (in microseconds) from the sending of the ping
  // to the reception of its echo off of an object.
  pinMode(pingPin, INPUT);
  duration = pulseIn(pingPin, HIGH);

  // convert the time into a distance
  cm = microsecondsToCentimeters(duration);
  return cm;
}


long microsecondsToCentimeters(long microseconds) {
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the object we
  // take half of the distance travelled.
  return microseconds / 29 / 2;
}


void setupADXL()
{
  Wire.setPins(PIN_ADXL_SDA, PIN_ADXL_SCL);  // 自定義SDA以及SCL的pin腳(必須要是GPIO:General Purpose)
  Wire.begin();                              // join I2C bus as a slave with address 0x12
  Wire.beginTransmission(ADXAddress);
  Wire.write(Register_2D);
  Wire.write(8);                //measuring enable
  Wire.endTransmission();       // stop transmitting
}

void showData(){
  
  Serial.print("X= ");
  Serial.print(Xg);
  Serial.print(" ");
  Serial.print("Y= ");
  Serial.print(Yg);
  Serial.print(" ");
  Serial.print("Z= ");
  Serial.println(Zg);  
  /*
  Serial.print("Roll(X、Z)= ");
  Serial.print(Roll);
  Serial.print(" ");
  Serial.print("Pitch(Y)= ");
  Serial.print(Pitch);
  Serial.println(" ");
  */
}


void detectCarAngle(){
  int X0,X1,X_out;
  int Y0,Y1,Y_out;
  int Z1,Z0,Z_out;
  //--------------X
  Wire.beginTransmission(ADXAddress); // transmit to device
  Wire.write(Register_X0);
  Wire.write(Register_X1);
  Wire.endTransmission();
  Wire.requestFrom(ADXAddress,2);
  if(Wire.available()<=2)
  {
    X0 = Wire.read();
    X1 = Wire.read();
    X1=X1<<8;
    X_out=X0+X1;
  }
  //------------------Y
  Wire.beginTransmission(ADXAddress); // transmit to device
  Wire.write(Register_Y0);
  Wire.write(Register_Y1);
  Wire.endTransmission();
  Wire.requestFrom(ADXAddress,2);
  if(Wire.available()<=2)
  {
    Y0 = Wire.read();
    Y1 = Wire.read();
    Y1=Y1<<8;
    Y_out=Y0+Y1;
  }
  //------------------Z
  Wire.beginTransmission(ADXAddress); // transmit to device
  Wire.write(Register_Z0);
  Wire.write(Register_Z1);
  Wire.endTransmission();
  Wire.requestFrom(ADXAddress,2);
  if(Wire.available()<=2)
  {
    Z0 = Wire.read();
    Z1 = Wire.read();
    Z1=Z1<<8;
    Z_out=Z0+Z1;
  }
  //----------------
  Xg=X_out/256.0;
  Yg=Y_out/256.0;
  Zg=Z_out/256.0;
  Roll = atan2(Yg , Zg) * 57.3;
  Pitch = atan2((- Xg) , sqrt(Yg * Yg + Zg * Zg)) * 57.3;
}
