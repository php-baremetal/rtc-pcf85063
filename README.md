# rtc-pcf85063

A driver for the PCF85063 real-time clock on I2C. It registers into ext-i2c's `i2c_drivers` registry, so a
firmware built with it has the class `Baremetal\I2c\Driver\Pcf85063`: it extends `Baremetal\I2c\Device`,
reads the date and time with `now()`, sets them with `set()`, and tells you whether the clock kept time.

The package requires [ext-i2c](package:php-baremetal/ext-i2c), so requiring the driver alone is enough;
conductor adds the I2C HAL for your SDK platform.

## Install

In a project created with `phpbm init`:

```bash
composer require php-baremetal/rtc-pcf85063
```

## Example

```php
<?php
use Baremetal\I2c\Bus;
use Baremetal\I2c\Driver\Pcf85063;

$rtc = new Pcf85063(new Bus(sda: 11, scl: 10)); // 0x51, 400 kHz

if (!$rtc->now()['valid']) {
    $rtc->set(2026, 10, 19, 9, 0, 0);           // only when the clock lost power
}

$t = $rtc->now();
printf("%04d-%02d-%02d %02d:%02d:%02d\n",
    $t['year'], $t['month'], $t['day'], $t['hour'], $t['minute'], $t['second']);
```

## Documentation

- [Overview](docs/overview.md): requirements, install, a first project, notes.
- [PHP API](docs/usage/php-api.md): constructor, `now()`, `set()`, the valid flag.
- [A clock driven by the reactor](docs/recipes/rtc-clock.md)

## License

MIT.
