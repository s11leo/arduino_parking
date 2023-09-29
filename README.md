# Arduino Parking System

**Technologies:** Arduino, Ethernet, Apache, HTML, JavaScript

**Functionality:**
- The "Parking" Arduino monitors the presence of cars in the parking lot and displays the number of occupied spaces.
- The "Checkpoint (KPP)" Arduino records the movement of cars (entry/exit) and keeps a count to display the number of cars on the complex's premises.
- A web server collects information from all Arduinos and calculates the total number of available parking spaces on the complex's premises.

**Application:** Automation of traffic management and access control for a closed area.

## Installation and Setup

1. **Arduino Code:** The project is designed for the Arduino Uno model.
   - Clone this repository.
   - Upload the code to the Arduino Uno.

2. **Web Server Configuration (Apache2):**
   - Install and configure Apache2 on your server.
   - Place HTML files in a directory accessible to the server.

3. **Sensor Connections:**
   - Connect the sensors to the Arduino with pull-ups to 5V (sensor activation is detected when the contact falls to ground).
   - Software-wise, a "Debounce" function is implemented to minimize false sensor triggers.

## Author

[s11leo](https://github.com/s11leo)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
