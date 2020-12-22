// Tutorial task 1

#include "mbed.h"

// Green LED
DigitalOut led1(LED1);
// Blue LED
DigitalOut led2(LED2);
// Red LED
DigitalOut led3(LED3);


void select_led(int l)
{
    if (l==1) {
        led1 = true;
        led2 = false;
        led3 = false;
    } else if (l==2) {
        led1 = false;
        led2 = true;
        led3 = false;
    } else if (l==3) {
        led1 = false;
        led2 = false;
        led3 = true;
    } else if (l==0) {
        led1 = false;
        led2 = false;
        led3 = false;
    } else {
        led1 = true;
        led2 = true;
        led3 = true;
    }
}



int main()
{
    int t=0, f=0;
    while(true) {
        f=(t%5)-1;
        select_led(f);
        wait(0.5);
        t++;
    }
}

// Tutorial task 2

#include "mbed.h"

DigitalIn button(USER_BUTTON);
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);

int main()
{
  led1 = 1;
  led2 = 0;
  led3 = 1;
  while(true)
  {
    led1 = button == 0;
    led2 = !(button == 0);
    led3 = button == 0;
    wait(0.02); // 20 ms
  }
}