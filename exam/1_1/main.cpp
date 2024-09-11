#include "mbed.h"
#include "uLCD_4DGL.h"
#include "math.h"

uLCD_4DGL uLCD(D1, D0, D2);
AnalogOut aout(D7);
//AnalogIn up(A0);
//AnalogIn down(A1);
//AnalogIn sel(A2);;
AnalogIn ain(A3);
InterruptIn up(A0);
InterruptIn down(A1);
InterruptIn sel(A2);

//float S, U, D;
float Freq[4];
int f = 0;
int done = 0;
Thread t(osPriorityLow);
Thread twave(osPriorityNormal);
EventQueue queue(32 * EVENTS_EVENT_SIZE);
EventQueue queuewave(32 * EVENTS_EVENT_SIZE);


/*
class Counter {
public:
    Counter(PinName pin): _interrupt(pin) {
        _interrupt.rise(callback(this, &Counter::increment));
    }
    void increment
}*/

void upup()
{
    if (down == 0) {
        if (f + 1 < 4) {
            f++;
            uLCD.cls();
            uLCD.text_height(1);

            uLCD.printf("\nSelect a freq.");

            uLCD.text_height(4);
            uLCD.locate(2, 2);
            uLCD.printf("%f Hz", Freq[f]);
        } else {
            uLCD.locate(0, 1);
            uLCD.text_height(1);

            uLCD.printf("Can't be larger.\n");


            uLCD.text_height(4);
            uLCD.locate(2, 2);
            uLCD.printf("%f Hz", Freq[f]);
        }
    }
}



void downdown()
{
    if (down == 0) {
        if (f - 1 >= 0) {
            f--;
            uLCD.cls();
            uLCD.text_height(1);
            uLCD.text_height(1);
            uLCD.printf("\nSelect a freq.");

            uLCD.text_height(4);
            uLCD.locate(2, 2);
            uLCD.printf("%f Hz", Freq[f]);
        } else {
            uLCD.locate(0, 1);
            uLCD.text_height(1);

            uLCD.printf("Can't be smaller.\n");
            uLCD.text_height(4);

            uLCD.locate(2, 2);
            uLCD.printf("%f Hz", Freq[f]);
        }
    }
}

void wave() 
{
    int delay_time = 0;
    float ADCdata[480];
    int cnt;
    float step_up, step_down;
    float height;
    int j;
    float waittime;
    /*if (f == 0) delay_time = ;
    else if (f == 1) delay_time = 13;
    else if (f == 2) delay_time = 4;
    else if (f == 3) */
    delay_time = 1000;
    if (f == 0) {
        step_up = 1/11.0;
        step_down = 1/11.0;
        waittime = 220.0;
    } else if (f == 1) {
        step_up = 1/22.0;
        step_down = 1/22.0;
        waittime = 200.0;
    } else if (f == 2) {
        step_up = 1/44.0;
        step_down = 1/44.0;
        waittime = 160.0;
    } else if (f == 3) {
        step_up = 1/88.0;
        step_down = 1/88.0;
        waittime = 80.0;
    }

    height = 3/3.3;
    j = 0;
    cnt = 0;
    while(1) {
        for (float i = 0.0; i < height; i += step_up) {
            aout = i;
            if (j < 480) {
                ADCdata[j] = ain;
                j++;
            }
            wait_us(delay_time);
        }
        for (float i = 0.0; i < waittime; i += 1.0) {
            aout = height;
            if (j < 480) {
                ADCdata[j] = ain;
                j++;
            }
            wait_us(delay_time);
        }
        for (float i = height; i > 0.0; i -= step_down) {
            aout = i;
            if (j < 480) {
                ADCdata[j] = ain;
                j++;
            }
            wait_us(delay_time);
        }
        if (!cnt) {
            if (j == 480) {
                cnt++;
                for (int k = 0; k < 480; k++) {
                    printf("%f\r\n", ADCdata[k]);
                    ThisThread::sleep_for(100ms);
                }
            }
        }

    }
}

void selsel()
{
    if (down == 0) {
        uLCD.cls();
        uLCD.text_height(1);

        uLCD.printf("\nSelect a freq.");

        uLCD.text_height(4);

        uLCD.locate(2, 2);
        uLCD.printf("%f Hz", Freq[f]);
        //ThisThread::sleep_for(2ms);
        queuewave.call(wave);
    }
    down = 1;
}

/*void upupone() {
    queue.call(upup);
}

void downdownone() {
    queue.call(downdown);
}

void selselone (){
    queue.call(selsel);
}*/

int main()
{
    Freq[0] = 1/8;
    Freq[1] = 1/4;
    Freq[2] = 1/2;
    Freq[3] = 1;
    /*int delay_time = 0;
    float ADCdata[300];
    int cnt;
    float step_up, step_down;
    float height;
    int j;*/

//    unit16_t sample = 0;
    // I have to set the frequency range and button connection

    Thread t(osPriorityLow);
    Thread twave(osPriorityNormal);
    t.start(callback(&queue, &EventQueue::dispatch_forever));
    twave.start(callback(&queuewave, &EventQueue::dispatch_forever));
    uLCD.printf("\nSelect a freq.\n");
    uLCD.text_height(4);
    uLCD.text_height(4);
    uLCD.locate(2, 2);
    uLCD.printf("%f Hz", Freq[f]);
    // Analog input and LCD
    up.rise(queue.event(upup));
    down.rise(queue.event(downdown));
    sel.rise(queue.event(selsel));
    
   // f = 1;
   // void wave() {
    //int delay_time = 0;
    //float ADCdata[300];
    //int cnt;
}
    /*if (f == 0) delay_time = 85;
    else if (f == 1) delay_time = 13;
    else if (f == 2) delay_time = 4;
    // make triangle waveform and frequency is selected above
    // Analog output
    step_up = 1/77.0;
    step_down = 1/33.0;
    height = 3/3.3;
    j = 0;
    cnt = 0;
    printf("%d\r\n", Freq[f]);
    while(1) {
        for (float i = 0.0; i < height; i += step_up) {
            aout = i;
            if (j < 300) {
                ADCdata[j] = ain;
                j++;
            }
            wait_us(delay_time);
        }
        for (float i = height; i > 0.0; i -= step_down) {
            aout = i;
            if (j < 300) {
                ADCdata[j] = ain;
                j++;
            }
            wait_us(delay_time);
        }
        if (!cnt) {
            if (j == 300) {
                cnt++;
                for (int k = 0; k < 300; k++) {
                    printf("%f\r\n", ADCdata[k]);
                    ThisThread::sleep_for(100ms);
                }
            }
        }

    }*/



}
