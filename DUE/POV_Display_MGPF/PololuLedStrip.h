#pragma once

#include <Arduino.h>

void Motor_Run(void);

namespace Pololu
{
  #ifndef _POLOLU_RGB_COLOR
  #define _POLOLU_RGB_COLOR
  typedef struct rgb_color
  {
    unsigned char red, green, blue;
    rgb_color() {};
    rgb_color(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b) {};
  } rgb_color;
  #endif

  class PololuLedStripBase
  {
    public:
    static bool interruptFriendly;
    void virtual write(rgb_color *, unsigned int count) = 0;
  };

  template<unsigned char pin> class PololuLedStrip : public PololuLedStripBase
  {
    public:
    void virtual write(rgb_color *, unsigned int count);
  };

  template<unsigned char pin> void __attribute__((aligned(16))) PololuLedStrip<pin>::write(rgb_color * colors, unsigned int count)
  {
    Pio * port = g_APinDescription[pin].pPort;
    uint32_t pinValue = g_APinDescription[pin].ulPin;
    PIO_SetOutput(port, pinValue, LOW, 0, 0);

    __disable_irq();   // Disable interrupts temporarily because we don't want our pulse timing to be messed up.

    while(count--)
    {
      // Send a color to the LED strip.
      // The assembly below also increments the 'colors' pointer,
      // it will be pointing to the next color at the end of this loop.
      
      asm volatile(
        "ldrb r12, [%0, #1]\n"    // Load green.
        "lsls r12, r12, #24\n"    // Put green in MSB of color register.
        "ldrb r3, [%0, #0]\n"     // Load red.
        "lsls r3, r3, #16\n"
        "orrs r12, r12, r3\n"     // Put red in color register.
        "ldrb r3, [%0, #2]\n"     // Load blue.
        "lsls r3, r3, #8\n"
        "orrs r12, r12, r3\n"     // Put blue in LSB of color register.
        "rbit r12, r12\n"         // Reverse the bits so we can use right rotations.
        "adds  %0, %0, #3\n"      // Advance pointer to next color.
    
        "mov r3, #24\n"           // Initialize the loop counter register.

        "send_led_strip_bit%=:\n"
        "str %[val], %[set]\n"            // Drive the line high.
        "rrxs r12, r12\n"                 // Rotate right through carry.

        "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
        "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
        "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"

        "it cc\n" "strcc %[val], %[clear]\n"  // If the bit to send is 0, set the line low now.

        "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
        "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
        "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
        "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"

        "it cs\n" "strcs %[val], %[clear]\n"  // If the bit to send is 1, set the line low now.

        "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
        "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"

        "sub r3, r3, #1\n"                // Decrement the loop counter.
        "cbz r3, led_strip_asm_end%=\n"   // If we have sent 24 bits, go to the end.
        "b send_led_strip_bit%=\n"

        "led_strip_asm_end%=:\n"

      : "=r" (colors)
      : "0" (colors),
        [set] "m" (port->PIO_SODR),
        [clear] "m" (port->PIO_CODR),
        [val] "r" (pinValue)
      : "r3", "r12", "cc"
      );

      if (PololuLedStripBase::interruptFriendly)
      {
        // Experimentally on an AVR we found that one NOP is required after the SEI to actually let the
        // interrupts fire.
        __enable_irq();
        Motor_Run();      
		__disable_irq();
      }
    }
    __enable_irq();         // Re-enable interrupts now that we are done.
    //delayMicroseconds(50);  // Send the reset signal.
  }
}

using namespace Pololu;
