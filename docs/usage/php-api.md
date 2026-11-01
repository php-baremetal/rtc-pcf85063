---
eyebrow: 'rtc-pcf85063 · PHP API'
lede:    'Baremetal\I2c\Driver\Pcf85063: the constructor, now() with its valid flag, and set().'
see_also:
  - { href: '../recipes/rtc-clock.md', meta: 'Recipe' }
  - { href: 'package:php-baremetal/ext-i2c#docs/usage/device.md', meta: 'ext-i2c', label: 'Device' }
prev: { label: 'Overview', href: '../overview.md' }
next: { label: 'A clock driven by the reactor', href: '../recipes/rtc-clock.md' }
---

# Pcf85063

`Baremetal\I2c\Driver\Pcf85063` extends `Baremetal\I2c\Device`. It implements no capability interface and has
no sampler. All the `Device` methods work on it; `poll()`, `sample()` and `drain()` throw
`this device has no poller (driver has no poll block)`.

## Constructor

<!-- @method name="new Pcf85063(Bus $bus, int $address = 0x51, int $hz = 400000)" visibility="public" -->

<!-- @params -->
<!-- @param name="$bus" type="Bus" required -->
The bus the chip is wired to.
<!-- @endparam -->
<!-- @param name="$address" type="int" default="0x51" -->
7-bit address. The PCF85063 has a fixed address.
<!-- @endparam -->
<!-- @param name="$hz" type="int" default="400000" -->
SCL clock for this device: 0 uses 400 kHz, otherwise 1 to 1000000 Hz (a negative value throws `\ValueError` from 1.0.2; earlier versions used 400 kHz).
<!-- @endparam -->
<!-- @endparams -->

The constructor attaches the device, reads Control_1 and writes it back with the STOP, software-reset, test and
12-hour bits cleared: the clock runs in 24-hour mode, and the crystal load setting (`CAP_SEL`) is left as the
board set it. It throws `\Exception`:

| Message | Cause |
| --- | --- |
| `cannot init pcf85063 at 0x..: ...` | The Control_1 read or write failed, for example `ESP_ERR_TIMEOUT` with no chip. |
| `address 0x.. on this bus is already used by another driver` | A raw device or another driver holds the address. |

Constructing it again with the same bus and address returns a handle to the same device without writing again.

## now()

<!-- @method name="Pcf85063::now(): array" returns="array" visibility="public" -->

Reads the seven time registers (from `0x04`) in one transaction and returns:

| Key | Type | Range |
| --- | --- | --- |
| `year` | int | 2000 to 2099 |
| `month` | int | 1 to 12 |
| `day` | int | 1 to 31 |
| `hour` | int | 0 to 23 |
| `minute` | int | 0 to 59 |
| `second` | int | 0 to 59 |
| `valid` | bool | `false` when the oscillator-stopped flag is set |

Throws `PCF85063 read failed`.

## set()

<!-- @method name="Pcf85063::set(int $year, int $month, int $day, int $hour, int $minute, int $second): void" returns="void" visibility="public" -->

Writes the date and time in one transaction, with the weekday computed from the date. Writing the seconds
clears the oscillator-stopped flag, so `now()['valid']` becomes `true`. Throws `PCF85063 write failed`.

Every argument is checked first, and a value out of range throws `\ValueError` without writing anything:
`$year` 2000 to 2099 (the chip stores two digits), `$month` 1 to 12, `$day` a day of that month (29 February
only in leap years), `$hour` 0 to 23, `$minute` and `$second` 0 to 59.

## Example

<!-- @code-block language="php" label="set the RTC from a build-time constant" -->
```php
<?php
use Baremetal\I2c\Bus;
use Baremetal\I2c\Driver\Pcf85063;

const SEED = [2026, 10, 19, 9, 0, 0];

$rtc = new Pcf85063(new Bus(sda: 11, scl: 10));

$t = $rtc->now();
if (!$t['valid']) {
    $rtc->set(...SEED);
    $t = $rtc->now();
}
printf("%04d-%02d-%02d %02d:%02d:%02d valid=%s\n", $t['year'], $t['month'], $t['day'],
    $t['hour'], $t['minute'], $t['second'], $t['valid'] ? 'yes' : 'no');
```
<!-- @endcode-block -->
