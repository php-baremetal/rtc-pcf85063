# Changelog

## 1.1.0

- Added: ide/rtc-pcf85063.stub, the PHP declarations of the package for IDEs (conductor copies it into vendor/php-baremetal/ide-stubs/).

## 1.0.2

- Fixed: `set()` wrote any value as BCD, so a month of 13 or a negative hour left the chip with nonsense, and
  kept only `$year % 100`. It now throws `\ValueError` for a year outside 2000 to 2099, a month outside 1 to
  12, a day the month does not have, an hour outside 0 to 23 and a minute or second outside 0 to 59.
- Fixed: `set()` wrote the weekday register as 0; it now writes the weekday of the date.
- Fixed: the constructor wrote `0x00` to Control_1, which also reset the crystal load setting (`CAP_SEL`). It
  now clears only the STOP, software-reset, test and 12-hour bits.
- Changed: a negative `$hz` throws `\ValueError` (with ext-i2c 1.0.2); it used 400 kHz. `0` still does.

## 1.0.1

- Changed: the author's contact email.
- Docs: projects use `"sdk": "esp32@^2.0"`.

## 1.0.0

Initial release.
