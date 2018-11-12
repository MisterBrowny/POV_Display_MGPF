#include <SPI.h>
#include <AccelStepper.h>
#include <Button.h>
#include "WS2801.h"

// Prototypes
ISR(SPI0_Handler);

void    Capteur_Interrupt (void);
void    Motor_Init (void);
void    SPI_Slave_Initialize (unsigned long Mode);
void    SPI_Slave_Stop (void);
void    SPI_Mask_Interrupts (void);
void    SPI_Unmask_Interrupts (void);
void    SPI_Refresh_Data (void);
void    SPI_Print_Data (int nb_data);
void    LED_Refresh (void);
void    LED_Refresh_Test (void);
void    Test_Led (void);
void    COLOR_Refresh (void);
void    COLOR_Refresh_Test(void);

// Déclarations
// LED
#define SCK_PIN     7
#define SDA_PIN     9
SPI_WS2801<SDA_PIN, SCK_PIN> WS2801;

#define LED_COUNT   15
rgb_color           colors[LED_COUNT];

// STEPPER
#define STEPPER_MAX_SPEED   1700        // steps / seconds
#define STEPPER_SPEED       1700        // steps / seconds
#define STEPPER_ACCEL       200         // acceleration rate in steps / seconds
#define STEPPER_INIT_SPEED  200         // steps / seconds
#define STEPPER_MIN_PULSE   0           // µs
#define STEPPER_INIT_MOVE   0x0fffffff  // nombre de step pour atteindre vmax
AccelStepper  Motor(AccelStepper::FULL4WIRE, 2, 3, 4, 5);
bool          Motor_Is_Init = false;
bool          Motor_Is_Running = false;

// SPI
#define NB_LED_DISPLAY      161
#define NB_BYTE_PAR_LED     3
#define NB_DATAS            (unsigned int) (NB_LED_DISPLAY * NB_BYTE_PAR_LED)
#define SPI_TIME_OUT        2000    // µs

typedef struct    StructSpi{
    unsigned int    Counter;
    unsigned long   Last_Time_Rcv;
    byte            Data[NB_DATAS];
    bool            Check_Time_Out;
    volatile bool   Save_Time;
}StruSpi;

StruSpi Spi0;

// BOUTON
#define BUTTON_INPUT    6
Button  Button_Moteur_On(BUTTON_INPUT);

// DIVERS
#define INPUT_CAPTEUR   8
#define SECTOR_NB_MAX   100
byte  Step, Sector, MemoSector;


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
    int SR = REG_SPI0_SR;
    // Récupére la data dans le buffer de reception
    Spi0.Data[Spi0.Counter] = REG_SPI0_RDR;
    Spi0.Counter ++;
    Spi0.Save_Time = true;
  }
}

void setup()
{
  // Configure inputs
  pinMode(INPUT_CAPTEUR, INPUT);

  // Configure WS2801
  WS2801.begin();

  // Initialise la liaison série à 115200 bauds
  Serial.begin(115200);

  // Configure interruptions
  attachInterrupt(INPUT_CAPTEUR, Capteur_Interrupt, FALLING);

  // Allume la strip light blanc très faible
  memset(Spi0.Data, 1, NB_DATAS);

  // SPI initialisation
  SPI_Slave_Initialize(SPI_MODE0);

  // Button initialisation
  Button_Moteur_On.begin();
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
        Motor_Init();
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
      if (Motor.runSpeed() == true)    {    Step ++;    }
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
    }
  }
  */
  if ((Spi0.Save_Time == false) && (Spi0.Check_Time_Out == false))
  {
    SPI_Slave_Stop();
    LED_Refresh();
    SPI_Slave_Initialize(SPI_MODE0);
    delayMicroseconds(1500);
  }
  SPI_Refresh_Data();
  //LED_Refresh_Test();
  //delayMicroseconds(1500);
}

void Motor_Init (void)
{
  // Stepper initialisation
  Motor.setMaxSpeed(STEPPER_MAX_SPEED);
  Motor.setAcceleration(STEPPER_ACCEL);
  Motor.setSpeed(STEPPER_SPEED);
  Motor.setMinPulseWidth(STEPPER_MIN_PULSE);
  Motor.move(STEPPER_INIT_MOVE);
  Motor.setSpeed(STEPPER_INIT_SPEED);
}

void SPI_Slave_Initialize (unsigned long Mode)
{
  //Nested Vector Interrupt Controller configuration
  //To set up the handler
  NVIC_DisableIRQ(SPI0_IRQn);
  NVIC_ClearPendingIRQ(SPI0_IRQn);
  NVIC_SetPriority(SPI0_IRQn, 0);
  NVIC_EnableIRQ(SPI0_IRQn);
  
  // SPI pour la DUE
  // MOSI    ICSP-4
  // MISO    ICSP-1
  // SCK    ICSP-3
  // SS0    pin10
  SPI.begin();
  REG_SPI0_CR = SPI_CR_SWRST;         // reset SPI
  REG_SPI0_MR = SPI_MR_MODFDIS;       // slave and no modefault
  REG_SPI0_CSR = Mode;                // DLYBCT=0, DLYBS=0, SCBR=0, 8 bit transfer
  REG_SPI0_IER = SPI_IER_RDRF;        // active RX interruption on SPI
  //REG_SPI0_IER |= SPI_IER_OVRES;    // active Overrun RX interruption on SPI
  REG_SPI0_CR = SPI_CR_SPIEN;         // enable SPI
}

void SPI_Slave_Stop (void)
{
  SPI_Mask_Interrupts();
  REG_SPI0_CR = SPI_CR_SWRST;     // reset SPI
  NVIC_DisableIRQ(SPI0_IRQn);
  NVIC_ClearPendingIRQ(SPI0_IRQn);
  REG_SPI0_CR = SPI_CR_SWRST;     // reset SPI
  Spi0.Save_Time = false;
  Spi0.Check_Time_Out = false;
  Spi0.Counter = 0;
}

void SPI_Mask_Interrupts (void)
{
  REG_SPI0_IMR = SPI_IMR_RDRF;
  //REG_SPI0_IMR |= SPI_IMR_OVRES;
}

void SPI_Unmask_Interrupts (void)
{
  REG_SPI0_IMR &= ~(SPI_IMR_RDRF);
  //REG_SPI0_IMR &= ~(SPI_IMR_RDRF | SPI_IMR_OVRES);
}

void SPI_Refresh_Data (void)
{
  if (Spi0.Save_Time == true)
  {
    Spi0.Save_Time = false;
    Spi0.Check_Time_Out = true;
    Spi0.Last_Time_Rcv = micros();
  }

  if (Spi0.Check_Time_Out == true)
  {
    if ((micros() - Spi0.Last_Time_Rcv) > SPI_TIME_OUT)
    {
      SPI_Slave_Stop();
      Spi0.Check_Time_Out = false;
      Spi0.Save_Time = false;
      //SPI_Print_Data(Spi0.Counter);
      Spi0.Counter = 0;
      SPI_Slave_Initialize(SPI_MODE0);
    }
  }
}

void SPI_Print_Data (int nb_data)
{
  int   i;

  Serial.println(nb_data);
  for(i = 0; i < nb_data; i++)
  {
    if (i % 12 == 0)
    {
      Serial.print(i);
      Serial.print(":");
      Serial.print(Spi0.Data[i]);
      Serial.print(',');
    }
    if (i % 8 == 0) Serial.println();
  }
  Serial.println();
}

void LED_Refresh (void)
{
  Sector = Step / 2;

  //if (Sector != MemoSector)
  {
    MemoSector = Sector;

    //COLOR_Refresh_Test();
    COLOR_Refresh();
    WS2801.writeStrip(colors, LED_COUNT);
  }
}

void LED_Refresh_Test (void)
{
  Test_Led();

  WS2801.writeStrip(colors, LED_COUNT);
}

void Test_Led (void)
{
  unsigned char    i;
  rgb_color        color;

  color.red = 0;
  color.green = 0;
  color.blue = 255;

  for(i = 0; i < LED_COUNT; i++)
  {
    colors[i] = color;
  }
}

void COLOR_Refresh(void)
{
  unsigned char    i,j;
  rgb_color        color;
  /*if (Sector <= 4)
  {
    color.red = 255;
    color.green = 255;
    color.blue = 255;
    
    for(i = 0; i < LED_COUNT; i++)
    {
      colors[i] = color;
    }
  }
  else*/
  {
    // Group8 - led [0] - Data[130 - 161] - 32 pixels
    // 4 - 3 - 3 - 3 - 3 - 3 - 3 - 3 - ...
    if (Sector < 4)              {    j = 0;    }
    else if (Sector < 7)         {    j = 1;    }
    else if (Sector < 10)        {    j = 2;    }
    else if (Sector < 13)        {    j = 3;    }
    else if (Sector < 16)        {    j = 4;    }
    else if (Sector < 19)        {    j = 5;    }
    else if (Sector < 22)        {    j = 6;    }
    else if (Sector < 25)        {    j = 7;    }
    else if (Sector < 29)        {    j = 8;    }
    else if (Sector < 32)        {    j = 9;    }
    else if (Sector < 35)        {    j = 10;    }
    else if (Sector < 38)        {    j = 11;    }
    else if (Sector < 41)        {    j = 12;    }
    else if (Sector < 44)        {    j = 13;    }
    else if (Sector < 47)        {    j = 14;    }
    else if (Sector < 50)        {    j = 15;    }
    else if (Sector < 54)        {    j = 16;    }
    else if (Sector < 57)        {    j = 17;    }
    else if (Sector < 60)        {    j = 18;    }
    else if (Sector < 63)        {    j = 19;    }
    else if (Sector < 66)        {    j = 20;    }
    else if (Sector < 69)        {    j = 21;    }
    else if (Sector < 72)        {    j = 22;    }
    else if (Sector < 75)        {    j = 23;    }
    else if (Sector < 79)        {    j = 24;    }
    else if (Sector < 82)        {    j = 25;    }
    else if (Sector < 85)        {    j = 26;    }
    else if (Sector < 88)        {    j = 27;    }
    else if (Sector < 91)        {    j = 28;    }
    else if (Sector < 94)        {    j = 29;    }
    else if (Sector < 97)        {    j = 30;    }
    else                         {    j = 31;    }
    
    colors[0].red = Spi0.Data[387 + 3*j];
    colors[0].green = Spi0.Data[388 + 3*j];
    colors[0].blue = Spi0.Data[389 + 3*j];
  
  
    // Group7 - led [1] - Data[98 - 129] - 32 pixels
    // 4 - 3 - 3 - 3 - 3 - 3 - 3 - 3 - ...
    if (Sector < 4)              {    j = 0;    }
    else if (Sector < 7)         {    j = 1;    }
    else if (Sector < 10)        {    j = 2;    }
    else if (Sector < 13)        {    j = 3;    }
    else if (Sector < 16)        {    j = 4;    }
    else if (Sector < 19)        {    j = 5;    }
    else if (Sector < 22)        {    j = 6;    }
    else if (Sector < 25)        {    j = 7;    }
    else if (Sector < 29)        {    j = 8;    }
    else if (Sector < 32)        {    j = 9;    }
    else if (Sector < 35)        {    j = 10;    }
    else if (Sector < 38)        {    j = 11;    }
    else if (Sector < 41)        {    j = 12;    }
    else if (Sector < 44)        {    j = 13;    }
    else if (Sector < 47)        {    j = 14;    }
    else if (Sector < 50)        {    j = 15;    }
    else if (Sector < 54)        {    j = 16;    }
    else if (Sector < 57)        {    j = 17;    }
    else if (Sector < 60)        {    j = 18;    }
    else if (Sector < 63)        {    j = 19;    }
    else if (Sector < 66)        {    j = 20;    }
    else if (Sector < 69)        {    j = 21;    }
    else if (Sector < 72)        {    j = 22;    }
    else if (Sector < 75)        {    j = 23;    }
    else if (Sector < 79)        {    j = 24;    }
    else if (Sector < 82)        {    j = 25;    }
    else if (Sector < 85)        {    j = 26;    }
    else if (Sector < 88)        {    j = 27;    }
    else if (Sector < 91)        {    j = 28;    }
    else if (Sector < 94)        {    j = 29;    }
    else if (Sector < 97)        {    j = 30;    }
    else                         {    j = 31;    }
    
    colors[1].red = Spi0.Data[291 + 3*j];
    colors[1].green = Spi0.Data[292 + 3*j];
    colors[1].blue = Spi0.Data[293 + 3*j];
    
  
    // Group6 - led [2] - Data[70 - 97] - 28 pixels
    // 4 - 3 - 4 - 3 - 4 - 3 - 4 - ...
    if (Sector < 4)              {    j = 0;    }
    else if (Sector < 7)         {    j = 1;    }
    else if (Sector < 11)        {    j = 2;    }
    else if (Sector < 14)        {    j = 3;    }
    else if (Sector < 18)        {    j = 4;    }
    else if (Sector < 21)        {    j = 5;    }
    else if (Sector < 25)        {    j = 6;    }
    else if (Sector < 29)        {    j = 7;    }
    else if (Sector < 32)        {    j = 8;    }
    else if (Sector < 36)        {    j = 9;    }
    else if (Sector < 39)        {    j = 10;    }
    else if (Sector < 43)        {    j = 11;    }
    else if (Sector < 46)        {    j = 12;    }
    else if (Sector < 50)        {    j = 13;    }
    else if (Sector < 54)        {    j = 14;    }
    else if (Sector < 57)        {    j = 15;    }
    else if (Sector < 61)        {    j = 16;    }
    else if (Sector < 64)        {    j = 17;    }
    else if (Sector < 68)        {    j = 18;    }
    else if (Sector < 71)        {    j = 19;    }
    else if (Sector < 75)        {    j = 20;    }
    else if (Sector < 79)        {    j = 21;    }
    else if (Sector < 82)        {    j = 22;    }
    else if (Sector < 86)        {    j = 23;    }
    else if (Sector < 89)        {    j = 24;    }
    else if (Sector < 93)        {    j = 25;    }
    else if (Sector < 96)        {    j = 26;    }
    else                         {    j = 27;    }
    
    colors[2].red = Spi0.Data[207 + 3*j];
    colors[2].green = Spi0.Data[208 + 3*j];
    colors[2].blue = Spi0.Data[209 + 3*j];
    
    // Group5 - led [3] - Data[46 - 69] - 24 pixels
    // 5 - 4 - 4 - 4 - 4 - 4 - ...
    if (Sector < 5)              {    j = 0;    }
    else if (Sector < 9)         {    j = 1;    }
    else if (Sector < 13)        {    j = 2;    }
    else if (Sector < 17)        {    j = 3;    }
    else if (Sector < 21)        {    j = 4;    }
    else if (Sector < 25)        {    j = 5;    }
    else if (Sector < 30)        {    j = 6;    }
    else if (Sector < 34)        {    j = 7;    }
    else if (Sector < 38)        {    j = 8;    }
    else if (Sector < 42)        {    j = 9;    }
    else if (Sector < 46)        {    j = 10;    }
    else if (Sector < 50)        {    j = 11;    }
    else if (Sector < 55)        {    j = 12;    }
    else if (Sector < 59)        {    j = 13;    }
    else if (Sector < 63)        {    j = 14;    }
    else if (Sector < 67)        {    j = 15;    }
    else if (Sector < 71)        {    j = 16;    }
    else if (Sector < 75)        {    j = 17;    }
    else if (Sector < 79)        {    j = 18;    }
    else if (Sector < 83)        {    j = 19;    }
    else if (Sector < 87)        {    j = 20;    }
    else if (Sector < 91)        {    j = 21;    }
    else if (Sector < 96)        {    j = 22;    }
    else                         {    j = 23;    }
    
    colors[3].red = Spi0.Data[135 + 3*j];
    colors[3].green = Spi0.Data[136 + 3*j];
    colors[3].blue = Spi0.Data[137 + 3*j];
    
    // Group4 - led [4] - Data[22 - 45] - 24 pixels
    // 5 - 4 - 4 - 4 - 4 - 4 - ...
    if (Sector < 5)              {    j = 0;    }
    else if (Sector < 9)         {    j = 1;    }
    else if (Sector < 13)        {    j = 2;    }
    else if (Sector < 17)        {    j = 3;    }
    else if (Sector < 21)        {    j = 4;    }
    else if (Sector < 25)        {    j = 5;    }
    else if (Sector < 30)        {    j = 6;    }
    else if (Sector < 34)        {    j = 7;    }
    else if (Sector < 38)        {    j = 8;    }
    else if (Sector < 42)        {    j = 9;    }
    else if (Sector < 46)        {    j = 10;    }
    else if (Sector < 50)        {    j = 11;    }
    else if (Sector < 55)        {    j = 12;    }
    else if (Sector < 59)        {    j = 13;    }
    else if (Sector < 63)        {    j = 14;    }
    else if (Sector < 67)        {    j = 15;    }
    else if (Sector < 71)        {    j = 16;    }
    else if (Sector < 75)        {    j = 17;    }
    else if (Sector < 79)        {    j = 18;    }
    else if (Sector < 83)        {    j = 19;    }
    else if (Sector < 87)        {    j = 20;    }
    else if (Sector < 91)        {    j = 21;    }
    else if (Sector < 96)        {    j = 22;    }
    else                         {    j = 23;    }
    
    colors[4].red = Spi0.Data[63 + 3*j];
    colors[4].green = Spi0.Data[64 + 3*j];
    colors[4].blue = Spi0.Data[65 + 3*j];
    
    // Group3 - led [5] - Data [10 - 21] - 12 pixels
    // 8 - 9 - 8 - ...
    if (Sector < 8)              {    j = 0;    }
    else if (Sector < 17)        {    j = 1;    }
    else if (Sector < 25)        {    j = 2;    }
    else if (Sector < 33)        {    j = 3;    }
    else if (Sector < 42)        {    j = 4;    }
    else if (Sector < 50)        {    j = 5;    }
    else if (Sector < 58)        {    j = 6;    }
    else if (Sector < 67)        {    j = 7;    }
    else if (Sector < 75)        {    j = 8;    }
    else if (Sector < 83)        {    j = 9;    }
    else if (Sector < 92)        {    j = 10;    }
    else                         {    j = 11;    }
    
    colors[5].red = Spi0.Data[27 + 3*j];
    colors[5].green = Spi0.Data[28 + 3*j];
    colors[5].blue = Spi0.Data[29 + 3*j];
    
    // Group2 - led [6] - Data [2 - 9] - 8 pixels
    // 12 - 13 - 12 - ...
    if (Sector < 12)             {    j = 0;    }
    else if (Sector < 25)        {    j = 1;    }
    else if (Sector < 37)        {    j = 2;    }
    else if (Sector < 50)        {    j = 3;    }
    else if (Sector < 62)        {    j = 4;    }
    else if (Sector < 75)        {    j = 5;    }
    else if (Sector < 87)        {    j = 6;    }
    else                         {    j = 7;    }
    
    colors[6].red = Spi0.Data[3 + 3*j];
    colors[6].green = Spi0.Data[4 + 3*j];
    colors[6].blue = Spi0.Data[5 + 3*j];
    
    // Group1 - led [7] - Data [1] - 1 pixel
    colors[7].red = Spi0.Data[0];
    colors[7].green = Spi0.Data[1];
    colors[7].blue = Spi0.Data[2];

    // Group2 - led [8] - Data [2 - 9] - 8 pixels
    // 12 - 13 - 12 - ...
    if (Sector < 12)             {    j = 7;    }
    else if (Sector < 25)        {    j = 6;    }
    else if (Sector < 37)        {    j = 5;    }
    else if (Sector < 50)        {    j = 4;    }
    else if (Sector < 62)        {    j = 3;    }
    else if (Sector < 75)        {    j = 2;    }
    else if (Sector < 87)        {    j = 1;    }
    else                         {    j = 0;    }
    
    colors[8].red = Spi0.Data[3 + 3*j];
    colors[8].green = Spi0.Data[4 + 3*j];
    colors[8].blue = Spi0.Data[5 + 3*j];

    // Group3 - led [9] - Data [10 - 21] - 12 pixels
    // 8 - 9 - 8 - ...
    if (Sector < 8)              {    j = 11;    }
    else if (Sector < 17)        {    j = 10;    }
    else if (Sector < 25)        {    j = 9;    }
    else if (Sector < 33)        {    j = 8;    }
    else if (Sector < 42)        {    j = 7;    }
    else if (Sector < 50)        {    j = 6;    }
    else if (Sector < 58)        {    j = 5;    }
    else if (Sector < 67)        {    j = 4;    }
    else if (Sector < 75)        {    j = 3;    }
    else if (Sector < 83)        {    j = 2;    }
    else if (Sector < 92)        {    j = 1;    }
    else                         {    j = 0;    }
    
    colors[9].red = Spi0.Data[27 + 3*j];
    colors[9].green = Spi0.Data[28 + 3*j];
    colors[9].blue = Spi0.Data[29 + 3*j];

    // Group4 - led [10] - Data[22 - 45] - 24 pixels
    // 5 - 4 - 4 - 4 - 4 - 4 - ...
    if (Sector < 5)              {    j = 23;    }
    else if (Sector < 9)         {    j = 22;    }
    else if (Sector < 13)        {    j = 21;    }
    else if (Sector < 17)        {    j = 20;    }
    else if (Sector < 21)        {    j = 19;    }
    else if (Sector < 25)        {    j = 18;    }
    else if (Sector < 30)        {    j = 17;    }
    else if (Sector < 34)        {    j = 16;    }
    else if (Sector < 38)        {    j = 15;    }
    else if (Sector < 42)        {    j = 14;    }
    else if (Sector < 46)        {    j = 13;    }
    else if (Sector < 50)        {    j = 12;    }
    else if (Sector < 55)        {    j = 11;    }
    else if (Sector < 59)        {    j = 10;    }
    else if (Sector < 63)        {    j = 9;    }
    else if (Sector < 67)        {    j = 8;    }
    else if (Sector < 71)        {    j = 7;    }
    else if (Sector < 75)        {    j = 6;    }
    else if (Sector < 79)        {    j = 5;    }
    else if (Sector < 83)        {    j = 4;    }
    else if (Sector < 87)        {    j = 3;    }
    else if (Sector < 91)        {    j = 2;    }
    else if (Sector < 96)        {    j = 1;    }
    else                         {    j = 0;    }
    
    colors[10].red = Spi0.Data[63 + 3*j];
    colors[10].green = Spi0.Data[64 + 3*j];
    colors[10].blue = Spi0.Data[65 + 3*j];

    // Group5 - led [11] - Data[46 - 69] - 24 pixels
    // 5 - 4 - 4 - 4 - 4 - 4 - ...
    if (Sector < 5)              {    j = 23;    }
    else if (Sector < 9)         {    j = 22;    }
    else if (Sector < 13)        {    j = 21;    }
    else if (Sector < 17)        {    j = 20;    }
    else if (Sector < 21)        {    j = 19;    }
    else if (Sector < 25)        {    j = 18;    }
    else if (Sector < 30)        {    j = 17;    }
    else if (Sector < 34)        {    j = 16;    }
    else if (Sector < 38)        {    j = 15;    }
    else if (Sector < 42)        {    j = 14;    }
    else if (Sector < 46)        {    j = 13;    }
    else if (Sector < 50)        {    j = 12;    }
    else if (Sector < 55)        {    j = 11;    }
    else if (Sector < 59)        {    j = 10;    }
    else if (Sector < 63)        {    j = 9;    }
    else if (Sector < 67)        {    j = 8;    }
    else if (Sector < 71)        {    j = 7;    }
    else if (Sector < 75)        {    j = 6;    }
    else if (Sector < 79)        {    j = 5;    }
    else if (Sector < 83)        {    j = 4;    }
    else if (Sector < 87)        {    j = 3;    }
    else if (Sector < 91)        {    j = 2;    }
    else if (Sector < 96)        {    j = 1;    }
    else                         {    j = 0;    }
    
    colors[11].red = Spi0.Data[135 + 3*j];
    colors[11].green = Spi0.Data[136 + 3*j];
    colors[11].blue = Spi0.Data[137 + 3*j];

    // Group6 - led [12] - Data[70 - 97] - 28 pixels
    // 4 - 3 - 4 - 3 - 4 - 3 - 4 - ...
    if (Sector < 4)              {    j = 27;    }
    else if (Sector < 7)         {    j = 26;    }
    else if (Sector < 11)        {    j = 25;    }
    else if (Sector < 14)        {    j = 24;    }
    else if (Sector < 18)        {    j = 23;    }
    else if (Sector < 21)        {    j = 22;    }
    else if (Sector < 25)        {    j = 21;    }
    else if (Sector < 29)        {    j = 20;    }
    else if (Sector < 32)        {    j = 19;    }
    else if (Sector < 36)        {    j = 18;    }
    else if (Sector < 39)        {    j = 17;    }
    else if (Sector < 43)        {    j = 16;    }
    else if (Sector < 46)        {    j = 15;    }
    else if (Sector < 50)        {    j = 14;    }
    else if (Sector < 54)        {    j = 13;    }
    else if (Sector < 57)        {    j = 12;    }
    else if (Sector < 61)        {    j = 11;    }
    else if (Sector < 64)        {    j = 10;    }
    else if (Sector < 68)        {    j = 9;    }
    else if (Sector < 71)        {    j = 8;    }
    else if (Sector < 75)        {    j = 7;    }
    else if (Sector < 79)        {    j = 6;    }
    else if (Sector < 82)        {    j = 5;    }
    else if (Sector < 86)        {    j = 4;    }
    else if (Sector < 89)        {    j = 3;    }
    else if (Sector < 93)        {    j = 2;    }
    else if (Sector < 96)        {    j = 1;    }
    else                         {    j = 0;    }
    
    colors[12].red = Spi0.Data[207 + 3*j];
    colors[12].green = Spi0.Data[208 + 3*j];
    colors[12].blue = Spi0.Data[209 + 3*j];

    // Group7 - led [13] - Data[98 - 129] - 32 pixels
    // 4 - 3 - 3 - 3 - 3 - 3 - 3 - 3 - ...
    if (Sector < 4)              {    j = 31;    }
    else if (Sector < 7)         {    j = 30;    }
    else if (Sector < 10)        {    j = 29;    }
    else if (Sector < 13)        {    j = 28;    }
    else if (Sector < 16)        {    j = 27;    }
    else if (Sector < 19)        {    j = 26;    }
    else if (Sector < 22)        {    j = 25;    }
    else if (Sector < 25)        {    j = 24;    }
    else if (Sector < 29)        {    j = 23;    }
    else if (Sector < 32)        {    j = 22;    }
    else if (Sector < 35)        {    j = 21;    }
    else if (Sector < 38)        {    j = 20;    }
    else if (Sector < 41)        {    j = 19;    }
    else if (Sector < 44)        {    j = 18;    }
    else if (Sector < 47)        {    j = 17;    }
    else if (Sector < 50)        {    j = 16;    }
    else if (Sector < 54)        {    j = 15;    }
    else if (Sector < 57)        {    j = 14;    }
    else if (Sector < 60)        {    j = 13;    }
    else if (Sector < 63)        {    j = 12;    }
    else if (Sector < 66)        {    j = 11;    }
    else if (Sector < 69)        {    j = 10;    }
    else if (Sector < 72)        {    j = 9;    }
    else if (Sector < 75)        {    j = 8;    }
    else if (Sector < 79)        {    j = 7;    }
    else if (Sector < 82)        {    j = 6;    }
    else if (Sector < 85)        {    j = 5;    }
    else if (Sector < 88)        {    j = 4;    }
    else if (Sector < 91)        {    j = 3;    }
    else if (Sector < 94)        {    j = 2;    }
    else if (Sector < 97)        {    j = 1;    }
    else                         {    j = 0;    }
    
    colors[13].red = Spi0.Data[291 + 3*j];
    colors[13].green = Spi0.Data[292 + 3*j];
    colors[13].blue = Spi0.Data[293 + 3*j];
    
    // Group8 - led [14] - Data[130 - 161] - 32 pixels
    // 4 - 3 - 3 - 3 - 3 - 3 - 3 - 3 - ...
    if (Sector < 4)              {    j = 31;    }
    else if (Sector < 7)         {    j = 30;    }
    else if (Sector < 10)        {    j = 29;    }
    else if (Sector < 13)        {    j = 28;    }
    else if (Sector < 16)        {    j = 27;    }
    else if (Sector < 19)        {    j = 26;    }
    else if (Sector < 22)        {    j = 25;    }
    else if (Sector < 25)        {    j = 24;    }
    else if (Sector < 29)        {    j = 23;    }
    else if (Sector < 32)        {    j = 22;    }
    else if (Sector < 35)        {    j = 21;    }
    else if (Sector < 38)        {    j = 20;    }
    else if (Sector < 41)        {    j = 19;    }
    else if (Sector < 44)        {    j = 18;    }
    else if (Sector < 47)        {    j = 17;    }
    else if (Sector < 50)        {    j = 16;    }
    else if (Sector < 54)        {    j = 15;    }
    else if (Sector < 57)        {    j = 14;    }
    else if (Sector < 60)        {    j = 13;    }
    else if (Sector < 63)        {    j = 12;    }
    else if (Sector < 66)        {    j = 11;    }
    else if (Sector < 69)        {    j = 10;    }
    else if (Sector < 72)        {    j = 9;    }
    else if (Sector < 75)        {    j = 8;    }
    else if (Sector < 79)        {    j = 7;    }
    else if (Sector < 82)        {    j = 6;    }
    else if (Sector < 85)        {    j = 5;    }
    else if (Sector < 88)        {    j = 4;    }
    else if (Sector < 91)        {    j = 3;    }
    else if (Sector < 94)        {    j = 2;    }
    else if (Sector < 97)        {    j = 1;    }
    else                         {    j = 0;    }
    
    colors[14].red = Spi0.Data[387 + 3*j];
    colors[14].green = Spi0.Data[388 + 3*j];
    colors[14].blue = Spi0.Data[389 + 3*j];
  }
}

void COLOR_Refresh_Test(void)
{
  unsigned char    i;
  rgb_color        color;
  byte tampon = Sector % 4;

  switch (tampon)
  {
    case 0:
    {
      color.red = 255;
      color.green = 255;
      color.blue = 255;
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

  for(i = 0; i < LED_COUNT; i++)
  {
    colors[i] = color;
  }
}
