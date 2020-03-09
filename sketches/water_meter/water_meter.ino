/* 
Water meter

   Author :             F. Guiet 
   Creation           : 20200103
   Last modification  : 20200126
  
  Version            : 1.0
  History            : 1.0 - First version
                       1.1 - Reduce message transmitted so it use only one packet!
                       2.0 - New version
                       2.1 - Improve algorithm
                       2.2 - Water sensor is reporting liter consumption ok!
                       2.3 - Add Low Power consumption stuff
                       2.4 - Fix issue with liter consumed
                       2.5 - Remove Low Power Consumption....reading are bad...
                       2.6 - Add alive blink
                       
References :

   - http://bricolsec.canalblog.com/archives/2010/09/15/19069451.html
   - http://www.home-automation-community.com/arduino-low-power-how-to-run-atmega328p-for-a-year-on-coin-cell-battery/

Installing Board Arduino Pro Micro Board

  - https://raw.githubusercontent.com/sparkfun/Arduino_Boards/master/IDE_Board_Manager/package_sparkfun_index.json
  - https://learn.sparkfun.com/tutorials/pro-micro--fio-v3-hookup-guide/installing-windows

Sur les cartes Arduino Pro Micro si problème lors du trasnfert d'un sketch la première fois, reinitialiser le bootloader avec le tuto ci-dessous

  - https://www.instructables.com/id/Burn-Bootloader-Arduino-Nano-As-ISP-to-Pro-Micro/

E32-TTL-100 Byte

  - Careful !! => The datasheet shows the module can be powered up to 5.2V but the data pins are only rated up to !!! 3.6V !!!
  - Datasheet : https://github.com/ccadic/E45TTL100-868MHZ/blob/master/E45-TTL-100_Datasheet_EN_v1.0.pdf
  - Note from datasheet : for some MCU works at 5VDC, it may need to add 4-10k pull-up resistor for the TXD & AUX pin
  - 512 bytes buffer (58 bytes per package)

Arduino Pro Micro Datasheet
  - https://github.com/sparkfun/Pro_Micro/blob/master/Documentation/ProMicro8MHzv2.pdf
  - Nice Ref about low power consumption : https://andreasrohner.at/posts/Electronics/How-to-modify-an-Arduino-Pro-Mini-clone-for-low-power-consumption/
  - LG33 voltage regulator : https://www.google.com/url?sa=t&rct=j&q=&esrc=s&source=web&cd=1&ved=2ahUKEwj7xJu8yoroAhUGDmMBHb58ChYQFjAAegQIARAB&url=https%3A%2F%2Fcdn-shop.adafruit.com%2Fproduct-files%2F3081%2Fmic5219.pdf&usg=AOvVaw01f0t1pEEkpVxkZzFphFFY

Consumption (without Power Led (add 3mAh with Power Led)) :
  -  RAW PIN (use 3.3v Arduino regulator) : 50mA (39mA idle mode) 
  -  VCC PIN (do not use 3.3v Arduino regulator, so be CAREFUL TX pin is at VCC and is not compatible with TX pin of E32-TTL-100 LoRa Node) : 50mA (39mA idle mode)

Careful :
   - Interrupt pin should not receive more than 3.3v...

HIGH will be report if voltage >= 2.0v
   https://www.arduino.cc/reference/en/language/variables/constants/constants/

*/

#include <ArduinoJson.h>
//#include <SoftwareSerial.h>
//#include <LowPower.h>

#define DEBUG 0
#define FIRMWARE_VERSION "2.6"

const int LED_PIN = 2;
const int INTERRUPT_PIN = 3; //Comes from sensor
const int M0_PIN = 4;
const int M1_PIN = 5;

const int SENSOR_PIN = A0;
const int VOLTAGE_PIN = A1;


int messageToSendCounter = 0;

volatile bool rising = true;
volatile bool literConsumed = false;
unsigned long literConsumedFromStart = 0;
unsigned int literConsumedCounter = 0;

unsigned long startTime = millis();
unsigned long idleTime = millis();
unsigned long blinkTime = millis();

const unsigned long interval = 5UL*1000UL;   //5s
const unsigned long blinkInterval = 10UL*1000UL;   //60s
const unsigned long idleInterval = 60UL * 1000UL * 60UL;  //1h

struct Sensor {
    String Name;    
    String SensorId;
};

#define SENSORS_COUNT 1
Sensor sensors[SENSORS_COUNT];

//SoftwareSerial mySerial(10, 16); // RX, TX

void setup() {

  analogReference(INTERNAL);

  //To monitor analog input voltage
  pinMode(SENSOR_PIN, INPUT);
  pinMode(VOLTAGE_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  pinMode(INTERRUPT_PIN, INPUT);

  //LoRa transceiver
  pinMode(M0_PIN, OUTPUT);    
  pinMode(M1_PIN, OUTPUT);  

  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN),OnRisingChange, RISING);

  // Initialize Serial Port 1 (from RX, TX pin), use by LoRa Transceiver
  Serial1.begin(9600);
  
  // Initialize Serial Port
  if (DEBUG) {
    Serial.begin(115200);
    //mySerial.begin(115200);
  }

  digitalWrite(M0_PIN, HIGH); //
  digitalWrite(M1_PIN, HIGH); //High = Sleep mode

  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);
  delay(100);
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);
  delay(100);

  InitSensors();
 
  debug_message("Setup completed, starting...",true);  
}

void OnRisingChange() {
   literConsumed = true;          
}

void loop() {

  /*int intPinValue = digitalRead(INTERRUPT_PIN);
  debug_message("Interrupt Pin Status :  " + String(intPinValue), true);    
  
  int inputVoltage = analogRead(SENSOR_PIN);  
  debug_message("Input Voltage :  " + String(inputVoltage), true);  
  

  delay(200);

  return;
  */
  /*
  
  if (cybleDetected) {
    debug_message("Cyble détecté : OUI", true);
  }
  else {
    debug_message("Cyble détecté : NON", true);
  } */
  
  if (literConsumed) {    

    //Check PIN HIGH Three times !
    int cybleDetectionCounter = 1;
    //then check if value is really high, if not maybe it's a false trigger...
    int intPinValue = digitalRead(INTERRUPT_PIN);
    while (intPinValue == HIGH && cybleDetectionCounter <= 3) {
      intPinValue = digitalRead(INTERRUPT_PIN);
      cybleDetectionCounter++;
      delay(100);
    }

    //cybleDetectionCounter equals 4 here if ok
    if (cybleDetectionCounter > 3) {
      literConsumedFromStart++;
      literConsumedCounter++;      
      digitalWrite(LED_PIN, HIGH);
      delay(100);
      digitalWrite(LED_PIN, LOW);
      delay(100); 
    }    
    
    //delay(500); //Ignore bounce for 500s + 500s (1s) - v2.1 => compte un peu trop faible
    delay(250); //v2.2 - Delay ok !!
    literConsumed = false;
  }

  //Wait 5 secondes before sending another message (in order not to flood the system in case someone is taking
  //a bath and is consumiming a lot of water)
  if (literConsumedCounter > 0 && (millis() - startTime) > interval) {    
    startTime = millis();
    idleTime = millis();
    sendMessage(String(literConsumedCounter));    
    literConsumedCounter--;    
    //delay(500); Removed in v2.1
  }  

  //Send battery voltage every hour
  if ((millis() - idleTime) > idleInterval || idleTime > millis()) {
    startTime = millis();
    idleTime = millis();
    sendMessage("0");
  }

  //Alive blink!
  if ((millis() - blinkTime) > blinkInterval || blinkTime > millis()) {
    blinkTime = millis();
    
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);    
  }

  /*

  //No activity during 30s...ok time to hibernate
  if ((millis() - idleTime) > idleInterval) {

    //60*60 / 8 = 450 = publication toutes les heures!
    for (int i=1; i<=450;i++) {
    
      LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);

      //reset idle time and start time
      idleTime = millis();
      startTime = millis();
      
      //Wake up by water consumption?
      if (literConsumed) {
        //yes, then we leave the loop
        break;
      }
    }  

    //ok you left the hibernation but for what reason?
    if (!literConsumed) {
      //send battery voltage information every one hour
      sendMessage("0");
    }
  }  
  */
  
  delay(50);
}

void InitSensors() {
  
  sensors[0].Name = "WM";
  sensors[0].SensorId = "20";
}

float ReadVoltage() {

  //AnalogRead = 307 pour 4.2v

  //R1 = 33kOhm
  //R2 = 7.5kOhm

  float sensorValue = 0.0f;  
  float vin = 0.f;

  delay(100); //Tempo so analog reading will be correct!
  sensorValue = analogRead(VOLTAGE_PIN);
  //analog_vcc = sensorValue;
  
  debug_message("Analog Reading : " + String(sensorValue,2), true);

  return (sensorValue * 4.2) / 307;

}

void debug_message(String message, bool doReturnLine) {
  if (DEBUG) {
    if (doReturnLine) {
      //mySerial.println(message);
      Serial.println(message);
    }
    else {
      //mySerial.println(message);
      Serial.print(message);
    }
  }
}

String ConvertToJSon(String battery, String liter) {
    //Create JSon object
    DynamicJsonDocument  jsonBuffer(200);
    JsonObject root = jsonBuffer.to<JsonObject>();
    
    root["id"] = sensors[0].SensorId;
    root["n"] = sensors[0].Name;
    root["f"]  = FIRMWARE_VERSION;
    root["b"] = battery;
    root["l"] = liter;    
    root["cft"] = String(literConsumedFromStart);
    
    String result;    
    serializeJson(root, result);

    return result;
}

void sendMessage(String liter) {
  
  digitalWrite(M0_PIN, LOW); //
  digitalWrite(M1_PIN, LOW); //low, low = normal mode
 
  float vin = ReadVoltage();  
  
  String message = ConvertToJSon(String(vin,2), liter);
  
  debug_message("Message envoyé : " + message, true);
  
  Serial1.print(message);
  
  Serial1.end();  
  delay(30);
  Serial1.begin(9600);
  delay(70); //The rest of requested delay. So 100 - 30 = 70

  //LoRa in Sleep mode
  digitalWrite(M0_PIN, HIGH); //
  digitalWrite(M1_PIN, HIGH); //high, high sleep mode
}
