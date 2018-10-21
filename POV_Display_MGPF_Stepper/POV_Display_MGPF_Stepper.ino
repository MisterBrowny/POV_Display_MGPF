#include <EnableInterrupt.h>
//#include "Stepper.h"

#define INPUT_CAPTEUR   8
#define MOT_STEPPER     5

// initialize the stepper library on pins 2 through 5:
//Stepper stepper(stepsPerRevolution, 2, 3, 4, 5);

unsigned int    Value = 300;

void Capteur_Interrupt()
{
    Top = True;
}

void setup() {
    // Configure inputs
    pinMode(INPUT_CAPTEUR, INPUT);

    // Configure interruptions
    enableInterrupt(INPUT_CAPTEUR, Capteur_Interrupt, RISING);

    pinMode(MOT_STEPPER, OUTPUT);
}

void loop() {

    if (Top == True)
    {
        digitalWrite(MOT_STEPPER, HIGH);
        digitalWrite(MOT_STEPPER, LOW);

        delayMicroseconds(Value);
        
        digitalWrite(MOT_STEPPER, HIGH);
        digitalWrite(MOT_STEPPER, LOW);
        
        delayMicroseconds(Value);
        
        digitalWrite(MOT_STEPPER, HIGH);
        digitalWrite(MOT_STEPPER, LOW);
        
        delayMicroseconds(Value);
        
        digitalWrite(MOT_STEPPER, HIGH);
        digitalWrite(MOT_STEPPER, LOW);

        Top = False;
    }
    
}
