# SIMBAD Integration Guide

## Overview

The [`SimbadQuery`](include/simbadquery.h:1) class provides an interface to query the SIMBAD astronomical database for celestial object coordinates (Right Ascension and Declination).

## Features

- **Asynchronous queries** - Non-blocking network requests using Qt's signal/slot mechanism
- **VOTable parsing** - Parses SIMBAD's VOTable XML format responses
- **Error handling** - Comprehensive error reporting for network and parsing issues
- **Object name resolution** - Supports common astronomical object naming conventions (e.g., "M31", "NGC 1234", "Betelgeuse")

## Usage Example

### Basic Usage

```cpp
#include "simbadquery.h"

// Create query object
SimbadQuery *query = new SimbadQuery(this);

// Connect to signals
connect(query, &SimbadQuery::coordinatesReceived, 
    [](double ra, double dec, const QString &objectName) {
        qDebug() << "Object:" << objectName;
        qDebug() << "RA:" << ra << "degrees";
        qDebug() << "Dec:" << dec << "degrees";
    });

connect(query, &SimbadQuery::errorOccurred,
    [](const QString &error) {
        qDebug() << "Error:" << error;
    });

// Query an object
query->queryObject("M31");  // Andromeda Galaxy
```

## API Reference

### Methods

#### `void queryObject(const QString &objectName)`
Initiates an asynchronous query to SIMBAD for the specified object.

**Parameters:**
- `objectName` - Name of the astronomical object (e.g., "M31", "NGC 1234", "Betelgeuse")

**Example:**
```cpp
query->queryObject("M42");  // Orion Nebula
```

#### `void cancelQuery()`
Cancels any pending query.

**Example:**
```cpp
query->cancelQuery();
```

### Signals

#### `coordinatesReceived(double ra, double dec, const QString &objectName)`
Emitted when coordinates are successfully retrieved from SIMBAD.

**Parameters:**
- `ra` - Right Ascension in decimal degrees (0-360)
- `dec` - Declination in decimal degrees (-90 to +90)
- `objectName` - The name of the object that was queried

#### `errorOccurred(const QString &error)`
Emitted when an error occurs during the query.

**Parameters:**
- `error` - Error message describing what went wrong

#### `queryStarted(const QString &objectName)`
Emitted when a query begins.

**Parameters:**
- `objectName` - The name of the object being queried

## Coordinate System

SIMBAD returns coordinates in the **ICRS (International Celestial Reference System)** coordinate system:

- **RA (Right Ascension)**: Returned in decimal degrees (0° to 360°)
- **DEC (Declination)**: Returned in decimal degrees (-90° to +90°)

## Supported Object Names

SIMBAD supports various naming conventions:

### Common Catalog Names
- **Messier**: M31, M42, M51
- **NGC**: NGC 1234, NGC 5678
- **IC**: IC 1234
- **Common names**: Andromeda, Betelgeuse, Sirius

### Other Catalogs
- HD, HR, HIP (star catalogs)
- Abell (galaxy clusters)
- UGC, CGCG (galaxy catalogs)
- And many more...

## Error Handling

The class reports errors through the `errorOccurred` signal. Common errors include:

- **Network errors**: No internet connection, timeout, server unreachable
- **Parse errors**: Invalid response format, object not found
- **Invalid input**: Empty object name

## Network Requirements

- **Internet connection required** for querying SIMBAD
- **Firewall**: Ensure outbound HTTPS connections to `simbad.u-strasbg.fr` are allowed
- **Rate limiting**: SIMBAD may rate-limit excessive queries


## Implementation Details

### VOTable Parsing

The class parses SIMBAD's VOTable XML format response:
1. Locates FIELD elements with UCDs `pos.eq.ra;meta.main` and `pos.eq.dec;meta.main`
2. Extracts corresponding data from TR/TD elements
3. Converts string values to double precision coordinates

### Network Protocol

- **Service**: SIMBAD sim-id service
- **URL**: `https://simbad.u-strasbg.fr/simbad/sim-id`
- **Method**: GET
- **Format**: VOTable (Virtual Observatory Table format)
- **User Agent**: mgwObsLog/1.0

## Troubleshooting

### No Results for Valid Object

- Check internet connection
- Verify object name spelling
- Try alternative names (e.g., "M31" vs "Andromeda")

### Parse Errors

- SIMBAD may have changed response format
- Network timeout may have truncated response
- Check console output with `qDebug()` enabled

### Network Timeouts

- Default Qt timeout applies
- For slower connections, implement custom timeout handling

## Future Enhancements

Possible improvements:
- Cache results to reduce queries
- Support batch queries
- Return additional object information (type, magnitude, etc.)
- Implement retry logic for failed queries
- Add timeout configuration