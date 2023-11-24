# Inventory_Management_System
IOT based inventory Management System using RFID Reader and cards
An Arduino-based RFID inventory management system that utilizes Wi-Fi for communication and ensures secure data transfer.

## Overview

This project implements an RFID inventory management system using Arduino (NodeMCU), allowing for real-time tracking of product quantities. The system leverages Wi-Fi for communication and includes a secure data transfer mechanism using HTTPS.

## Features

- RFID tag scanning for product identification
- Wi-Fi connectivity for communication
- Secure communication using HTTPS
- Real-time inventory updates
- Low quantity alerts with push notifications

## Getting Started

### Prerequisites

- Arduino IDE
- Necessary Arduino libraries (ESP8266WiFi, MFRC522, etc.)
- Access to a secure server with HTTPS support
Usage
Connect the RFID reader and NodeMCU to your Arduino board.

Power up the system and wait for it to connect to Wi-Fi.

Scan RFID tags to update inventory and trigger low quantity alerts.

Monitor the serial output for system updates and alerts.
License
This project is licensed under the MIT License.

Acknowledgements
MFRC522 library
ESP8266WiFi library
Arduino
