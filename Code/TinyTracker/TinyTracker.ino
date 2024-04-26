#include "include/Functions.h"
#include "include/TMC2300.h"
#include "include/CRC.h"
#include "Adafruit_TinyUSB.h"
#include "nRF52_PWM.h"

// Important these are the I/O pin values not the physical pin values
#define LED_STATUS LED_BUILTIN 
#define MOTOR_EN 8 
#define MOTOR_STEP 9
#define MOTOR_DIR 10
#define SW1 0
#define SW2 1
#define LSR 2
#define DIM 3
#define _PWM_LOGLEVEL_       1
#define USING_TIMER       false   //true
#define pinToUse       MOTOR_STEP

//creates pwm instance
nRF52_PWM* PWM_Instance;
float frequency = 23720.0f;
float dutyCycle = 50.0f;
int ledState = LOW;        // the current state of the output pin
int buttonState;            // the current reading from the input pin
int lastButtonState = LOW;  // the previous reading from the input pin
int speed = LOW;            // the motor speed
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
unsigned long pressDelay = 1500;    // the long press delay time
unsigned long timeout = 15000;    // auto turn off time
unsigned long ontime = 0;    // last time the light was turned on


void tmc2300_current(int currentInt)   
{    
  uint32_t value = 1 << TMC2300_IHOLDDELAY_SHIFT    
                 | ((currentInt << TMC2300_IRUN_SHIFT) & TMC2300_IRUN_MASK)    
                 | 8 << TMC2300_IHOLD_SHIFT;    
  tmc2300_writeInt(TMC2300_IHOLD_IRUN, value);  
}    

void setup() {
  Serial.begin(9600);

  pinMode(LED_STATUS, OUTPUT);
  pinMode(MOTOR_EN, OUTPUT);
  pinMode(MOTOR_STEP,OUTPUT);    
  pinMode(MOTOR_DIR,OUTPUT);  
  pinMode(SW2,INPUT_PULLUP);  

// Disable the motor
  digitalWrite(MOTOR_EN, LOW);   
  digitalWrite(MOTOR_STEP,LOW);  
  digitalWrite(MOTOR_DIR,digitalRead(SW2));    

// TMC2300 IC UART connection
  Serial1.begin(115200);
  delay(1000);

// Initialize CRC calculation for TMC2300 UART datagrams
  tmc_fillCRC8Table(0x07, true, 0);
  tmc2300_writeInt(TMC2300_CHOPCONF, 0x13008001);  // Re-write the CHOPCONF back to the default
  tmc2300_writeInt(TMC2300_PWMCONF, 0xC40F1024);
  tmc2300_current(31);
  digitalWrite(MOTOR_EN, HIGH);   


//assigns PWM frequency of 1.0 KHz and a duty cycle of 0%
  PWM_Instance = new nRF52_PWM(pinToUse, frequency, dutyCycle);
  
  pinMode(SW1, INPUT_PULLUP);
  pinMode(LSR, OUTPUT);
  pinMode(DIM, OUTPUT);
  digitalWrite(LSR, ledState);
  digitalWrite(DIM, HIGH);  

}

void loop() {
  // read the state of the switch into a local variable:
  int reading = !digitalRead(SW1);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();      
    }


  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // only toggle the LED if the new button state is HIGH
      if (buttonState == HIGH) {
        ledState = !ledState;
        ontime = millis();
      }
    }

  }

  // set the LED:
  if ((millis()-ontime) > timeout){
    ledState = false;
  }
  
  digitalWrite(LSR, ledState);

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;
}
