#include <EnableInterrupt.h>
//#include "Stepper.h"

#define INPUT_CAPTEUR   8
#define MOT_STEPPER     5

// initialize the stepper library on pins 2 through 5:
//Stepper stepper(stepsPerRevolution, 2, 3, 4, 5);

unsigned int    Value;
unsigned int    Value1 = 3;
unsigned int    Value_Max = 2000;
unsigned int    Value_Min = 300;
bool            Top = false;

void Capteur_Interrupt()
{
    Top = true;
}

void setup() {
    // Configure inputs
    pinMode(INPUT_CAPTEUR, INPUT);

    // Configure interruptions
    enableInterrupt(INPUT_CAPTEUR, Capteur_Interrupt, RISING);

    pinMode(MOT_STEPPER, OUTPUT);

    Value = Value_Max;
}

void loop() {

    if (Top == true)
    {
        digitalWrite(MOT_STEPPER, HIGH);
        delayMicroseconds(Value1);
        digitalWrite(MOT_STEPPER, LOW);

        delayMicroseconds(Value);
        
        digitalWrite(MOT_STEPPER, HIGH);
        delayMicroseconds(Value1);
        digitalWrite(MOT_STEPPER, LOW);
        
        delayMicroseconds(Value);
        
        digitalWrite(MOT_STEPPER, HIGH);
        delayMicroseconds(Value1);
        digitalWrite(MOT_STEPPER, LOW);
        
        delayMicroseconds(Value);
        
        digitalWrite(MOT_STEPPER, HIGH);
        delayMicroseconds(Value1);
        digitalWrite(MOT_STEPPER, LOW);

        Top = false;
        
        if (Value > Value_Min) {    Value --;   }
    }
}
