/**
 * @file idflog.c
 * @brief ESP-IDF Logging Component Implementation - C struct with function pointers
 */

#include "idflog.h"
#include <sys/time.h>

static const char *TAG = "IDFLOG";

// Static variables for standalone functions
static bool print_enabled = true;
static bool show_time_enabled = true;
static idflog_callback_t log_callback = NULL;

/**
 * @brief Get current timestamp in milliseconds (equivalent to Arduino millis())
 */
unsigned long get_millis(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (unsigned long)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

/**
 * @brief Get level name string with brackets (slog style)
 */
static const char* slog_level_name(idflog_level_t level)
{
    switch (level) {
        case IDFLOG_DEBUG:   return "[DEBUG]";
        case IDFLOG_INFO:    return "[INFO]";
        case IDFLOG_ERROR:   return "[ERROR]";
        case IDFLOG_VERBOSE: return "[VERBOSE]";
        case IDFLOG_WARNING: return "[WARNING]";
        case IDFLOG_WTF:     return "[WTF]";
        default:             return "[UNKNOWN]";
    }
}

// ============================================================================
// slog method implementations
// ============================================================================

/**
 * @brief Set the print state
 * @param self Pointer to slog instance
 * @param state The state
 */
static void slog_set_print(slog_t* self, bool state)
{
    self->_print = state;
}

/**
 * @brief Set whether to show time
 * @param self Pointer to slog instance
 * @param state The state
 */
static void slog_show_time(slog_t* self, bool state)
{
    self->_time = state;
}

/**
 * @brief Set the logging callback
 * @param self Pointer to slog instance
 * @param callback The callback function
 */
static void slog_set_log_callback(slog_t* self, idflog_callback_t callback)
{
    self->logging_callback = callback;
}

/**
 * @brief Send the logs to callback or print
 * @param self Pointer to slog instance
 * @param level The level type
 * @param message The message
 */
static void slog_send_logs(slog_t* self, idflog_level_t level, const char* message)
{
    unsigned long time = get_millis();
    char msg[512];
    
    if (self->_time) {
        snprintf(msg, sizeof(msg), "%lu %s: %s\n", time, slog_level_name(level), message);
    } else {
        snprintf(msg, sizeof(msg), "%s: %s\n", slog_level_name(level), message);
    }
    
    if (self->logging_callback != NULL) {
        self->logging_callback(level, time, msg);
    }
    
    if (self->_print) {
        printf("%s", msg);
    }
}

/**
 * @brief Log debug message
 * @param self Pointer to slog instance
 * @param msg The debug message
 */
static void slog_d(slog_t* self, const char* msg)
{
    slog_send_logs(self, IDFLOG_DEBUG, msg);
}

/**
 * @brief Log debug with format
 * @param self Pointer to slog instance
 * @param format The message format
 * @param ... The arguments
 */
static void slog_d_format(slog_t* self, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    
    slog_send_logs(self, IDFLOG_DEBUG, buffer);
    
    va_end(args);
}

/**
 * @brief Log error message
 * @param self Pointer to slog instance
 * @param msg The error message
 */
static void slog_e(slog_t* self, const char* msg)
{
    slog_send_logs(self, IDFLOG_ERROR, msg);
}

/**
 * @brief Log error with format
 * @param self Pointer to slog instance
 * @param format The message format
 * @param ... The arguments
 */
static void slog_e_format(slog_t* self, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    
    slog_send_logs(self, IDFLOG_ERROR, buffer);
    
    va_end(args);
}

/**
 * @brief Log info message
 * @param self Pointer to slog instance
 * @param msg The info message
 */
static void slog_i(slog_t* self, const char* msg)
{
    slog_send_logs(self, IDFLOG_INFO, msg);
}

/**
 * @brief Log info with format
 * @param self Pointer to slog instance
 * @param format The message format
 * @param ... The arguments
 */
static void slog_i_format(slog_t* self, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    
    slog_send_logs(self, IDFLOG_INFO, buffer);
    
    va_end(args);
}

/**
 * @brief Log verbose message
 * @param self Pointer to slog instance
 * @param msg The verbose message
 */
static void slog_v(slog_t* self, const char* msg)
{
    slog_send_logs(self, IDFLOG_VERBOSE, msg);
}

/**
 * @brief Log verbose with format
 * @param self Pointer to slog instance
 * @param format The message format
 * @param ... The arguments
 */
static void slog_v_format(slog_t* self, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    
    slog_send_logs(self, IDFLOG_VERBOSE, buffer);
    
    va_end(args);
}

/**
 * @brief Log warning message
 * @param self Pointer to slog instance
 * @param msg The warning message
 */
static void slog_w(slog_t* self, const char* msg)
{
    slog_send_logs(self, IDFLOG_WARNING, msg);
}

/**
 * @brief Log warning with format
 * @param self Pointer to slog instance
 * @param format The message format
 * @param ... The arguments
 */
static void slog_w_format(slog_t* self, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    
    slog_send_logs(self, IDFLOG_WARNING, buffer);
    
    va_end(args);
}

/**
 * @brief Log wtf message
 * @param self Pointer to slog instance
 * @param msg The wtf message
 */
static void slog_wtf(slog_t* self, const char* msg)
{
    slog_send_logs(self, IDFLOG_WTF, msg);
}

/**
 * @brief Log wtf with format
 * @param self Pointer to slog instance
 * @param format The message format
 * @param ... The arguments
 */
static void slog_wtf_format(slog_t* self, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    
    slog_send_logs(self, IDFLOG_WTF, buffer);
    
    va_end(args);
}

/**
 * @brief Generic log function
 * @param self Pointer to slog instance
 * @param level The level type
 * @param msg The message
 */
static void slog_log(slog_t* self, idflog_level_t level, const char* msg)
{
    slog_send_logs(self, level, msg);
}

/**
 * @brief Generic log function with format
 * @param self Pointer to slog instance
 * @param level The level type
 * @param format The message format
 * @param ... The arguments
 */
static void slog_log_format(slog_t* self, idflog_level_t level, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    
    slog_send_logs(self, level, buffer);
    
    va_end(args);
}

// ============================================================================
// Public functions
// ============================================================================

/**
 * @brief Initialize a slog instance
 * @param slog_instance Pointer to slog instance to initialize
 */
void slog_init(slog_t* slog_instance)
{
    // Initialize member variables
    slog_instance->_print = true;
    slog_instance->_time = true;
    slog_instance->logging_callback = NULL;
    
    // Initialize function pointers
    slog_instance->set_print = slog_set_print;
    slog_instance->show_time = slog_show_time;
    slog_instance->set_log_callback = slog_set_log_callback;
    
    slog_instance->d = slog_d;
    slog_instance->d_format = slog_d_format;
    slog_instance->e = slog_e;
    slog_instance->e_format = slog_e_format;
    slog_instance->i = slog_i;
    slog_instance->i_format = slog_i_format;
    slog_instance->v = slog_v;
    slog_instance->v_format = slog_v_format;
    slog_instance->w = slog_w;
    slog_instance->w_format = slog_w_format;
    slog_instance->wtf = slog_wtf;
    slog_instance->wtf_format = slog_wtf_format;
    
    slog_instance->log = slog_log;
    slog_instance->log_format = slog_log_format;
}

// Global slog instance
slog_t slog;

void idflog_init(void)
{
    // Initialize the global slog instance
    slog_init(&slog);
    ESP_LOGI(TAG, "IDFLOG component initialized with slog");
}

// ============================================================================
// Standalone functions (for backward compatibility)
// ============================================================================

void idflog_set_print(bool state)
{
    print_enabled = state;
}

void idflog_show_time(bool state)
{
    show_time_enabled = state;
}

void idflog_set_callback(idflog_callback_t callback)
{
    log_callback = callback;
}

/**
 * @brief Get level name string (standalone version)
 */
static const char* get_level_name(idflog_level_t level)
{
    switch (level) {
        case IDFLOG_DEBUG:   return "DEBUG";
        case IDFLOG_INFO:    return "INFO";
        case IDFLOG_ERROR:   return "ERROR";
        case IDFLOG_VERBOSE: return "VERBOSE";
        case IDFLOG_WARNING: return "WARNING";
        case IDFLOG_WTF:     return "WTF";
        default:             return "UNKNOWN";
    }
}

/**
 * @brief Get current timestamp in milliseconds (standalone version)
 */
static unsigned long get_timestamp_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (unsigned long)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

/**
 * @brief Send logs to callback and/or console (standalone version)
 */
static void send_logs(idflog_level_t level, const char* message)
{
    unsigned long timestamp = get_timestamp_ms();
    
    // Call callback if set
    if (log_callback != NULL) {
        log_callback(level, timestamp, message);
    }
    
    // Print to console if enabled
    if (print_enabled) {
        if (show_time_enabled) {
            printf("[%lu] [%s] %s\n", timestamp, get_level_name(level), message);
        } else {
            printf("[%s] %s\n", get_level_name(level), message);
        }
    }
}

void idflog_debug(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    
    send_logs(IDFLOG_DEBUG, buffer);
    
    va_end(args);
}

void idflog_info(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    
    send_logs(IDFLOG_INFO, buffer);
    
    va_end(args);
}

void idflog_error(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    
    send_logs(IDFLOG_ERROR, buffer);
    
    va_end(args);
}

void idflog_verbose(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    
    send_logs(IDFLOG_VERBOSE, buffer);
    
    va_end(args);
}

void idflog_warning(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    
    send_logs(IDFLOG_WARNING, buffer);
    
    va_end(args);
}

void idflog_wtf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    
    send_logs(IDFLOG_WTF, buffer);
    
    va_end(args);
}

void idflog_log(idflog_level_t level, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    
    send_logs(level, buffer);
    
    va_end(args);
}
