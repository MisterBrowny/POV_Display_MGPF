#include <EnableInterrupt.h>
#include <SPI.h>
#include "PololuLedStrip.h"

const int stepsPerRevolution = 400;  // number of steps per revolution

#define INPUT_CAPTEUR   8
#define OUTPUT_COM      7
#define MOT_STEPPER     5

PololuLedStrip<OUTPUT_COM> ledStrip;

#define LED_COUNT 28
rgb_color colors[LED_COUNT];

unsigned char   Step, Sector, NbTours, MemoNbTours;
unsigned int    Delay_Inter_Step, Cpt;
unsigned int    Delay_Inter_Step_Max = 2000;    // delay entre chaque step en µs 
unsigned int    Delay_Inter_Step_Min = 860;    // delay entre chaque step en µs 
bool            InitPos;


#define         NB_LED_DISPLAY      28
#define         NB_BYTE_PAR_LED     3
#define			NB_DATAS			(NB_LED_DISPLAY * NB_BYTE_PAR_LED)

#define         SPI_TIME_OUT        20 //ms

unsigned long   SPI_Rcv_Time;
unsigned char   SPI_led_number, SPI_color, data[28*3], memodata;
rgb_color       SPI_colors[NB_LED_DISPLAY];
bool            Write, timeOut;

// Définition des interruptions
void Capteur_Interrupt()
{
    InitPos = true;
    //Sector = 0;
}

ISR (SPI_STC_vect)
{
    byte c = SPDR;  // grab byte from SPI Data Register
  	//div_t	temp = div(Cpt,3);
  	
    // add to buffer if room
    /*if (SPI_color == 0)         {   SPI_colors[SPI_led_number].red = c;     }
    else if (SPI_color == 1)    {   SPI_colors[SPI_led_number].green = c;   }
    else                        {   SPI_colors[SPI_led_number].blue = c;    }
    
    if (++ SPI_color >= NB_BYTE_PAR_LED)    
    {  
        SPI_color = 0;
        SPI_led_number ++;
        Serial.print("color received : ");
        Serial.println(SPI_led_number);
    }
    if (SPI_led_number >= NB_LED_DISPLAY)   
    {
        SPI_led_number = 0;
        Write = true;
        Serial.println("toutes les color received");       
    }
    
    SPI_Rcv_Time = micros();
    */
    //if (temp.rem == 0) 			{	SPI_colors[temp.quot].red = c;		}
    //else if (temp.rem == 1) 	{	SPI_colors[temp.quot].green = c;	}
    //else						{	SPI_colors[temp.quot].blue = c;		}
    
    data[Cpt] = c;
    if (++ Cpt >= NB_DATAS)
    { 
    	Cpt = 0;
    	Write = true;
    }

    SPI_Rcv_Time = millis();
    timeOut = false;
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

    // turn on SPI in slave mode
    SPCR |= bit(SPE);

    // now turn on interrupts
    SPI.attachInterrupt();    //idem : SPCR |= _BV(SPIE);
}

void loop()
{   
	unsigned int    i;
	
    if (Write == true)
    {
    	digitalWrite(MOT_STEPPER, HIGH);
		/*Serial.println("début trame");
		for(i = 0; i < NB_DATAS; i++)
        {
        	Serial.println(data[i], DEC);
        }
        Serial.println("fin trame");*/
		memcpy(SPI_colors, data, NB_DATAS);
        // Update the colors buffer.
        ledStrip.write(SPI_colors, LED_COUNT);
        
        delayMicroseconds(50);
        
        digitalWrite(MOT_STEPPER, LOW);
        Write = false;
	}
    else if (timeOut == false)
	{
		if ((millis() - SPI_Rcv_Time) > SPI_TIME_OUT)
		{
	        if (Cpt != 0)
	        {
	        	Cpt = 0;
	        	timeOut = true;
	        	Serial.println("time out");
	        }
	    }
	}
	
    /*if (InitPos == false)
    {
        digitalWrite(MOT_STEPPER, HIGH);
        digitalWrite(MOT_STEPPER, LOW);
    
        delayMicroseconds(2000);
    }
    else*/
    //if (Write == true)
    //{   
    //    
        
        /*unsigned char   temp = Sector % 4;
        rgb_color color;
    
        if (NbTours != MemoNbTours)
        {
            MemoNbTours = NbTours;
            switch (temp)
            {
                case 0:
                {
                    color.red = 100;
                    color.green = 100;
                    color.blue = 100;
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
            // Update the colors buffer.
            for(i = 0; i < LED_COUNT; i++)
            {
                colors[i] = color;
            }
        }*/

        /*Serial.println("début trame");
        
        // Update the colors buffer.
        for(i = 0; i < LED_COUNT; i++)
        {
            colors[i] = SPI_colors[i];
            Serial.print(SPI_colors[i].red);
            Serial.print(',');
            Serial.print(SPI_colors[i].green);
            Serial.print(',');
            Serial.println(SPI_colors[i].blue);
        }
        Write = false;
        Serial.println("fin trame");
        
        //ledStrip.write(colors, LED_COUNT);
        //delayMicroseconds(50);
		*/
        
        /*digitalWrite(MOT_STEPPER, HIGH);
        
        if (Delay_Inter_Step)   {   delayMicroseconds(Delay_Inter_Step);   }
    
        if (Delay_Inter_Step > Delay_Inter_Step_Min)    {   Delay_Inter_Step --;    }
        
        if (++ Sector >= 100)  
        {
            Sector = 0;
            NbTours ++;
        }
        
        digitalWrite(MOT_STEPPER, LOW);*/
    //}

    /*if ((micros() - SPI_Rcv_Time) > SPI_TIME_OUT)
    {
        SPI_color = 0;
        SPI_led_number = 0;
        Serial.println("time out");
    }*/
}

