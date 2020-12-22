/*
1B Device Programming Coursework
Matias Silva (mw781)
Activity 2: I2C bus and sensors
*/

#include "mbed.h"

#define LM75_REG_TEMP (0x00) // Temperature Register
#define LM75_REG_CONF (0x01) // Configuration Register
#define LM75_ADDR     (0x90) // LM75 address

#define LM75_REG_TOS (0x03) // TOS register
#define LM75_REG_THYST (0x02) // THYST register

#define TEMPERATURE_BUFFER_SIZE 60
#define ALARM_RING_INTERVAL 0.1

I2C i2c(I2C_SDA, I2C_SCL);

DigitalOut led_1(LED1);
DigitalOut led_2(LED2);
DigitalOut led_3(LED3);

InterruptIn lm75_int(D7);

/*
LM75 pin configuration
GND - GND
VCC - 3.3V
SDA - D14
SCL - D15
OS - D7
*/

Serial pc(SERIAL_TX, SERIAL_RX);

float temperatures[TEMPERATURE_BUFFER_SIZE];
uint8_t buffer_index;

uint8_t alarm_ring_index = 0;
Ticker alarm_ring_ticker;

// flag to control whether data should be flushed to serial port
bool wants_data_flushed = false;
// flag to control behaviors on interrupt trigger
bool has_been_triggered = false;

int16_t i16;

bool states[2][3] =  {
    {true, true, true},
    {false, false, false},
};

bool off_state[] = {false, false, false};

void set_leds(bool* state)
{
    led_1 = state[0];
    led_2 = state[1];
    led_3 = state[2];
}

void add_temp(float t)
{
    for(int i = 0; i < TEMPERATURE_BUFFER_SIZE - 1; i++) {
        temperatures[i] = temperatures[i + 1];
    }
    temperatures[TEMPERATURE_BUFFER_SIZE - 1] = t;
}

void on_alarm_ring(void)
{
    set_leds(states[alarm_ring_index % 2]);
    if(alarm_ring_index % 2 == 0) {
        alarm_ring_index = 0;
    }
    alarm_ring_index++;
}

void sound_the_alarm()
{
    if(has_been_triggered) {
        set_leds(off_state);
        alarm_ring_ticker.detach();
        alarm_ring_index = 0;
        has_been_triggered = false;
    } else {
        alarm_ring_ticker.attach(on_alarm_ring, ALARM_RING_INTERVAL);
        has_been_triggered = true;
        wants_data_flushed = true;
    }
}

int main()
{   
    char data_write[3];
    char data_read[3];

    /* Configure the Temperature sensor device STLM75:
       - Thermostat mode Interrupt
       - Fault tolerance: 0
       - Interrupt mode means that the line will trigger when you exceed TOS and stay triggered until a register is read - see data sheet
    */
    data_write[0] = LM75_REG_CONF;
    data_write[1] = 0x02;
    int status = i2c.write(LM75_ADDR, data_write, 2, 0);
    if (status != 0) {
        // Error
        while (1) {
            led_1 = !led_1;
            wait(0.2);
        }
    }

    float tos=28; // TOS temperature
    float thyst=26; // THYST tempertuare

    // This section of code sets the TOS register
    data_write[0]=LM75_REG_TOS;
    i16 = (int16_t)(tos*256) & 0xFF80;
    data_write[1]=(i16 >> 8) & 0xff;
    data_write[2]=i16 & 0xff;
    i2c.write(LM75_ADDR, data_write, 3, 0);

    //This section of codes set the THYST register
    data_write[0]=LM75_REG_THYST;
    i16 = (int16_t)(thyst*256) & 0xFF80;
    data_write[1]=(i16 >> 8) & 0xff;
    data_write[2]=i16 & 0xff;
    i2c.write(LM75_ADDR, data_write, 3, 0);

    // This line attaches the interrupt.
    // The interrupt line is active low so we trigger on a falling edge
    lm75_int.fall(sound_the_alarm);

    while (1) {
        // Read temperature register
        data_write[0] = LM75_REG_TEMP;
        i2c.write(LM75_ADDR, data_write, 1, 1); // no stop
        i2c.read(LM75_ADDR, data_read, 2, 0);

        // Calculate temperature value in Celcius
        int16_t i16 = (data_read[0] << 8) | data_read[1];
        // Read data as twos complement integer so sign is correct
        float temp = i16 / 256.0;

        // Display result
        pc.printf("Temperature = %.3f\r\n",temp);

        // Store results
        add_temp(temp);

        led_1 = !led_1;
        wait(1.0);

        if(wants_data_flushed) {
            for(int i = TEMPERATURE_BUFFER_SIZE - 1; i >= 0; i--) {
                pc.printf("%.3f", temperatures[i]);
                if(i > 0) {
                    pc.printf(",");
                }
            }
            pc.printf("\r\n");
            wants_data_flushed = false;
        }
    }

}