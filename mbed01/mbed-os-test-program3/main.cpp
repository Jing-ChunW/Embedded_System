#include "mbed.h"

DigitalOut myLED(LED3);
DigitalOut myLED2(LED1);

void Led(DigitalOut &ledpin);
void Led2(DigitalOut &ledpin);

int main()
{
   myLED = 1;
   myLED2 = 1;
   while (true)
   {
      Led(myLED);
      Led2(myLED2);
   }
}