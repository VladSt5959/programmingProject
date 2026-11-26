[README.md](https://github.com/user-attachments/files/35483983/README.md)
# Pico Arcade

Pico Arcade is a small collection of retro-style games and utilities for a Raspberry Pi Pico 2 W with an SPI ILI9341 color display and GPIO buttons. The firmware includes a grid menu, several games, a desk clock, display settings, and automatic brightness control using a photoresistor.

## Features

- Snake
- Platformer (Mario-style)
- Racing game
- Tetris
- Pac-Man
- 2048
- Pixel desk clock
- Settings for display brightness, console uptime, and USB boot mode
- Photoresistor-based automatic brightness adjustment

## Hardware and pin connections

The project is configured for `pico2_w` in `CMakeLists.txt`.

### ILI9341 display

| Display signal | Pico GPIO |
| --- | ---: |
| DC | GP20 |
| CS | GP17 |
| Reset | GP21 |
| SPI clock (SCK) | GP18 |
| SPI data (MOSI) | GP19 |
| Backlight PWM | GP22 |

The display driver initializes SPI and the backlight PWM. This project does not use MISO for the display.

### Buttons

Buttons are configured as active-low inputs with internal pull-ups. Connect each button between its GPIO and GND.

| Control | Pico GPIO |
| --- | ---: |
| Reload to USB boot mode | GP0 |
| Left | GP6 |
| Right | GP7 |
| Up | GP3 |
| Down | GP4 |
| A | GP5 |
| B | GP2 |

`BTN_MENU` is currently an alias for A (GP5), and `BTN_MENU2` is an alias for B (GP2). They are not separate inputs in the current configuration.

### Photoresistor

The photoresistor input is on GP27 / ADC1. The brightness code maps its reading to the display backlight level. The response depends on the photoresistor circuit and may need calibration for the actual sensor and resistor values.

## Build and flash

The project uses the Raspberry Pi Pico SDK, CMake, and Ninja. `CMakeLists.txt` specifies Pico SDK 2.3.1 and the `pico2_w` board.

From the project directory, configure and build with:

```sh
cmake -S . -B build -G Ninja
cmake --build build
```

The build generates `build/gameboyv2.uf2`. To flash it, hold the Pico's BOOTSEL button while connecting it over USB, then copy the UF2 file to the `RPI-RP2` drive. You can also build and flash through the Raspberry Pi Pico VS Code extension.

USB serial stdio is enabled and UART stdio is disabled in the CMake configuration. Open the board's USB serial port to see debug output such as button GPIO numbers and photoresistor readings.

## Controls

- **Menu:** use the direction buttons to move the selection and A to open an item.
- **Most games:** B returns to the menu. A restarts after a game-over screen where supported.
- **Platformer:** left/right move, up or A jumps, B returns to the menu.
- **Racing:** up/down move the car, A retries after a crash, B returns to the menu.
- **Tetris:** left/right move pieces, down drops faster, up or A rotates, B returns to the menu.
- **Pac-Man:** direction buttons steer, A restarts after winning or losing, B returns to the menu.
- **2048:** direction buttons move tiles, A continues after reaching 2048 or retries after a loss, B returns to the menu.
- **Clock:** A toggles 12/24-hour display, B returns to the menu.
- **Settings:** up/down select a row; left/right adjust brightness; A switches between automatic and manual brightness; B returns to the menu. Select the reboot row and press A or right to enter USB boot mode.

## Project layout

```text
.
├── gameboyv2.c             Main loop and application state routing
├── menu.c / menu.h         Main menu and selection handling
├── settings.c / settings.h Display settings and light-sensor service
├── colors.h                RGB565 palette and pixel-art sprites
├── games/                  Individual games and clock
└── libs/
    ├── gfx/                Framebuffer, drawing, and font routines
    ├── ili9341/            SPI display driver and backlight control
    └── ownLibs/            Button reader and photoresistor support
```

