#include <EnableInterrupt.h>
#include "PololuLedStrip.h"

const int stepsPerRevolution = 400;  // number of steps per revolution

#define INPUT_CAPTEUR   8
#define OUTPUT_COM      7
#define MOT_STEPPER     5

PololuLedStrip<OUTPUT_COM> ledStrip;

#define LED_COUNT 28
rgb_color colors[LED_COUNT];

unsigned char   Step, Sector, NbTours, MemoNbTours;
unsigned int    Delay_Inter_Step;
unsigned int    Delay_Inter_Step_Max = 1000;    // delay entre chaque step en µs 
unsigned int    Delay_Inter_Step_Min = 300;    // delay entre chaque step en µs 

// Définition des interruptions
void Capteur_Interrupt()
{
    InitPos = True;
}

void setup()
{
    // Configure inputs
    pinMode(INPUT_CAPTEUR, INPUT);

    // Configure outputs
    pinMode(OUTPUT_COM, OUTPUT);

    // Initialise la liaison série à 115200 bauds
    Serial.begin(115200);

    // Configure interruptions
    enableInterrupt(INPUT_CAPTEUR, Capteur_Interrupt, FALLING);

    pinMode(MOT_STEPPER, OUTPUT);

    Delay_Inter_Step = Delay_Inter_Step_Max;
}

void loop()
{   
    if (InitPos == False)
    {
        digitalWrite(MOT_STEPPER, HIGH);
        digitalWrite(MOT_STEPPER, LOW);
    
        delayMicroseconds(1200);
    }
    else
    {   
        unsigned char   temp = NbTours % 4;
        unsigned int    i;
        rgb_color color;
    
        if (NbTours != MemoNbTours)
        {
            MemoNbTours = NbTours;
            switch (temp)
            {
                case 0:
                {
                    color.red = 0;
                    color.green = 0;
                    color.blue = 0;
                }
                break;
                case 1:
                {
                    color.red = 255;
                    color.green = 0;
                    color.blue = 0;
                }
                break;
                case 2:
                {
                    color.red = 0;
                    color.green = 255;
                    color.blue = 0;
                }
                break;
                case 3:
                {
                    color.red = 0;
                    color.green = 0;
                    color.blue = 255;
                }
                break;
                default:
                {
                    color.red = 0;
                    color.green = 0;
                    color.blue = 0;
                }
                break;
            }
            //myStepper.onestep(1);
            
            // Update the colors buffer.
            for(i = 0; i < LED_COUNT; i++)
            {
                colors[i] = color;
            }
        }
        
        ledStrip.write(colors, LED_COUNT);
    
        digitalWrite(MOT_STEPPER, HIGH);
        digitalWrite(MOT_STEPPER, LOW);
        
        if (Delay_Inter_Step)   {   delayMicroseconds(Delay_Inter_Step);   }
    
        if (Delay_Inter_Step > Delay_Inter_Step_Min)    {   Delay_Inter_Step --;    }
        
        if (++ Sector >= 100)  
        {
            Sector =0;
            NbTours ++;
        }
    } 
}

