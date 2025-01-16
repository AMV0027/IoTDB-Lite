# IoTDB-Lite: A Lightweight MQTT-Integrated IoT Database

Welcome to IoTDB-Lite! This lightweight IoT database is designed for resource-constrained environments and enables efficient data collection and querying for IoT devices. The system integrates seamlessly with the MQTT protocol for handling sensor data, making it ideal for IoT deployments.

---

## Features

- **Lightweight Database**: Optimized for storing IoT data in memory with a compressed file-based backup using Run-Length Encoding (RLE).
- **MQTT Integration**: Allows data insertion and querying through the MQTT protocol, enabling seamless communication with IoT devices.
- **Custom Commands**:
  - `INSERT`: Store sensor readings (timestamp, value, metadata).
  - `QUERY`: Retrieve sensor data based on a time range.
- **Persistence**: Automatically saves and loads compressed data on disk for reliability.

---

## Installation

### Prerequisites

- A C compiler (e.g., GCC)
- The [Eclipse Paho MQTT C Client library](https://github.com/eclipse/paho.mqtt.c)
- An MQTT broker (e.g., [Mosquitto](https://mosquitto.org/))

### Building the Project

1. Clone the repository:
   ```bash
   git clone https://github.com/AMV0027/IoTDB-Lite.git
   cd IoTDB-Lite
   ```

2. Install dependencies:
   - Follow the instructions for installing the Eclipse Paho MQTT C Client library.

3. Compile the code:
   ```bash
   gcc -o iotdb main.c iotdb.c -lpaho-mqtt3c
   ```

4. Run the program:
   ```bash
   ./iotdb
   ```

---

## Usage

### MQTT Topics
- **Request Topic**: `iotdb/request`
- **Response Topic**: `iotdb/response`

### Supported Commands

#### 1. `INSERT`
Add a record to the database:
```
INSERT <timestamp> <value> <metadata>
```
- Example:
  ```
  INSERT 2025-01-16T10:00:00 25.5 temperature
  ```
  Response:
  ```
  INSERT OK: 2025-01-16T10:00:00
  ```

#### 2. `QUERY`
Retrieve records within a specified time range:
```
QUERY <start_time> <end_time>
```
- Example:
  ```
  QUERY 2025-01-16T10:00:00 2025-01-16T11:00:00
  ```
  Response:
  ```
  ID:1 TS:2025-01-16T10:00:00 Value:25.5 Metadata:temperature
  ```

---

## Project Structure

- **main.c**: Main program handling MQTT communication and command parsing.
- **iotdb.c**: Database implementation with compression and query functionality.
- **iotdb.h**: Header file defining database structures and function prototypes.

---

## Contributing

IoTDB-Lite is an open-source project, and we welcome contributions from the community! To contribute:

1. Fork the repository.
2. Create a new branch for your feature or bug fix.
3. Submit a pull request describing your changes.

---

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

---

## Acknowledgments

- [Eclipse Paho MQTT C Client](https://github.com/eclipse/paho.mqtt.c)
- [Mosquitto MQTT Broker](https://mosquitto.org/)

---

Start collecting and querying IoT data efficiently with IoTDB-Lite! 🚀

