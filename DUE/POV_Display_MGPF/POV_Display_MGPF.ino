#include <SPI.h>
#include <DueTimer.h>
#include <AccelStepper.h>
#include <Button.h>
#include "PololuLedStrip.h"

// Prototypes
ISR(SPI0_Handler);

void	Capteur_Interrupt (void);
void	Control_Stepper (void);
void	SPI_Slave_Initialize (unsigned long Mode);
void	SPI_Mask_Interrupts (void);
void	SPI_Unmask_Interrupts (void);
void	SPI_Refresh_Data (void);
void	SPI_Print_Data (void);
void	Test_Led (void);
void	COLOR_Refresh (void);
void	LED_Refresh (void);
void	Motor_Run(void);


// Déclarations
// LED
#define 		LED_COMMAND			7
#define 		LED_COUNT 			28
PololuLedStrip<LED_COMMAND> 		ledStrip;
rgb_color							colors[LED_COUNT];

// STEPPER
#define			STEPPER_MAX_SPEED	1500		// steps / seconds
#define			STEPPER_SPEED		1500		// steps / seconds
#define			STEPPER_ACCEL 		400			// acceleration rate in steps / seconds
#define			STEPPER_MIN_PULSE	50			// µs
#define			STEPPER_INIT_MOVE	0x0fffffff	// nombre de step pour atteindre vmax
AccelStepper 						Motor(AccelStepper::FULL4WIRE, 2, 3, 4, 5);
bool								Motor_Is_Init = false;
bool								Motor_Is_Running = false;

// SPI
#define			NB_LED_DISPLAY		161
#define			NB_BYTE_PAR_LED		3
#define			NB_DATAS			(unsigned int) (NB_LED_DISPLAY * NB_BYTE_PAR_LED)
#define			SPI_TIME_OUT		5	// ms

typedef struct	StructSpi{
	unsigned int	Counter;
	unsigned long	Last_Time_Rcv;
	unsigned char	Data[NB_DATAS];
	unsigned char	Rcv_Data[NB_DATAS];
	bool			Check_Time_Out;
	bool			DataOk;
}StruSpi;

StruSpi								Spi0;

// BOUTON
#define			BUTTON_INPUT		6
Button 								Button_Moteur_On(BUTTON_INPUT);

// DIVERS
#define 		INPUT_CAPTEUR		8
#define			SECTOR_NB_MAX		100
//#define		TIMER_PERIOD		(60 * 1000 * 1000) / (SECTOR_NB_MAX * STEPPER_MAX_SPEED) // µs
byte								Step, Sector, MemoSector;


// Définition des interruptions
void Capteur_Interrupt(void)
{
	Step = 0;
}

ISR (SPI0_Handler)
{
	/*if (REG_SPI0_SR & SPI_SR_OVRES)
	{
		// Au moins 1 byte à été perdu
	}*/

	if (REG_SPI0_SR & SPI_SR_RDRF)
	{
		// Récupére la data dans le buffer de reception
		Spi0.Rcv_Data[Spi0.Counter] = REG_SPI0_RDR;
		// Nombre de datas max atteint
		if (++ Spi0.Counter >= NB_DATAS)
		{ 
			Spi0.Counter = 0;
			Spi0.DataOk = true;
		}

		Spi0.Check_Time_Out = true;
		Spi0.Last_Time_Rcv = millis();
	}
}

/*// USE TIMER
 * void Control_Stepper(void)
{
	if (Motor_Is_Running == true)
	{
		if (++ Sector >= SECTOR_NB_MAX)  
		{
			Sector = 0;
		}
	}
}*/

void setup()
{
	// Configure inputs
	pinMode(INPUT_CAPTEUR, INPUT);

	// Configure outputs
	pinMode(LED_COMMAND, OUTPUT);
	
	// Initialise la liaison série à 115200 bauds
	Serial.begin(115200);

	// Configure interruptions
	attachInterrupt(INPUT_CAPTEUR, Capteur_Interrupt, FALLING);

	// Allume la strip light blanc très faible
	memset(Spi0.Data, 1, NB_DATAS);

	// SPI initialisation
	SPI_Slave_Initialize(SPI_MODE0);

	// Stepper initialisation 
	Motor.setMaxSpeed(STEPPER_MAX_SPEED);
	Motor.setAcceleration(STEPPER_ACCEL);
	Motor.setSpeed(STEPPER_SPEED);
	Motor.setMinPulseWidth(STEPPER_MIN_PULSE);
	//Motor.move(STEPPER_INIT_MOVE);
	
	// Timer initialisation
	//Timer3.attachInterrupt(Control_Stepper);
	//Timer3.start(TIMER_PERIOD);

	// Button initialisation
	Button_Moteur_On.begin();

	NVIC_EnableIRQ(SPI0_IRQn);
}

void loop()
{   
	/*if (Button_Moteur_On.read() == Button::PRESSED)
	{
		if (Motor_Is_Running == false)
		{
			if (Motor_Is_Init == false)
			{
				Motor_Is_Init = true;
				Motor.move(STEPPER_INIT_MOVE);
				Motor.setSpeed(200);
				Serial.println("Motor init");
			}
			else if (Motor.speed() == STEPPER_SPEED)
			{
				Motor_Is_Running = true;
				Motor.setSpeed(STEPPER_SPEED);
				Serial.println("Motor end acceleration");
			}
			Motor.run();
		}
		else
		{
			LED_Refresh();
			if (Motor.runSpeed() == true)	{	Step ++;	}
		}
	}
	else if (Button_Moteur_On.read() == Button::RELEASED)
	{
		if ((Motor_Is_Init == true) || (Motor_Is_Running == true))
		{
			Motor_Is_Running = false;
			Motor_Is_Init = false;	
			Motor.stop();
			Motor.disableOutputs();
			Serial.println("Motor stop");
			Serial.println(Motor.speed());
		}
	}*/
	SPI_Refresh_Data();	
}

void SPI_Slave_Initialize (unsigned long Mode)
{
	// SPI pour la DUE
	// MOSI	ICSP-4
	// MISO	ICSP-1
	// SCK	ICSP-3
	// SS0  pin10
	SPI.begin();
	REG_SPI0_CR = SPI_CR_SWRST;		// reset SPI
	REG_SPI0_CR = SPI_CR_SPIEN;		// enable SPI
	REG_SPI0_MR = SPI_MR_MODFDIS;	// slave and no modefault
	REG_SPI0_CSR = Mode;			// DLYBCT=0, DLYBS=0, SCBR=0, 8 bit transfer
	REG_SPI0_IER = (SPI_IER_RDRF /*| SPI_IER_OVRES*/);
}

void SPI_Mask_Interrupts (void)
{
	REG_SPI0_IMR = (SPI_IMR_RDRF | SPI_IMR_OVRES);
}

void SPI_Unmask_Interrupts (void)
{
	REG_SPI0_IMR &= ~(SPI_IMR_RDRF | SPI_IMR_OVRES);
}

void SPI_Refresh_Data (void)
{
	if (Spi0.DataOk == true)
	{
		SPI_Mask_Interrupts();
		memcpy(Spi0.Data, &Spi0.Rcv_Data[0], NB_DATAS);
		Spi0.DataOk = false;
		Spi0.Check_Time_Out = false;
		SPI_Unmask_Interrupts();
		
		SPI_Print_Data();
	}
	else if (Spi0.Check_Time_Out == true)
	{
		if ((millis() - Spi0.Last_Time_Rcv) > SPI_TIME_OUT)
		{
			Spi0.Check_Time_Out = false;
			Spi0.Counter = 0;
		}
	}
}

void SPI_Print_Data (void)
{
	unsigned char    i;
	
	Serial.println("Début trame");
	for(i = 0; i < NB_DATAS; i++)
	{
		Serial.print(Spi0.Data[i]);
	}
	Serial.println("fin trame");
}

void Test_Led (void)
{
	unsigned char    i;
	rgb_color		color;
	
	color.red = 0;
	color.green = 0;
	color.blue = 255;
		
	for(i = 0; i < LED_COUNT; i++)
    {
        colors[i] = color;
    }
}

void COLOR_Refresh (void)
{
	unsigned char    i,j;
	
	// Group8 - led [28-26] - Data[130 - 161] - 32 pixels
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
		
		colors[i].red = Spi0.Data[387 + 3*j];
		colors[i].green = Spi0.Data[388 + 3*j];
		colors[i].blue = Spi0.Data[389 + 3*j];
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
		
		colors[i].red = Spi0.Data[291 + 3*j];
		colors[i].green = Spi0.Data[292 + 3*j];
		colors[i].blue = Spi0.Data[293 + 3*j];
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
		
		colors[i].red = Spi0.Data[207 + 3*j];
		colors[i].green = Spi0.Data[208 + 3*j];
		colors[i].blue = Spi0.Data[209 + 3*j];
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
		
		colors[i].red = Spi0.Data[135 + 3*j];
		colors[i].green = Spi0.Data[136 + 3*j];
		colors[i].blue = Spi0.Data[137 + 3*j];
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
		
		colors[i].red = Spi0.Data[63 + 3*j];
		colors[i].green = Spi0.Data[64 + 3*j];
		colors[i].blue = Spi0.Data[65 + 3*j];
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
		
		colors[i].red = Spi0.Data[27 + 3*j];
		colors[i].green = Spi0.Data[28 + 3*j];
		colors[i].blue = Spi0.Data[29 + 3*j];
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
		
		colors[i].red = Spi0.Data[3 + 3*j];
		colors[i].green = Spi0.Data[4 + 3*j];
		colors[i].blue = Spi0.Data[5 + 3*j];
	}

	// Group1 - led [03-01] - Data [1] - 1 pixel
	for (i = 25; i < 28; i ++)
	{
		colors[i].red = Spi0.Data[0];
		colors[i].green = Spi0.Data[1];
		colors[i].blue = Spi0.Data[2];
	}
}

void LED_Refresh (void)
{
	Sector = Step / 2;
	
	if (Sector != MemoSector)
	{
		MemoSector = Sector;

		COLOR_Refresh();
		
		ledStrip.write(colors, LED_COUNT);
	}
}

void Motor_Run(void)
{
	if (Motor.runSpeed() == true)	{	Step ++;	}
}
