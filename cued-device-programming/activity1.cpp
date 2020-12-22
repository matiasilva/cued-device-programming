/*
1B Device Programming Coursework
Matias Silva (mw781)
Activity 1: Memory and interrupts
*/

#include "mbed.h"

#define LED_CYCLE_INTERVAL 1
#define NUMBER_SET_STATES 3
#define THRESHOLD_REALLOC 5
#define DEBOUNCE_TIME_INTERVAL 0.1
#define DOUBLE_CLICK_MAX_TIME_INTERVAL 0.4

InterruptIn button(USER_BUTTON);

DigitalOut led_1(LED1);
DigitalOut led_2(LED2);
DigitalOut led_3(LED3);

Ticker led_cycler_ticker;
Timeout button_debounce_timeout;
Timeout button_double_click_timeout;
Timeout led_player_timeout;
Timeout program_animation_timeout;

// these want to be in a separate header file
void on_cycler_ticker(void);
void set_leds(bool* state);
void on_button_press(void);
void on_button_stop_debouncing(void);
void on_button_double_click_timeout(void);
void start_recording(void);
void play_sequence(void);
void on_player_timeout(void);
void play_sequence_step(void);
void end_sequence(void);

// during recording: stores the current LED state
// during playback: stores the index in the queue
uint8_t current_state;
uint8_t program_state = 0;

bool has_been_pressed_before = false;

// maximum of 255 states and queue length
uint8_t* selected_led_queue;
uint8_t selected_led_queue_length = 0;

// use of const here is best practice
// but using it is troublesome for some reason
bool states[NUMBER_SET_STATES][3] =  {
    {true, false, false},
    {false, true, false},
    {false, false, true}
};

bool off_state[] = {false, false, false};
bool all_on_state[] = {true, true, true};

int main()
{
    // program setup/init
    // make LED states unambiguous from the start
    start_recording();
    
    if (selected_led_queue == NULL) {
        // ideally do something more useful here
        printf("memory allocation failed :(");
    }

    while(true) {
        if (program_state == 0) {
            // keep checking if we need to expand the array
            if (selected_led_queue_length != 0 && selected_led_queue_length % THRESHOLD_REALLOC == 0) {
                // exceeded queue size, need to increase size of "array"
                uint8_t new_queue_length = selected_led_queue_length + THRESHOLD_REALLOC;
                uint8_t *new_ptr = (uint8_t *) realloc(selected_led_queue, sizeof(uint8_t) * (new_queue_length));
                if (new_ptr == NULL) {
                    // memory reallocation failed, stop accepting inputs
                    // replay sequence
                    end_sequence();
                }
                selected_led_queue = new_ptr;
            }
        } else if (program_state == 1) {
            if(selected_led_queue != NULL) {
                free(selected_led_queue);
                selected_led_queue = NULL;
                start_recording();
                program_state = 0;
            }
        }

    }
}

void on_cycler_ticker(void)
{
    // cycles from 0 to 2
    if(++current_state % NUMBER_SET_STATES == 0) {
        current_state = 0;
    }
    set_leds(states[current_state]);
}

void on_button_press(void)
{
    // first click
    if(!has_been_pressed_before) {
        // access memory in next free address, set it to the current state
        *(selected_led_queue + selected_led_queue_length) = current_state;
        selected_led_queue_length++;

        button.rise(NULL);
        button_debounce_timeout.attach(on_button_stop_debouncing, DEBOUNCE_TIME_INTERVAL);

        button_double_click_timeout.attach(on_button_double_click_timeout, DOUBLE_CLICK_MAX_TIME_INTERVAL);
        has_been_pressed_before = true;
    } else {
        // double_click
        end_sequence();
        // set pressed flag to false to prevent further reclicks
        has_been_pressed_before = false;
    }
}

void on_button_stop_debouncing(void)
{
    button.rise(on_button_press);
}

void on_button_double_click_timeout(void)
{
    has_been_pressed_before = false;
}

void set_leds(bool* state)
{
    // map program state to LED states
    // const arrays are copied in
    // but potential efficiency improvement w pointers
    led_1 = state[0];
    led_2 = state[1];
    led_3 = state[2];
}

void start_recording(void)
{
    selected_led_queue_length = 0;
    current_state = -1;
    // start button interrupt and LED timer
    led_cycler_ticker.attach(on_cycler_ticker, LED_CYCLE_INTERVAL);
    button.rise(on_button_press);
    selected_led_queue = (uint8_t *) malloc(sizeof(uint8_t)*THRESHOLD_REALLOC);
}

void end_sequence(void)
{
    // disable all existing interrupts and timers
    led_cycler_ticker.detach();
    button.rise(NULL);

    play_sequence();
}

void play_sequence_step(void)
{
    uint8_t stored_state = *(selected_led_queue + current_state);
    set_leds(states[stored_state]);
    current_state++;
    led_player_timeout.attach(on_player_timeout, LED_CYCLE_INTERVAL);
}

void play_sequence(void)
{
    current_state = 0;
    set_leds(all_on_state);
    program_animation_timeout.attach(on_player_timeout, 0.5);
}

void on_player_timeout(void)
{
    if (current_state > selected_led_queue_length - 1) {
        program_state = 1;
    } else {
        play_sequence_step();
    }
}