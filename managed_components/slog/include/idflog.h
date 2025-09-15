/**
 * @file idflog.h
 * @brief ESP-IDF Logging Component
 * @author khoi.nv0323.work@gmail.com
 * @date 2025
 */

#ifndef _IDFLOG_H_
#define _IDFLOG_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Log levels for the logging system
 */
typedef enum {
    IDFLOG_DEBUG = 0,
    IDFLOG_INFO,
    IDFLOG_ERROR,
    IDFLOG_VERBOSE,
    IDFLOG_WARNING,
    IDFLOG_WTF
} idflog_level_t;

/**
 * @brief Callback function type for log messages
 * @param level Log level
 * @param timestamp Timestamp in milliseconds
 * @param message Log message
 */
typedef void (*idflog_callback_t)(idflog_level_t level, unsigned long timestamp, const char* message);


// Forward declaration
typedef struct slog slog_t;

/**
 * @brief slog structure with function pointers (C-style object)
 */
struct slog {
    // Private members
    bool _print;
    bool _time;
    idflog_callback_t logging_callback;
    
    // Function pointers (methods)
    void (*set_print)(slog_t* self, bool state);
    void (*show_time)(slog_t* self, bool state);
    void (*set_log_callback)(slog_t* self, idflog_callback_t callback);
    
    void (*d)(slog_t* self, const char* msg);
    void (*d_format)(slog_t* self, const char* format, ...);
    void (*e)(slog_t* self, const char* msg);
    void (*e_format)(slog_t* self, const char* format, ...);
    void (*i)(slog_t* self, const char* msg);
    void (*i_format)(slog_t* self, const char* format, ...);
    void (*v)(slog_t* self, const char* msg);
    void (*v_format)(slog_t* self, const char* format, ...);
    void (*w)(slog_t* self, const char* msg);
    void (*w_format)(slog_t* self, const char* format, ...);
    void (*wtf)(slog_t* self, const char* msg);
    void (*wtf_format)(slog_t* self, const char* format, ...);
    
    void (*log)(slog_t* self, idflog_level_t level, const char* msg);
    void (*log_format)(slog_t* self, idflog_level_t level, const char* format, ...);
};

/**
 * @brief Initialize a slog instance
 * @param slog_instance Pointer to slog instance to initialize
 */
void slog_init(slog_t* slog_instance);

/**
 * @brief Global slog instance (like Arduino-style)
 */
extern slog_t slog;

/**
 * @brief Get current timestamp in milliseconds (equivalent to Arduino millis())
 */
unsigned long get_millis(void);

/**
 * @brief Initialize the logging system
 */
void idflog_init(void);

/**
 * @brief Set whether to print logs to console
 * @param state true to enable, false to disable
 */
void idflog_set_print(bool state);

/**
 * @brief Set whether to show timestamp in logs
 * @param state true to enable, false to disable
 */
void idflog_show_time(bool state);

/**
 * @brief Set callback function for log messages
 * @param callback Callback function pointer
 */
void idflog_set_callback(idflog_callback_t callback);

/**
 * @brief Log debug message
 * @param format Printf-style format string
 * @param ... Variable arguments
 */
void idflog_debug(const char *format, ...);

/**
 * @brief Log info message
 * @param format Printf-style format string
 * @param ... Variable arguments
 */
void idflog_info(const char *format, ...);

/**
 * @brief Log error message
 * @param format Printf-style format string
 * @param ... Variable arguments
 */
void idflog_error(const char *format, ...);

/**
 * @brief Log verbose message
 * @param format Printf-style format string
 * @param ... Variable arguments
 */
void idflog_verbose(const char *format, ...);

/**
 * @brief Log warning message
 * @param format Printf-style format string
 * @param ... Variable arguments
 */
void idflog_warning(const char *format, ...);

/**
 * @brief Log WTF (What a Terrible Failure) message
 * @param format Printf-style format string
 * @param ... Variable arguments
 */
void idflog_wtf(const char *format, ...);

/**
 * @brief Generic log function
 * @param level Log level
 * @param format Printf-style format string
 * @param ... Variable arguments
 */
void idflog_log(idflog_level_t level, const char *format, ...);

// Convenience macros
#define IDFLOG_D(format, ...) idflog_debug(format, ##__VA_ARGS__)
#define IDFLOG_I(format, ...) idflog_info(format, ##__VA_ARGS__)
#define IDFLOG_E(format, ...) idflog_error(format, ##__VA_ARGS__)
#define IDFLOG_V(format, ...) idflog_verbose(format, ##__VA_ARGS__)
#define IDFLOG_W(format, ...) idflog_warning(format, ##__VA_ARGS__)
#define IDFLOG_WTF(format, ...) idflog_wtf(format, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif // _IDFLOG_H_