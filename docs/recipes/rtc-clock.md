---
eyebrow: 'rtc-pcf85063 · Recipe'
lede:    'An event-driven wall clock: a one-second timer event reads the PCF85063 and prints the date, time and weekday.'
see_also:
  - { href: '../usage/php-api.md', meta: 'PHP API' }
  - { href: 'package:php-baremetal/ext-events', meta: 'Extension', label: 'ext-events' }
prev: { label: 'Pcf85063', href: '../usage/php-api.md' }
next: { label: 'No next page', href: '#' }
---

# A clock driven by the reactor

The PCF85063 of a Waveshare ESP32-S3-Touch-LCD-1.85B (SDA 11 / SCL 10), in the `event-driven` runtime: the
script seeds the RTC if it lost power, registers a handler and a 1 s timer, and returns; the reactor then calls
the handler once a second.

## The project

<!-- @code-block language="json" label="composer.json" -->
```json
{
  "name": "acme/rtc-clock",
  "type": "project",
  "repositories": [
    { "type": "composer", "url": "https://repository.php-baremetal.com", "canonical": true }
  ],
  "require": {
    "php-baremetal/conductor": "^1.0",
    "php-baremetal/ext-events": "^1.0",
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
      "runtime": "event-driven",
      "storage": "embedded"
    }
  }
}
```
<!-- @endcode-block -->

ext-events would come in through ext-i2c anyway; requiring it states that the script uses `Events` and
`every()` directly.

## The script

<!-- @code-block language="php" label="project-src/index.php" -->
```php
<?php
use Baremetal\Event;
use Baremetal\Events;
use Baremetal\I2c\Bus;
use Baremetal\I2c\Driver\Pcf85063;

final class Tick extends Event {}

$rtc = new Pcf85063(new Bus(sda: 11, scl: 10));

if (!$rtc->now()['valid']) {
    $rtc->set(2026, 10, 19, 9, 0, 0);
    echo "RTC was not running: set to 2026-10-19 09:00:00\n";
}

function weekday(int $y, int $m, int $d): string
{
    if ($m < 3) { $m += 12; $y--; }                 // Zeller's congruence
    $k = $y % 100;
    $j = intdiv($y, 100);
    $h = ($d + intdiv(13 * ($m + 1), 5) + $k + intdiv($k, 4) + intdiv($j, 4) + 5 * $j) % 7;
    return ['Sat', 'Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri'][$h];
}

Events::listen(Tick::class, function (Tick $e) use ($rtc): void {
    $t = $rtc->now();
    printf("%s %04d-%02d-%02d %02d:%02d:%02d\n",
        weekday($t['year'], $t['month'], $t['day']),
        $t['year'], $t['month'], $t['day'], $t['hour'], $t['minute'], $t['second']);
});

every(1000, Tick::class);
```
<!-- @endcode-block -->

## How it works

- `every(1000, Tick::class)` (ext-events) raises a `Tick` every second; the handler runs on the PHP task and
  reads the RTC with one transaction.
- The RTC keeps counting while the board is off, as long as it has its own supply, so the clock is right on
  every boot after the first `set()`.
- The weekday is computed in PHP, because the driver does not maintain the chip's weekday register.
