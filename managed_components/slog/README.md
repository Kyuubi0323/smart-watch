# SLOG - ESP-IDF Logging Component

A flexible and lightweight logging component for ESP-IDF projects that provides enhanced logging capabilities beyond the standard ESP-IDF logging system. 

## Features

- Multiple log levels (DEBUG, INFO, ERROR, VERBOSE, WARNING, WTF)
- Timestamp support
- Callback function support for custom log handling
- Easy-to-use macros
- Compatible with ESP-IDF v4.4+
- Thread-safe


## Installation

### As a Git Submodule

1. Add this repository as a submodule to your ESP-IDF project's `components` directory:
   ```bash
   cd your_project/components
   git submodule add https://github.com/Kyuubi0323/slog slog
   ```

2. To use a specific version (recommended for stability):
   ```bash
   cd your_project/components/slog
   git checkout v0.3
   ```

3. Update your project's CMakeLists.txt to include the component (if not automatically detected):
   ```cmake
   # In your main CMakeLists.txt
   set(EXTRA_COMPONENT_DIRS components/slog)
   ```

### Using ESP Component Registry

Add to your project's `idf_component.yml`:
```yaml
dependencies:
  slog:
    git: https://github.com/Kyuubi0323/slog
    version: v0.3  # Use specific version for stability
```

## Usage

### Using slog 

The component provides a global `slog` instance :

```c
#include "idflog.h"

void app_main(void)
{
    idflog_init();  // This initializes the global slog instance
    
    // Use the slog instance with C-style naming
    slog.i(&slog, "Application started");
    slog.d_format(&slog, "Debug value: %d", 42);
    slog.e(&slog, "An error occurred");
    
    // Configure the logger
    slog.set_print(&slog, true);   // Enable/disable console output
    slog.show_time(&slog, true);   // Enable/disable timestamps
}
```

### Available Methods

The slog_t struct provides these method-like function pointers:

```c
// Configuration
slog.set_print(&slog, bool state);             // Enable/disable console output
slog.show_time(&slog, bool state);             // Enable/disable timestamps
slog.set_log_callback(&slog, callback);        // Set custom callback

// Simple logging (const char* message)
slog.d(&slog, "Debug message");                // Debug
slog.i(&slog, "Info message");                 // Info
slog.w(&slog, "Warning message");              // Warning
slog.e(&slog, "Error message");                // Error
slog.v(&slog, "Verbose message");              // Verbose
slog.wtf(&slog, "WTF message");                // What a Terrible Failure

// Formatted logging (printf-style)
slog.d_format(&slog, "Debug: %d", value);      // Debug with format
slog.i_format(&slog, "Info: %s", string);      // Info with format
slog.w_format(&slog, "Warning: %d", code);     // Warning with format
slog.e_format(&slog, "Error: %s", error);      // Error with format
slog.v_format(&slog, "Verbose: %f", val);      // Verbose with format
slog.wtf_format(&slog, "WTF: %d", code);       // WTF with format

// Generic logging
slog.log(&slog, IDFLOG_INFO, "message");                      // Generic log
slog.log_format(&slog, IDFLOG_DEBUG, "Debug: %d", value);     // Generic with format
```

### Creating Custom Instances

You can create multiple slog_t instances with different configurations:

```c
slog_t my_logger;
slog_init(&my_logger);

// Configure differently than the global slog
my_logger.show_time(&my_logger, false);
my_logger.set_print(&my_logger, true);

my_logger.i(&my_logger, "Custom logger message");
```

### Standalone Functions (Alternative Usage)

For those who prefer functional programming style, the component also provides standalone functions:

```c
// Using convenient macros
IDFLOG_D("Debug message: %d", value);
IDFLOG_I("Info message: %s", string);
IDFLOG_W("Warning message");
IDFLOG_E("Error occurred: %d", error_code);
IDFLOG_V("Verbose details");
IDFLOG_WTF("What a terrible failure!");

// Using function calls directly
idflog_debug("Debug message: %d", value);
idflog_info("Info message: %s", string);
idflog_warning("Warning message");
idflog_error("Error occurred: %d", error_code);
idflog_verbose("Verbose details");
idflog_wtf("What a terrible failure!");

// Configuration functions
idflog_set_print(true);   // Enable/disable console output
idflog_show_time(true);   // Enable/disable timestamps
idflog_set_callback(my_callback);  // Set custom callback
```

## Log Levels

- `IDFLOG_DEBUG` (0) - Debug information
- `IDFLOG_INFO` (1) - General information
- `IDFLOG_ERROR` (2) - Error messages
- `IDFLOG_VERBOSE` (3) - Detailed information
- `IDFLOG_WARNING` (4) - Warning messages
- `IDFLOG_WTF` (5) - What a Terrible Failure (critical errors)

## Example Output

With timestamps enabled (slog format):
```
1234567 [INFO]: Application started
1234568 [DEBUG]: Initializing sensor
1234570 [WARNING]: Sensor calibration needed
1234575 [ERROR]: Failed to read sensor: -1
```

Without timestamps:
```
[INFO]: Application started
[DEBUG]: Initializing sensor
[WARNING]: Sensor calibration needed
[ERROR]: Failed to read sensor: -1
```

Standalone functions format:
```
[1234567] [INFO] Application started
[1234568] [DEBUG] Initializing sensor
[1234570] [WARNING] Sensor calibration needed
[1234575] [ERROR] Failed to read sensor: -1
```

## Integration with Your Project

To use this component in your ESP-IDF project:

1. Add it to your `components` directory as a submodule
2. Include the header: `#include "idflog.h"`
3. Initialize with `idflog_init()`
4. Start logging with the `slog` instance or standalone functions

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.
