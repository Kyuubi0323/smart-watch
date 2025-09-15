/**
 * @file example_usage.c
 * @brief Example usage of IDFLOG component with TimberClass
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "idflog.h"

// Custom log callback function
void custom_log_callback(idflog_level_t level, unsigned long timestamp, const char* message)
{
    // Example: You could send logs to a server, save to SD card, etc.
    printf("CALLBACK: [%lu] Level=%d Message=%s", timestamp, level, message);
}

void app_main(void)
{
    // Initialize the logging system (this also initializes the global slog instance)
    idflog_init();
    
    // Basic logging examples using slog (C-style)
    slog.i(&slog, "=== slog Component Example ===");
    
    // Test different log levels using the struct methods
    slog.d(&slog, "This is a debug message");
    slog.d_format(&slog, "Debug with value: %d", 42);
    
    slog.i(&slog, "This is an info message");
    slog.i_format(&slog, "Info with string: %s", "Hello World");
    
    slog.w(&slog, "This is a warning message");
    slog.w_format(&slog, "Warning with error code: %d", -1);
    
    slog.e(&slog, "This is an error message");
    slog.e_format(&slog, "Error occurred: %s", "File not found");
    
    slog.v(&slog, "This is a verbose message");
    slog.v_format(&slog, "Verbose details: %f", 3.14159);
    
    slog.wtf(&slog, "This is a WTF message");
    slog.wtf_format(&slog, "WTF! Critical error: %d", 999);
    
    // Demonstrate configuration options
    slog.i(&slog, "--- Testing configuration options ---");
    
    // Disable timestamps
    slog.show_time(&slog, false);
    slog.i(&slog, "This message has no timestamp");
    
    // Re-enable timestamps
    slog.show_time(&slog, true);
    slog.i(&slog, "This message has timestamp again");
    
    // Set custom callback
    slog.i(&slog, "Setting custom callback...");
    slog.set_log_callback(&slog, custom_log_callback);
    slog.i(&slog, "This message will also go to the callback");
    
    // Disable console output (only callback will receive logs)
    slog.i(&slog, "Disabling console output...");
    slog.set_print(&slog, false);
    slog.i(&slog, "This message only goes to callback"); // Won't appear in console
    
    // Re-enable console output
    slog.set_print(&slog, true);
    slog.i(&slog, "Console output re-enabled");
    
    // Remove callback
    slog.set_log_callback(&slog, NULL);
    slog.i(&slog, "Callback removed - back to console only");
    
    // Demonstrate using generic log function
    slog.log(&slog, IDFLOG_DEBUG, "Using generic log function");
    slog.log_format(&slog, IDFLOG_INFO, "Generic log with format: %s", "formatted text");
    
    // You can also use the standalone functions for compatibility
    IDFLOG_I("=== Standalone Functions Example ===");
    IDFLOG_D("Debug message: %d", 123);
    IDFLOG_W("Warning message");
    
    // Create your own slog instance
    slog_t my_logger;
    slog_init(&my_logger);
    
    // Configure it differently
    my_logger.show_time(&my_logger, false);
    my_logger.i(&my_logger, "Custom logger instance without timestamps");
    
    slog.i(&slog, "=== Example completed ===");
    
    // Keep the task running
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(2000));
        slog.d_format(&slog, "Heartbeat message every 2 seconds - tick: %lu", get_millis());
    }
}
