# Crystal Log

A high-performance, asynchronous logging system designed for the **Crystal Sound Lab** project. [cite_start]It leverages **C++20 Modules** to ensure fast compilation and a clean API boundary [cite: 58-59].

## 1. Installation & Integration

Since this library uses C++20 Modules, ensure your compiler (e.g., MSVC 19.28+) supports the `export module` syntax.

1. Add `Types.ixx`, `Internal.ixx`, and `Crystal_Log.ixx` to your project.
2. In your application, simply call:
```cpp
import Crystal_Log;
```
## 2. Configuration (.ini File)

### Log File Saving Location

By default, the system uses the **Win32 API** (`GetModuleFileNameW`) to locate the directory of your executable. It looks for a file named **`Log_Settings.ini`** in the same directory.

**Default Path**: `[Your_Executable_Dir]/Log_Settings.ini`.  
**Custom Path**: You can provide a specific path during initialization, e.g:
```cpp
Crystal::Crystal_Log_Init("C:/MyConfigs/Log_Settings.ini");
```

### .ini Configuration Format

The parser uses a `key = value` format. Lines starting with `#` are ignored. Below are the available keys and their default values:

```ini
# --- Console Output Settings ---
if_colored_output = true   # Use ANSI escape codes for level colors
if_to_console     = true   # Print to std::cout/std::cerr

# --- File Output Settings ---
if_to_file        = true   # Record logs to a physical file
log_output_path   = ./Logs # Directory for .log files (auto-created)

# --- Formatting Settings ---
if_time_stamp     = true   # Add [HH:MM:SS] timestamps
if_file_location  = true   # Show [FileName.cpp:LineNumber]

# --- Core Performance ---
if_threaded       = true   # Enable asynchronous background thread
```

**Note**: Values are case-insensitive. `TRUE`, `true`, and `True` are all processed as logical `true`. But values with " or ' are unaccepted.

## 3. Usage Example

```cpp
#include <optional>
import Crystal_Log;

int main() {
    // 1. Initialize (Loads .ini or uses internal defaults)
    Crystal::Crystal_Log_Init(); 

    // 2. Set runtime log level
    Crystal::Crystal_Log_Set_Level(Crystal::Log_Level::Debug);

    // 3. Start logging
    Crystal::Crystal_Info_Log("Hello Crystal Log!");
    Crystal::Crystal_Error_Log("This is an error log.");
    Crystal::Crystal_Error_Log("This is an critical log.");

    // 4. Background thread terminates automatically on exit
    return 0; 
}
```

