#include <EnableInterrupt.h>
#include "PololuLedStrip.h"
#include "TimerOne.h"

#define INPUT_CAPTEUR   4
#define OUTPUT_COM      7

PololuLedStrip<OUTPUT_COM> ledStrip;

#define LED_COUNT 28
rgb_color colors[LED_COUNT];


#define NB_SECTOR       2

// Pour le calcul à l'aide du nb de tours sur un temps donné
//#define TEMPS_COMPTAGE  10000    // en ms
//#define CONVERT_us_PAR_SECTOR(v)    (unsigned long) ((float) (TEMPS_COMPTAGE * 1000.0f) / (float) (v * NB_SECTOR))
//#define CONVERT_TR_PAR_MIN(v)       (float) ((float) v * 1000.0f * 60.0f / TEMPS_COMPTAGE)

// Pour le calcul à partir du temps d'un tour
#define CONVERT_TR_PAR_MIN(v)       (float) (1000000.0f * 60.0f / v)
#define CONVERT_us_PAR_SECTOR(v)    (unsigned long) (v / NB_SECTOR)

volatile unsigned char  Count;
unsigned char   MemoCount;
unsigned char   NbTours;
unsigned long   Count_ms;
unsigned long   Count_us;
unsigned long   MemoCount_us;
unsigned long   Time_By_Sector; // en µs
bool            NewTurn = false;
bool            Running = false;
unsigned int    Sector_En_Cours;

// Définition des interruptions
void Capteur_Interrupt()
{
    Timer1.stop();
    Sector_En_Cours = 0;
    NewTurn = true;
    Running = true;
}

void Sector_Interrupt()
{
    if (++ Sector_En_Cours >= NB_SECTOR)    {   Running = false;    }
}

void setup()
{
    // Configure inputs
    pinMode(INPUT_CAPTEUR, INPUT);

    // Configure outputs
    pinMode(OUTPUT_COM, OUTPUT);

    // Initialise la liaison série à 9600 bauds
    Serial.begin(115200);

    // Configure interruptions
    enableInterrupt(INPUT_CAPTEUR, Capteur_Interrupt, FALLING);

    Timer1.initialize(); // set a timer of length xxx microseconds
    Timer1.attachInterrupt(Sector_Interrupt);
    Timer1.stop();
}

void Calcul_Time_By_Sector()
{
    Count_us = micros() - MemoCount_us;
    MemoCount_us = micros();
        
    Time_By_Sector = CONVERT_us_PAR_SECTOR(Count_us);
    
    Timer1.setPeriod(Time_By_Sector);
    Timer1.start();

    /*
    // Optionnel affiche le nombre de tours
    Serial.print("1 tours en ");
    Serial.print(Count_us);
    Serial.println("µs");
    
    // Optionnel affiche la vitesse en Tr/min
    Serial.print("Speed: ");
    Serial.print(CONVERT_TR_PAR_MIN(Count_us));
    Serial.println("Tr/min");
    
    // Optionnel affiche le temps attribué par secteurs
    Serial.print("Temps par secteur : ");
    Serial.println(Time_By_Sector);
    */   
}

void loop()
{   
    unsigned int i;
    rgb_color color;
          
    if (NewTurn == true)
    {
        NewTurn = false;
        Calcul_Time_By_Sector();
    }
    
    if (    (Running == true)
        &&  (Sector_En_Cours != Memo_Sector_En_Cours))
    {
        i = Sector_En_Cours % 2;

        switch (i)
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
            default:
            {
                color.red = 0;
                color.green = 0;
                color.blue = 0;
            }
            break;
            }
        }
        // Update the colors buffer.
        for(i = 0; i < LED_COUNT; i++)
        {
            colors[i] = color;
        }

        ledStrip.write(colors, LED_COUNT);
     }
}

/*
case 3:
{
    color.red = 255;
    color.green = 255;
    color.blue = 0;
}
break;
case 4:
{
    color.red = 127;
    color.green = 255;
    color.blue = 0;
}
break;
case 5:
{
    color.red = 0;
    color.green = 255;
    color.blue = 0;
}
break;
case 6:
{
    color.red = 0;
    color.green = 255;
    color.blue = 127;
}
break;
case 7:
{
    color.red = 0;
    color.green = 255;
    color.blue = 255;
}
break;
case 8:
{
    color.red = 0;
    color.green = 127;
    color.blue = 255;
}
break;
case 9:
{
    color.red = 0;
    color.green = 0;
    color.blue = 255;
}
break;
case 10:
{
    color.red = 127;
    color.green = 0;
    color.blue = 255;
}
break;
case 11:
{
    color.red = 255;
    color.green = 0;
    color.blue = 255;
}
break;
case 12:
{
    color.red = 255;
    color.green = 0;
    color.blue = 127;
}
break;
*/
