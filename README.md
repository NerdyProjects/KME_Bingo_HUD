## KME HUD

This project implements a small HUD to display information read out from a KME Bingo venturi gas controller on a small OLED display.

### Hardware
* Arduino
* Display
* Cable to KME controller communication port

Beware to interface the KME controller's serial port with 5V, your display might need 3.3V.

### Software
See code, uses u8x8lib for display.

See my KME Bingo protocol description for further information.
