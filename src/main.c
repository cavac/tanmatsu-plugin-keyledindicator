// SPDX-License-Identifier: MIT
// Tanmatsu Test Plugin - Key Press LED Indicator
//
// This plugin demonstrates input hooks and LED control.
// When any key is pressed, LED 5 blinks white for 100ms.
// Key presses are forwarded normally to the launcher.

#include "tanmatsu_plugin.h"

// Plugin state
static int hook_id = -1;
static int widget_id = -1;
static volatile uint32_t led_off_time = 0;  // Tick when LED should turn off
static volatile bool led_active = false;

// Status widget callback - returns a red circle icon when running
static plugin_icontext_t status_widget_callback(void* user_data) {
    (void)user_data;
    // Return a simple text indicator (no icon)
    static char status_text[] = "(*)";
    return (plugin_icontext_t){ .icon = NULL, .text = status_text };
}

// Plugin metadata
static const plugin_info_t plugin_info = {
    .name = "Key LED Indicator",
    .slug = "key-led-indicator",
    .version = "1.0.0",
    .author = "Tanmatsu Test",
    .description = "LED 5 blinks white on keypress",
    .api_version = TANMATSU_PLUGIN_API_VERSION,
    .type = PLUGIN_TYPE_SERVICE,
    .flags = 0,
};

// Return plugin info
static const plugin_info_t* get_info(void) {
    return &plugin_info;
}

// Input hook callback - called for every input event
static bool input_hook_callback(plugin_input_event_t* event, void* user_data) {
    (void)user_data;

    // Only react to key press events (state == true), not releases
    if (event->state) {
        // Turn on LED 5 white
        plugin_led_set_pixel_rgb(5, 255, 255, 255);
        plugin_led_send();

        // Set time to turn off LED (current time + 100ms)
        led_off_time = plugin_get_tick_ms() + 100;
        led_active = true;
    }

    // Return false - don't consume the event, let it pass through to the launcher
    return false;
}

// Plugin initialization
static int plugin_init(plugin_context_t* ctx) {
    (void)ctx;

    plugin_log_info("keyled", "Key LED plugin initializing...");

    // Register input hook
    hook_id = plugin_input_hook_register(input_hook_callback, NULL);
    if (hook_id < 0) {
        plugin_log_error("keyled", "Failed to register input hook");
        return -1;
    }

    // Register status widget to show plugin is running
    widget_id = plugin_status_widget_register(status_widget_callback, NULL);
    if (widget_id < 0) {
        plugin_log_error("keyled", "Failed to register status widget");
    }

    plugin_log_info("keyled", "Key LED plugin initialized, hook_id=%d, widget_id=%d", hook_id, widget_id);
    return 0;
}

// Plugin cleanup
static void plugin_cleanup(plugin_context_t* ctx) {
    (void)ctx;

    // Unregister status widget
    if (widget_id >= 0) {
        plugin_status_widget_unregister(widget_id);
        widget_id = -1;
    }

    // Unregister input hook
    if (hook_id >= 0) {
        plugin_input_hook_unregister(hook_id);
        hook_id = -1;
    }

    // Turn off LED 5
    plugin_led_set_pixel_rgb(5, 0, 0, 0);
    plugin_led_send();

    plugin_log_info("keyled", "Key LED plugin cleaned up");
}

// Service main loop - monitors LED timing
static void plugin_service_run(plugin_context_t* ctx) {
    plugin_log_info("keyled", "Key LED service starting...");

    while (!plugin_should_stop(ctx)) {
        // Check if LED needs to be turned off
        if (led_active) {
            uint32_t now = plugin_get_tick_ms();
            if (now >= led_off_time) {
                // Turn off LED 5
                plugin_led_set_pixel_rgb(5, 0, 0, 0);
                plugin_led_send();
                led_active = false;
            }
        }

        // Short sleep to avoid busy loop
        plugin_delay_ms(10);
    }

    plugin_log_info("keyled", "Key LED service stopped");
}

// Plugin entry point structure
static const plugin_entry_t entry = {
    .get_info = get_info,
    .init = plugin_init,
    .cleanup = plugin_cleanup,
    .menu_render = NULL,
    .menu_select = NULL,
    .service_run = plugin_service_run,
    .hook_event = NULL,
};

// Register this plugin with the host
TANMATSU_PLUGIN_REGISTER(entry);
