/* Arduino WS2801 Library
 * Copyright (C) 2018 by MisterBrowny
*/

#ifndef WS2801_h
#define WS2801_h

typedef struct rgb_color
{
  unsigned char red, green, blue;
  rgb_color() {};
  rgb_color(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b) {};
} rgb_color;

/** read pin value
 * @param[in] pin Arduino pin number
 * @return value read
 */
inline __attribute__((always_inline))
bool fastDigitalRead(uint8_t pin) {
  return g_APinDescription[pin].pPort->PIO_PDSR & g_APinDescription[pin].ulPin;
}

/** Set pin value
 * @param[in] pin Arduino pin number
 * @param[in] level value to write
 */
inline __attribute__((always_inline))
void fastDigitalWrite(uint8_t pin, bool value) {
  if (value) {
    g_APinDescription[pin].pPort->PIO_SODR = g_APinDescription[pin].ulPin;
  } else {
    g_APinDescription[pin].pPort->PIO_CODR = g_APinDescription[pin].ulPin;
  }
}

/** set pin configuration
 * @param[in] pin Arduino pin number
 * @param[in] mode mode INPUT or OUTPUT.
 * @param[in] level If mode is output, set level high/low.
 *                  If mode is input, enable or disable the pin's 20K pullup.
 */
#define fastPinConfig(pin, mode, level)\
  {pinMode(pin, mode); fastDigitalWrite(pin, level);}
  


template<uint8_t SdaPin, uint8_t SckPin>
class SPI_WS2801 {
 public:
  //----------------------------------------------------------------------------
  /** Initialize SPI_WS2801 pins. */
  void begin() {
    fastPinConfig(SdaPin, OUTPUT, LOW);
    fastPinConfig(SckPin, OUTPUT, LOW);
  }
	//----------------------------------------------------------------------------
  /** SPI_WS2801 send bit.
  * @param[in] bit bit to send. 
  * @param[in] data value.
  */
  inline __attribute__((always_inline))
  void sendBit(uint8_t bit, uint8_t data) {
    //fastDigitalWrite(SckPin, 0);
    fastDigitalWrite(SdaPin, data & (1 << bit));
    fastDigitalWrite(SckPin, HIGH);
    asm volatile ("nop\n");
    asm volatile ("nop\n");
    asm volatile ("nop\n");
    asm volatile ("nop\n");
    asm volatile ("nop\n");
    asm volatile ("nop\n");
    asm volatile ("nop\n");
    asm volatile ("nop\n");
    asm volatile ("nop\n");
    asm volatile ("nop\n");
    asm volatile ("nop\n");
    asm volatile ("nop\n");
    fastDigitalWrite(SckPin, LOW);
  }
  //----------------------------------------------------------------------------
  /** SPI_WS2801 send byte.
  * @param[in] data Data byte to send.
  */
  inline __attribute__((always_inline))
  void send(uint8_t data) {
    sendBit(7, data);
    sendBit(6, data);
    sendBit(5, data);
    sendBit(4, data);
    sendBit(3, data);
    sendBit(2, data);
    sendBit(1, data);
    sendBit(0, data);
  }

  //----------------------------------------------------------------------------
  /** SPI_WS2801 send color.
  * @param[in] color Color to send.
  */
  inline __attribute__((always_inline))
  void write(rgb_color * color) {
    //__disable_irq();
    send(color->red);
    send(color->blue);
    send(color->green);
    //__enable_irq();
  }

  //----------------------------------------------------------------------------
  /** SPI_WS2801 send colors.
  * @param[in] colors Colors to send.
  * @param[in] count Number of Color to send.
  */
  inline __attribute__((always_inline))
  void writeStrip(rgb_color * colors, unsigned int count) {
    int i = 0;
    __disable_irq();
    while(count --)
    {
      write(&colors[i]);
      i ++;
    }
    __enable_irq();
  }
};
#endif // WS2801_h
