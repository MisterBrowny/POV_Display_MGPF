#include <EnableInterrupt.h>
#include <SPI.h>
#include <TimerOne.h>
#include "PololuLedStrip.h"

const int stepsPerRevolution = 400;	// number of steps per revolution

#define INPUT_CAPTEUR	8
#define OUTPUT_COM		7
#define MOT_STEPPER		5

PololuLedStrip<OUTPUT_COM> ledStrip;

#define 		LED_COUNT 			28
#define			NB_LED_DISPLAY		28
#define			NB_BYTE_PAR_LED		3
#define			NB_DATAS_2			(unsigned int) (NB_LED_DISPLAY * NB_BYTE_PAR_LED)
#define			NB_DATAS			483

bool			Write;
byte			Sector, MemoSector, data[NB_DATAS], rcv_data[NB_DATAS];
unsigned int	Cpt;

rgb_color		SPI_colors[LED_COUNT];


// Définition des interruptions
void Capteur_Interrupt()
{
	Sector = 0;
}

ISR (SPI_STC_vect)
{
	// Récupére la data dans le buffer de reception
	rcv_data[Cpt] = SPDR;

	// Nombre de datas max atteint
	if (++ Cpt >= NB_DATAS_2)
	{ 
		Cpt = 0;
		Write = true;
	}
}

void Control_Stepper(void)
{
	if (digitalRead(MOT_STEPPER))
	{
		digitalWrite(MOT_STEPPER, LOW);
	}
	else
	{
		digitalWrite(MOT_STEPPER, HIGH);
		if (++ Sector >= 100)  
		{
			Sector = 0;
		}
	}
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

	memset(data, 1, NB_DATAS_2);
	
	// turn on SPI in slave mode
	SPCR |= bit(SPE);

	// now turn on interrupts
	SPI.attachInterrupt();    //idem : SPCR |= _BV(SPIE);

	Timer1.initialize(2000);
	Timer1.attachInterrupt(Control_Stepper);
}

void loop()
{   
	unsigned char    i,j;
	rgb_color color;
	
	if (Write == true)
	{
		bitClear(SPCR, SPIE);
		memcpy(data, rcv_data, NB_DATAS_2);
		for(i = 0; i < NB_DATAS_2; i++)
		{
			Serial.print(data[i]);
		}
		Serial.println("fin trame");
		Write = false;
		bitSet(SPCR, SPIE);
	}
	
	if (Sector != MemoSector)
	{
		bitClear(SPCR, SPIE);
		MemoSector = Sector;

		/*// Group8 - led [28-26] - Data[130 - 161] - 32 pixels
		// 4 - 3 - 3 - 3 - 3 - 3 - 3 - 3 - ...
		for (i = 0; i < 3; i ++)
		{
			if (Sector < 4)				{	j = 0;	}
			else if (Sector < 7)		{	j = 1;	}
			else if (Sector < 10)		{	j = 2;	}
			else if (Sector < 13)		{	j = 3;	}
			else if (Sector < 16)		{	j = 4;	}
			else if (Sector < 19)		{	j = 5;	}
			else if (Sector < 22)		{	j = 6;	}
			else if (Sector < 25)		{	j = 7;	}
			else if (Sector < 29)		{	j = 8;	}
			else if (Sector < 32)		{	j = 9;	}
			else if (Sector < 35)		{	j = 10;	}
			else if (Sector < 38)		{	j = 11;	}
			else if (Sector < 41)		{	j = 12;	}
			else if (Sector < 44)		{	j = 13;	}
			else if (Sector < 47)		{	j = 14;	}
			else if (Sector < 50)		{	j = 15;	}
			else if (Sector < 54)		{	j = 16;	}
			else if (Sector < 57)		{	j = 17;	}
			else if (Sector < 60)		{	j = 18;	}
			else if (Sector < 63)		{	j = 19;	}
			else if (Sector < 66)		{	j = 20;	}
			else if (Sector < 69)		{	j = 21;	}
			else if (Sector < 72)		{	j = 22;	}
			else if (Sector < 75)		{	j = 23;	}
			else if (Sector < 79)		{	j = 24;	}
			else if (Sector < 82)		{	j = 25;	}
			else if (Sector < 85)		{	j = 26;	}
			else if (Sector < 88)		{	j = 27;	}
			else if (Sector < 91)		{	j = 28;	}
			else if (Sector < 94)		{	j = 29;	}
			else if (Sector < 97)		{	j = 30;	}
			else 						{	j = 31;	}
			
			SPI_colors[i].red = data[387 + 3*j];
			SPI_colors[i].green = data[388 + 3*j];
			SPI_colors[i].blue = data[389 + 3*j];
		}

		// Group7 - led [25-23] - Data[98 - 129] - 32 pixels
		// 4 - 3 - 3 - 3 - 3 - 3 - 3 - 3 - ...
		for (i = 3; i < 6; i ++)
		{
			if (Sector < 4)				{	j = 0;	}
			else if (Sector < 7)		{	j = 1;	}
			else if (Sector < 10)		{	j = 2;	}
			else if (Sector < 13)		{	j = 3;	}
			else if (Sector < 16)		{	j = 4;	}
			else if (Sector < 19)		{	j = 5;	}
			else if (Sector < 22)		{	j = 6;	}
			else if (Sector < 25)		{	j = 7;	}
			else if (Sector < 29)		{	j = 8;	}
			else if (Sector < 32)		{	j = 9;	}
			else if (Sector < 35)		{	j = 10;	}
			else if (Sector < 38)		{	j = 11;	}
			else if (Sector < 41)		{	j = 12;	}
			else if (Sector < 44)		{	j = 13;	}
			else if (Sector < 47)		{	j = 14;	}
			else if (Sector < 50)		{	j = 15;	}
			else if (Sector < 54)		{	j = 16;	}
			else if (Sector < 57)		{	j = 17;	}
			else if (Sector < 60)		{	j = 18;	}
			else if (Sector < 63)		{	j = 19;	}
			else if (Sector < 66)		{	j = 20;	}
			else if (Sector < 69)		{	j = 21;	}
			else if (Sector < 72)		{	j = 22;	}
			else if (Sector < 75)		{	j = 23;	}
			else if (Sector < 79)		{	j = 24;	}
			else if (Sector < 82)		{	j = 25;	}
			else if (Sector < 85)		{	j = 26;	}
			else if (Sector < 88)		{	j = 27;	}
			else if (Sector < 91)		{	j = 28;	}
			else if (Sector < 94)		{	j = 29;	}
			else if (Sector < 97)		{	j = 30;	}
			else 						{	j = 31;	}
			
			SPI_colors[i].red = data[291 + 3*j];
			SPI_colors[i].green = data[292 + 3*j];
			SPI_colors[i].blue = data[293 + 3*j];
		}

		// Group6 - led [22-20] - Data[70 - 97] - 28 pixels
		// 4 - 3 - 4 - 3 - 4 - 3 - 4 - ...
		for (i = 6; i < 9; i ++)
		{
			if (Sector < 4)				{	j = 0;	}
			else if (Sector < 7)		{	j = 1;	}
			else if (Sector < 11)		{	j = 2;	}
			else if (Sector < 14)		{	j = 3;	}
			else if (Sector < 18)		{	j = 4;	}
			else if (Sector < 21)		{	j = 5;	}
			else if (Sector < 25)		{	j = 6;	}
			else if (Sector < 29)		{	j = 7;	}
			else if (Sector < 32)		{	j = 8;	}
			else if (Sector < 36)		{	j = 9;	}
			else if (Sector < 39)		{	j = 10;	}
			else if (Sector < 43)		{	j = 11;	}
			else if (Sector < 46)		{	j = 12;	}
			else if (Sector < 50)		{	j = 13;	}
			else if (Sector < 54)		{	j = 14;	}
			else if (Sector < 57)		{	j = 15;	}
			else if (Sector < 61)		{	j = 16;	}
			else if (Sector < 64)		{	j = 17;	}
			else if (Sector < 68)		{	j = 18;	}
			else if (Sector < 71)		{	j = 19;	}
			else if (Sector < 75)		{	j = 20;	}
			else if (Sector < 79)		{	j = 21;	}
			else if (Sector < 82)		{	j = 22;	}
			else if (Sector < 86)		{	j = 23;	}
			else if (Sector < 89)		{	j = 24;	}
			else if (Sector < 93)		{	j = 25;	}
			else if (Sector < 96)		{	j = 26;	}
			else 						{	j = 27;	}
			
			SPI_colors[i].red = data[207 + 3*j];
			SPI_colors[i].green = data[208 + 3*j];
			SPI_colors[i].blue = data[209 + 3*j];
		}

		// Group5 - led [19-16] - Data[46 - 69] - 24 pixels
		// 5 - 4 - 4 - 4 - 4 - 4 - ...
		for (i = 9; i < 13; i ++)
		{
			if (Sector < 5)				{	j = 0;	}
			else if (Sector < 9)		{	j = 1;	}
			else if (Sector < 13)		{	j = 2;	}
			else if (Sector < 17)		{	j = 3;	}
			else if (Sector < 21)		{	j = 4;	}
			else if (Sector < 25)		{	j = 5;	}
			else if (Sector < 30)		{	j = 6;	}
			else if (Sector < 34)		{	j = 7;	}
			else if (Sector < 38)		{	j = 8;	}
			else if (Sector < 42)		{	j = 9;	}
			else if (Sector < 46)		{	j = 10;	}
			else if (Sector < 50)		{	j = 11;	}
			else if (Sector < 55)		{	j = 12;	}
			else if (Sector < 59)		{	j = 13;	}
			else if (Sector < 63)		{	j = 14;	}
			else if (Sector < 67)		{	j = 15;	}
			else if (Sector < 71)		{	j = 16;	}
			else if (Sector < 75)		{	j = 17;	}
			else if (Sector < 79)		{	j = 18;	}
			else if (Sector < 83)		{	j = 19;	}
			else if (Sector < 87)		{	j = 20;	}
			else if (Sector < 91)		{	j = 21;	}
			else if (Sector < 96)		{	j = 22;	}
			else 						{	j = 23;	}
			
			SPI_colors[i].red = data[135 + 3*j];
			SPI_colors[i].green = data[136 + 3*j];
			SPI_colors[i].blue = data[137 + 3*j];
		}

		// Group4 - led [15-12] - Data[22 - 45] - 24 pixels
		// 5 - 4 - 4 - 4 - 4 - 4 - ... 
		for (i = 13; i < 17; i ++)
		{
			if (Sector < 5)				{	j = 0;	}
			else if (Sector < 9)		{	j = 1;	}
			else if (Sector < 13)		{	j = 2;	}
			else if (Sector < 17)		{	j = 3;	}
			else if (Sector < 21)		{	j = 4;	}
			else if (Sector < 25)		{	j = 5;	}
			else if (Sector < 30)		{	j = 6;	}
			else if (Sector < 34)		{	j = 7;	}
			else if (Sector < 38)		{	j = 8;	}
			else if (Sector < 42)		{	j = 9;	}
			else if (Sector < 46)		{	j = 10;	}
			else if (Sector < 50)		{	j = 11;	}
			else if (Sector < 55)		{	j = 12;	}
			else if (Sector < 59)		{	j = 13;	}
			else if (Sector < 63)		{	j = 14;	}
			else if (Sector < 67)		{	j = 15;	}
			else if (Sector < 71)		{	j = 16;	}
			else if (Sector < 75)		{	j = 17;	}
			else if (Sector < 79)		{	j = 18;	}
			else if (Sector < 83)		{	j = 19;	}
			else if (Sector < 87)		{	j = 20;	}
			else if (Sector < 91)		{	j = 21;	}
			else if (Sector < 96)		{	j = 22;	}
			else 						{	j = 23;	}
			
			SPI_colors[i].red = data[63 + 3*j];
			SPI_colors[i].green = data[64 + 3*j];
			SPI_colors[i].blue = data[65 + 3*j];
		}

		// Group3 - led [11-08] - Data [10 - 21] - 12 pixels
		// 8 - 9 - 8 - ...
		for (i = 17; i < 21; i ++)
		{
			if (Sector < 8)				{	j = 0;	}
			else if (Sector < 17)		{	j = 1;	}
			else if (Sector < 25)		{	j = 2;	}
			else if (Sector < 33)		{	j = 3;	}
			else if (Sector < 42)		{	j = 4;	}
			else if (Sector < 50)		{	j = 5;	}
			else if (Sector < 58)		{	j = 6;	}
			else if (Sector < 67)		{	j = 7;	}
			else if (Sector < 75)		{	j = 8;	}
			else if (Sector < 83)		{	j = 9;	}
			else if (Sector < 92)		{	j = 10;	}
			else 						{	j = 11;	}
			
			SPI_colors[i].red = data[27 + 3*j];
			SPI_colors[i].green = data[28 + 3*j];
			SPI_colors[i].blue = data[29 + 3*j];
		}

		// Group2 - led [07-04] - Data [2 - 9] - 8 pixels
		// 12 - 13 - 12 - ...
		for (i = 21; i < 25; i ++)
		{
			if (Sector < 12)			{	j = 0;	}
			else if (Sector < 25)		{	j = 1;	}
			else if (Sector < 37)		{	j = 2;	}
			else if (Sector < 50)		{	j = 3;	}
			else if (Sector < 62)		{	j = 4;	}
			else if (Sector < 75)		{	j = 5;	}
			else if (Sector < 87)		{	j = 6;	}
			else 						{	j = 7;	}
			
			SPI_colors[i].red = data[3 + 3*j];
			SPI_colors[i].green = data[4 + 3*j];
			SPI_colors[i].blue = data[5 + 3*j];
		}

		// Group1 - led [03-01] - Data [1] - 1 pixel
		for (i = 25; i < 28; i ++)
		{
			SPI_colors[i].red = data[0];
			SPI_colors[i].green = data[1];
			SPI_colors[i].blue = data[2];
		}
		*/
		/*
		// Test leds
		color.red = 0;
		color.green = 0;
		color.blue = 255;
		
		for(i = 0; i < LED_COUNT; i++)
        {
            SPI_colors[i] = color;
        }*/
        memcpy(SPI_colors, data, NB_DATAS_2);
		ledStrip.write(SPI_colors, LED_COUNT);	// Update the colors buffer.
		delay(1);
		Cpt = 0;
		bitSet(SPCR, SPIE);
	}
}

