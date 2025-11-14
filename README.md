# MGW Observation Log

A Qt6 C++ application for managing astronomical observation data with SQLite database backend.

## Project Structure

```
mgwObsLog/
├── CMakeLists.txt          # CMake build configuration
├── README.md               # This file
├── include/                # Header files
│   ├── mainwindow.h
│   ├── databasemanager.h
│   └── [tab headers...]
├── src/                    # Implementation files
│   ├── main.cpp
│   ├── mainwindow.cpp
│   ├── databasemanager.cpp
│   └── [tab implementations...]
└── uifiles/                # Qt Designer UI files
    ├── mainwindow.ui
    └── [tab UI files...]
```

## Features

- **Tabbed Interface** with 11 specialized tabs:
  - Objects: Manage celestial objects
  - Sessions: Track observation sessions
  - Cameras: Camera equipment database
  - Filter Types: Filter type management
  - Filters: Individual filter tracking
  - Telescopes: Telescope equipment database
  - Observations: Individual observation records
  - Object Stats: Statistics per object
  - Monthly Stats: Monthly observation statistics
  - Settings: Application settings
  - About: Application information

- **SQLite Database** with automatic initialization
- **Automatic Table Creation** on first run
- **SIMBAD Integration** - Query astronomical object coordinates from SIMBAD database

## Database Schema

The application creates the following tables on first run:

- `cameras` - Camera equipment
- `telescopes` - Telescope equipment  
- `objects` - Celestial objects
- `filter_types` - Filter type categories
- `filters` - Individual filters
- `sessions` - Observation sessions
- `observations` - Individual observations (links all entities)

## Prerequisites

- CMake 3.16 or higher
- Qt6 (Core, Widgets, Sql, Network modules)
- MinGW compiler (or other C++ compiler)
- SQLite3 (included with Qt6 Sql module)
- Internet connection (for SIMBAD queries)

## Building

### Windows with MinGW

1. Ensure Qt6 is installed and CMAKE_PREFIX_PATH points to Qt6:
```bash
set CMAKE_PREFIX_PATH=C:\Qt\6.x.x\mingw_xx
```

2. Create build directory and configure:
```bash
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
```

3. Build:
```bash
cmake --build .
```

4. Run:
```bash
.\mgwObsLog.exe
```

### Alternative CMake Generators

For Visual Studio:
```bash
cmake -G "Visual Studio 17 2022" ..
cmake --build . --config Release
```

For Ninja:
```bash
cmake -G "Ninja" ..
cmake --build .
```

## Database

The application creates a SQLite database file `mgw_observations.db` in the application directory on first run. All tables are created automatically with the schema defined in the task requirements.

## Architecture

- **MainWindow**: Central window managing the tabbed interface
- **DatabaseManager**: Singleton-style database access manager
  - Handles database initialization
  - Creates tables on first run
  - Provides database access to all tabs
- **Tab Classes**: Each tab has its own class for managing UI and logic
  - Initialize UI from .ui files
  - Access database through DatabaseManager
  - Implement tab-specific functionality
- **SimbadQuery**: Asynchronous SIMBAD database query handler
  - Queries astronomical object coordinates (RA, DEC)
  - Signal-based asynchronous interface
  - Parses VOTable XML responses

## Development

The application is structured for easy extension:

1. Each tab is self-contained with its own .h, .cpp, and .ui files
2. Database operations are centralized in DatabaseManager
3. Tab functionality can be implemented independently
4. UI can be edited in Qt Designer

## Future Implementation

Tab-specific functionality (CRUD operations, data display, statistics) should be implemented in each tab's `initialize()` and `refreshData()` methods.

## License

[Specify your license here]