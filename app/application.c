#include <application.h>

// interval to get measurements
#define UPDATE_NORMAL_INTERVAL             (1 * 100)

// LED instance
bc_led_t led;

// Accelerometer
bc_lis2dh12_t acc;
bc_lis2dh12_result_g_t result;

// vector from accelerometer
float magnitude;
// biggest vector
float biggest_magnitude = 0;
// delta from stady state
float delta;
// helper variable to get maximum magnitude
float last_delta = 0;

// number of measurements before send
#define SEND_AFTER  10
// helper variable counting measurements
int counter;


void lis2_event_handler(bc_lis2dh12_t *self, bc_lis2dh12_event_t event, void *event_param)
{
    if (event == BC_LIS2DH12_EVENT_UPDATE)
    {
        // get data from accelerometer
        bc_lis2dh12_get_result_g(&acc, &result);

        // calculate vector of movement
        magnitude = pow(result.x_axis, 2) + pow(result.y_axis, 2) + pow(result.z_axis, 2);
        magnitude = sqrt(magnitude);
        
        // get difference from stady state
        delta = 1 - magnitude;

        // absolute delta
        if (delta < 0)
        {
            delta = delta * -1;
        }

        // we are looking for maximum number
        if (delta > last_delta)
        {
            biggest_magnitude = magnitude;
        }

        // we send data after SEND_AFTER count measurements       
        if (counter++ > SEND_AFTER)
        {
            // reset helper variables
            counter = 0;
            last_delta = 0;

            // blink LED
            bc_led_pulse(&led, 100);
            
            // send data
            bc_radio_pub_float("magnitude", &biggest_magnitude);
        }
    }
}

void application_init(void)
{
    // Initialize LED
    bc_led_init(&led, BC_GPIO_LED, false, false);
    bc_led_set_mode(&led, BC_LED_MODE_OFF);
    bc_led_pulse(&led, 1000);

    // Initialize accelerometer
    // https://en.wikipedia.org/wiki/Accelerometer
    bc_lis2dh12_init(&acc, BC_I2C_I2C0, 0x19);
    bc_lis2dh12_set_event_handler(&acc, lis2_event_handler, NULL);
    bc_lis2dh12_set_update_interval(&acc, UPDATE_NORMAL_INTERVAL);

    // Initialize radio
    bc_radio_init(BC_RADIO_MODE_NODE_SLEEPING);
    bc_radio_pairing_request("vibration-monitor", VERSION);
}

