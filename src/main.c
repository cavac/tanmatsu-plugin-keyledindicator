// SPDX-License-Identifier: MIT
// Tanmatsu Test Plugin - Blinking Status Indicator
//
// This plugin demonstrates the plugin system by showing a blinking
// red circle in the launcher's status bar.

#include "tanmatsu_plugin.h"

// Plugin state
static bool blink_state = false;
static int widget_id = -1;
static volatile bool running = true;

// Plugin metadata
static const plugin_info_t plugin_info = {
    .name = "Blink Indicator",
    .slug = "blink-indicator",
    .version = "1.0.0",
    .author = "Tanmatsu Test",
    .description = "Blinking red circle in status bar",
    .api_version = TANMATSU_PLUGIN_API_VERSION,
    .type = PLUGIN_TYPE_SERVICE,
    .flags = 0,
};

// Return plugin info
static const plugin_info_t* get_info(void) {
    return &plugin_info;
}

// Status widget callback - called during header render
static plugin_icontext_t status_widget_callback(void* user_data) {
    (void)user_data;

    // Draw blinking red circle when blink_state is true
    if (blink_state) {
        int x = plugin_status_get_draw_x();
        int y = plugin_status_get_draw_y() + 8;  // Center in header
        plugin_status_draw_circle(x + 8, y + 8, 6, 0xFFFF0000);  // Red circle
    }

    // Return empty - we draw directly
    return (plugin_icontext_t){.icon = NULL, .text = NULL};
}

// Plugin initialization
static int plugin_init(plugin_context_t* ctx) {
    (void)ctx;

    plugin_log_info("blink", "Blink indicator plugin initializing...");

    // Register our status widget
    widget_id = plugin_status_widget_register(status_widget_callback, NULL);
    if (widget_id < 0) {
        plugin_log_error("blink", "Failed to register status widget");
        return -1;
    }

    plugin_log_info("blink", "Blink indicator plugin initialized, widget_id=%d", widget_id);
    return 0;
}

// Plugin cleanup
static void plugin_cleanup(plugin_context_t* ctx) {
    (void)ctx;

    running = false;

    if (widget_id >= 0) {
        plugin_status_widget_unregister(widget_id);
        widget_id = -1;
    }

    plugin_log_info("blink", "Blink indicator plugin cleaned up");
}

// Service main loop - runs in its own FreeRTOS task
static void plugin_service_run(plugin_context_t* ctx) {
    (void)ctx;

    plugin_log_info("blink", "Blink service starting...");

    while (running) {
        // Toggle blink state
        blink_state = !blink_state;

        // Request display refresh (triggers status bar redraw)
        plugin_display_flush();

        // Wait 500ms
        plugin_delay_ms(500);
    }

    plugin_log_info("blink", "Blink service stopped");
}

// Plugin entry point structure
static const plugin_entry_t entry = {
    .get_info = get_info,
    .init = plugin_init,
    .cleanup = plugin_cleanup,
    .menu_render = NULL,    // Not a menu plugin
    .menu_select = NULL,    // Not a menu plugin
    .service_run = plugin_service_run,
    .hook_event = NULL,     // Not a hook plugin
};

// Register this plugin with the host
TANMATSU_PLUGIN_REGISTER(entry);
