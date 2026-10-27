---
eyebrow: 'Package · Driver'
lede:    'The PCF85063 real-time clock as a PHP class: now() returns the date and time with a valid flag, set() writes them. Registers into ext-i2c.'
see_also:
  - { href: './usage/php-api.md', meta: 'PHP API' }
  - { href: './recipes/rtc-clock.md', meta: 'Recipe' }
  - { href: 'package:php-baremetal/ext-i2c', meta: 'Extension', label: 'ext-i2c' }
  - { href: 'package:php-baremetal/ext-i2c#docs/drivers/registry.md', meta: 'Registry', label: 'The i2c_drivers registry' }
prev: { label: 'No previous page', href: '#' }
next: { label: 'Pcf85063', href: './usage/php-api.md' }
---

# PCF85063 real-time clock

rtc-pcf85063 adds the class `Baremetal\I2c\Driver\Pcf85063` to the PHP on your board. The PCF85063 keeps the
date and time while the board is off, as long as it has power of its own (a coin cell or a supercap). `now()`
reads it, `set()` writes it, and the `valid` flag says whether the clock kept running since it was last set.

| Property | Value |
| --- | --- |
| Class | `Baremetal\I2c\Driver\Pcf85063`, extends `Baremetal\I2c\Device` |
| Capability | none |
| Default address, clock | `0x51`, 400 kHz |
| Range | years 2000 to 2099, 24-hour time, one-second resolution |
| Background sampling | none: `poll()`, `sample()` and `drain()` throw |

## Requirements

| Requires | Why |
| --- | --- |
| `php-baremetal/ext-i2c` | The `Bus` and `Device` classes and the `i2c_drivers` registry this driver registers into. |

ext-i2c in turn requires the virtual `php-baremetal/i2c-hal`, `php-baremetal/ext-events` and
`php-baremetal/bm-common`. Conductor adds [esp32-i2c-hal](package:php-baremetal/esp32-i2c-hal) for an `esp32`
SDK; you never require it.

## Install

<!-- @steps -->
- **Install conductor once per machine**

  `composer global require php-baremetal/conductor`

- **Create the project**

  `phpbm init my-clock`

- **Require the driver**

  `composer require php-baremetal/rtc-pcf85063` (ext-i2c comes with it).

- **Build and flash**

  `phpbm flash`, then `phpbm monitor` for the console.
<!-- @endsteps -->

The resulting `composer.json` for a Waveshare ESP32-S3-Touch-LCD-1.85B, which has a PCF85063 on its on-board
bus (SDA 11 / SCL 10):

<!-- @code-block language="json" label="composer.json" -->
```json
{
  "name": "acme/my-clock",
  "type": "project",
  "repositories": [
    { "type": "composer", "url": "https://repository.php-baremetal.com", "canonical": true }
  ],
  "require": {
    "php-baremetal/conductor": "^1.0",
    "php-baremetal/rtc-pcf85063": "^1.0"
  },
  "config": {
    "allow-plugins": { "php-baremetal/conductor": true },
    "platform": { "php": "8.5.11" }
  },
  "extra": {
    "baremetal": {
      "sdk": "esp32@^2.0",
      "board": "waveshare-esp32-s3-touch-lcd-1.85b",
      "runtime": "init-loop",
      "storage": "embedded"
    }
  }
}
```
<!-- @endcode-block -->

## A first script

<!-- @code-block language="php" label="project-src/index.php" -->
```php
<?php
use Baremetal\I2c\Bus;
use Baremetal\I2c\Driver\Pcf85063;

$rtc = new Pcf85063(new Bus(sda: 11, scl: 10));

if (!$rtc->now()['valid']) {
    $rtc->set(2026, 10, 19, 9, 0, 0);   // seed a clock that lost power
    echo "RTC was not running: set to 2026-10-19 09:00:00\n";
}

function loop(): void
{
    global $rtc;

    $t = $rtc->now();
    printf("%04d-%02d-%02d %02d:%02d:%02d\n",
        $t['year'], $t['month'], $t['day'], $t['hour'], $t['minute'], $t['second']);
    sleep(1);
}
```
<!-- @endcode-block -->

## PHP API at a glance

| Method | Returns | Meaning |
| --- | --- | --- |
| `new Pcf85063(Bus $bus, int $address = 0x51, int $hz = 400000)` | | Attach and start the clock. |
| `now()` | `array` | `year`, `month`, `day`, `hour`, `minute`, `second` (ints) and `valid` (bool). |
| `set(int $year, int $month, int $day, int $hour, int $minute, int $second)` | `void` | Write the date and time; clears the stopped flag. |

The class also has every `Device` method, so other registers (alarm, offset, timer) are reachable with
`readReg()` and `writeReg()`. Details in [Pcf85063](./usage/php-api.md).

## Registration

The package's `composer.json` declares `"register": { "i2c_drivers": ["pcf85063_driver_desc"] }`. At build time
the descriptor goes into ext-i2c's driver table and at startup ext-i2c registers the class from it; requiring
the package is all it takes. `Bus::scan()` reports `pcf85063` for a device opened through this class. See
[The i2c_drivers registry](package:php-baremetal/ext-i2c#docs/drivers/registry.md).

## Notes

- The constructor writes `0x00` to Control_1 (register `0x00`): clock running, 24-hour mode, no software
  reset, and the 7 pF crystal load setting. It does not check the chip identity (the PCF85063 has none); a
  missing chip fails that write and throws `cannot init pcf85063 at 0x51: ...`.
- `valid` is false while the chip's oscillator-stopped flag is set, i.e. the clock lost power since the last
  `set()`. The time read then is not meaningful.
- `set()` does not validate its arguments and writes the weekday register as 0; `now()` does not return a
  weekday. Compute it in PHP when you need it.
- Failed transfers throw `\Exception` with `PCF85063 read failed` or `PCF85063 write failed`.
- The driver only talks to the chip: it does not set the board's system clock, so `set()` does not change
  what PHP's `time()` returns, and `now()` is the way to read the RTC.
