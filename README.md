# LavaBot - Autonomous UV Disinfection Robot

An Arduino-based autonomous robot system for UV disinfection with scheduled operation and manual control modes.

## Features

- **Automatic Mode**: Scheduled operation at 9:00-9:59 and 14:00-14:59
- **Manual Mode**: Direct control via panel buttons
- **Safety System**: PIR motion sensor automatically disables UV when human presence is detected
- **Real-time Clock**: Uses DS3231 RTC for accurate scheduling

## Hardware

- Arduino Mega (or compatible)
- DS3231 RTC module
- PIR motion sensor
- UV lamp relay
- Dual motor driver
- Control panel with mode and directional buttons

## Software Structure

- `LABVABOT.ino` - Main Arduino sketch
- `src/schedule.h` - Schedule window logic
- `src/system_state.h` - System state machine for testable logic

## Testing

The project includes comprehensive unit tests that run on the host (no hardware required):

```bash
cd tests/host
make
make run
```

Tests cover:
- Schedule logic
- System state integrity
- Durability and stress testing

## CI/CD

GitHub Actions automatically:
- Lints the code
- Compiles for Arduino Uno and Mega
- Runs all unit tests

## License

See LICENSE file for details.

