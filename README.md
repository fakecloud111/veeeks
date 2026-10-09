# VEX - Modular System

## Overview

The VEX project has been refactored following modular architecture principles, dependency inversion, and separation of concerns. This new structure provides greater flexibility, testability, and maintainability.

## Project Structure

```
VEX/
├── include/                    # Public headers
│   ├── vex.hpp                # Main header (includes all interfaces)
│   ├── core/                  # System core
│   │   └── iapplication.hpp   # Main application interface
│   ├── driver/                # Driver interface
│   │   └── idriver.hpp        # Abstract driver interface
│   ├── game/                  # Game system
│   │   └── igame_system.hpp   # Game system interface
│   └── utils/                 # Utilities
│       ├── logger.hpp         # Logging system
│       └── config.hpp         # Configuration system
├── src/                       # Implementations
│   ├── core/                  # System core
│   │   └── application.cpp    # Application implementation
│   ├── driver/                # Driver implementation
│   │   └── driver_impl.cpp    # Concrete driver implementation
│   ├── game/                  # Game system
│   │   └── vgk_system.cpp     # VGK system implementation
│   └── utils/                 # Utilities
│       ├── console_logger.cpp # Console logger
│       └── memory_config.cpp  # Memory configuration manager
├── driver/                    # Kernel driver (kept)
├── main.cpp                   # Main entry point
└── README.md                  # This file
```

## Design Principles

### 1. Dependency Inversion
- High-level modules don't depend on low-level modules
- Both depend on abstractions (interfaces)
- Dependencies are injected through constructors

### 2. Separation of Concerns
- **Core**: Manages application lifecycle
- **Driver**: Abstracts low-level system operations
- **Game**: Handles game-specific logic
- **Utils**: Provides reusable auxiliary functionality

### 3. Abstract Interfaces
- Each module defines a clear interface
- Concrete implementations can be easily swapped
- Facilitates unit testing and mock objects

## Main Modules

### Core (`include/core/`)
- **IApplication**: Interface for the main application
- Manages initialization, execution, and shutdown
- Coordinates all other modules

### Driver (`include/driver/`)
- **IDriver**: Abstract interface for driver operations
- Memory, process, and system operations
- Support for different driver implementations

### Game (`include/game/`)
- **IGameSystem**: Interface for game systems
- VGK (Valorant) specific logic
- Game data decryption and processing

### Utils (`include/utils/`)
- **ILogger**: Flexible logging system
- **IConfigManager**: Configuration management
- Reusable auxiliary functionality

## Usage

### 1. Include Main Header
```cpp
#include "include/vex.hpp"
```

### 2. Create Instances
```cpp
// Create driver
auto driver = vex::driver::create_driver();

// Create game system
auto game_system = vex::game::create_vgk_system(driver);

// Create application
auto app = vex::core::create_application();
```

### 3. Configure and Run
```cpp
// Initialize
if (app->initialize()) {
    // Set target process
    app->set_target_process(L"notepad.exe");
    
    // Run
    app->run();
}
```

## Benefits of New Architecture

### 1. **Testability**
- Abstract interfaces allow mock objects
- Injected dependencies facilitate unit testing
- Clear separation of responsibilities

### 2. **Maintainability**
- Code organized in logical modules
- Changes in one module don't affect others
- Clear documentation of responsibilities

### 3. **Flexibility**
- Easy implementation swapping
- Support for different driver types
- Flexible configuration via configuration system

### 4. **Reusability**
- Utilities usable in different contexts
- Standardized interfaces for new modules
- DRY (Don't Repeat Yourself) code

## Extensibility

### Add New Game System
1. Implement `IGameSystem`
2. Create factory function
3. Integrate into main application

### Add New Driver
1. Implement `IDriver`
2. Create factory function
3. Configure via configuration system

### Add New Utilities
1. Define abstract interface
2. Implement functionality
3. Add to `vex::utils` namespace

## Compilation

The project maintains compatibility with the existing build system. New modular files are automatically included in the Visual Studio project.

## Migration

Original code preserved for compatibility. For complete migration:

1. Remove old files (`core.cpp`, `sdk.cpp`)
2. Update references to use new interfaces
3. Configure dependencies via configuration system

## Contribution

When contributing to the project:

1. Follow established design principles
2. Maintain abstract interfaces
3. Implement tests for new modules
4. Document new features
5. Use logging system for debugging

## License

This project follows the same license terms as the original project.


