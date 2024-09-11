#include "mbed.h"

#define CENTER_BASE 1500

BufferedSerial pc(USBTX, USBRX);
DigitalIn encoder(D10);
PwmOut servo(D11);

Timer t;
Ticker encoder_ticker;

volatile int steps;
volatile int last;

void servo_control(int speed) {
   if (speed > 200)       speed = 200;
   else if (speed < -200) speed = -200;

   servo=(CENTER_BASE + speed)/20000.0f;
}

void encoder_control() {
   int value = encoder;
   if(!last && value) steps++;
   last = value;
}


int main() {

   pc.set_baud(9600);

   encoder_ticker.attach(&encoder_control, .01);

   servo.period_ms(20);
   int num1 = 5, num2 = 5;
   while(num1 > 0) {
   //TODO: revise this value according to your result
   num1 -= 1;
   servo_control(-27.245);

   steps = 0;
   t.reset();
   t.start();

   ThisThread::sleep_for(1000ms);

   float time = t.read();

   printf("%1.3f\r\n", (float) steps * 6.5 * 3.14 / 32 / time);

   }
   while(num2 > 0) {
   //TODO: revise this value according to your result
   num2 -= 1;
   servo_control(44.758);

   steps = 0;
   t.reset();
   t.start();

   ThisThread::sleep_for(1000ms);

   float time = t.read();

   printf("%1.3f\r\n", (float) steps * 6.5 * 3.14 / 32 / time);

   }
   return 0;
/*
   while(1) {

   //TODO: revise this value according to your result

   servo_control(-27.245);

   steps = 0;
   t.reset();
   t.start();

   ThisThread::sleep_for(8000ms);

   float time = t.read();

   printf("%1.3f\r\n", (float) steps * 6.5 * 3.14 / 32 / time);

   }*/

}