
#include <EnableInterrupt.h>
#include <PololuLedStrip.h>

#define INPUT_CAPTEUR   4
#define OUTPUT_COM      7

PololuLedStrip<OUTPUT_COM> ledStrip;

#define LED_COUNT 28
rgb_color colors[LED_COUNT];

#define TEMPS_COMPTAGE  2000    // en ms
#define NB_SECTOR       36

#define CONVERT_us_PAR_SECTOR(v)    (unsigned long) ((float) (TEMPS_COMPTAGE * 1000.0f) / (float) (v * NB_SECTOR))
#define CONVERT_TR_PAR_MIN(v)       (float) ((float) v * 1000.0f * 60.0f / TEMPS_COMPTAGE)

volatile unsigned char  Count;
unsigned char   MemoCount;
unsigned char   NbTours;
unsigned long   Count_ms;
unsigned long   Count_us;
unsigned long   Time_By_Sector; // en µs

unsigned int    Sector_En_Cours;

// Définition des interruptions
void Capteur_Interrupt()
{
    Count ++;
    Sector_En_Cours = 0;
}

void setup()
{
    // Configure inputs
    pinMode(INPUT_CAPTEUR, INPUT);

    // Configure interruptions
    enableInterrupt(INPUT_CAPTEUR, Capteur_Interrupt, RISING);

    // Configure outputs
    pinMode(OUTPUT_COM, OUTPUT);
}

void Calcul_Time_By_Sector()
{
    NbTours = (Count - MemoCount);

    // Optionnel affiche la vitesse en Tr/min
    Serial.print("Speed: ");
    Serial.print(CONVERT_TR_PAR_MIN(NbTours));
    Serial.println("Tr/min \n");
    
    Time_By_Sector = CONVERT_us_PAR_SECTOR(NbTours);
    
    MemoCount = Count;
}

void loop()
{   
    unsigned int i;
    rgb_color color;
          
    if ((millis() - Count_ms) > TEMPS_COMPTAGE)
    {
        Count_ms = millis();

        Calcul_Time_By_Sector();
    }

    if ((micros() - Count_us) > Time_By_Sector)
    {
        Count_us = micros();

        i = Sector_En_Cours % 4;

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
        }

        // Update the colors buffer.
        for(i = 0; i < LED_COUNT; i++)
        {
            colors[i] = color;
        }
        
        ledStrip.write(colors, LED_COUNT);
        
        Sector_En_Cours ++;
    }
}
