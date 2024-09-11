#include "mbed.h"

void Led2(DigitalOut &ledpin)
{
   for (int i = 0; i < 4; ++i)
   {                     //blink for 10 times
      ledpin = !ledpin; // toggle led
      ThisThread::sleep_for(100ms);
   }
}