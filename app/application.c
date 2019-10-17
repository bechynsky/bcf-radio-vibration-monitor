#include <application.h>

#define TEMPERATURE_TAG_PUB_NO_CHANGE_INTEVAL (15 * 60 * 1000)
#define TEMPERATURE_TAG_PUB_VALUE_CHANGE 0.2f

#define UPDATE_NORMAL_INTERVAL             (5 * 1000)

// LED instance
bc_led_t led;

// Accelerometer
bc_lis2dh12_t acc;
bc_lis2dh12_result_g_t result;

float magnitude;
float last_magnitude;

bc_tick_t next_pub;

void lis2_event_handler(bc_lis2dh12_t *self, bc_lis2dh12_event_t event, void *event_param)
{
    if (event == BC_LIS2DH12_EVENT_UPDATE)
    {

        bc_lis2dh12_get_result_g(&acc, &result);

        magnitude = pow(result.x_axis, 2) + pow(result.y_axis, 2) + pow(result.z_axis, 2);
        magnitude = sqrt(magnitude);
        
        if ((fabs(magnitude - last_magnitude) >= TEMPERATURE_TAG_PUB_VALUE_CHANGE) || (next_pub < bc_scheduler_get_spin_tick()))
        {
            // Make longer pulse when transmitting
            bc_led_pulse(&led, 100);

            bc_radio_pub_float("magnitude", &magnitude);
            last_magnitude = magnitude;
            next_pub = bc_scheduler_get_spin_tick() + TEMPERATURE_TAG_PUB_NO_CHANGE_INTEVAL;
        }

    
    }
}
void application_init(void)
{
    // Initialize logging
    //bc_log_init(BC_LOG_LEVEL_DUMP, BC_LOG_TIMESTAMP_ABS);

    // Initialize LED
    bc_led_init(&led, BC_GPIO_LED, false, false);
    bc_led_set_mode(&led, BC_LED_MODE_OFF);
    bc_led_pulse(&led, 1000);

    bc_radio_init(BC_RADIO_MODE_NODE_SLEEPING);
    bc_radio_pairing_request("printer-vibration-monitor", VERSION);

    
    bc_lis2dh12_init(&acc, BC_I2C_I2C0, 0x19);
    bc_lis2dh12_set_event_handler(&acc, lis2_event_handler, NULL);
    bc_lis2dh12_set_update_interval(&acc, UPDATE_NORMAL_INTERVAL);
}

