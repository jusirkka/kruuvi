# KRuuvi
KDE/Plasma applet to display meteoronomy data from [Ruuvi Tags](https://ruuvi.com/ruuvitag/).

Only the [Data Format 5](https://docs.ruuvi.com/communication/bluetooth-advertisements/data-format-5-rawv2) has been tested /  is supported.

The application suite in *KRuuvi* consists of
- a plasma applet to display current temperature, pressure and humidity values as well as a meteogram of measurement history.
- a program to read the Ruuvi Tag [measurement history](https://docs.ruuvi.com/communication/bluetooth-connection/nordic-uart-service-nus/log-read) to a sqlite database.

Before launching the applet, to initialize the measurement database, execute

```shell
$ kruuvi_readlog <ruuvitag bt addresses>
```
at least once. To ensure that the measurement database is updated regularly, add e.g. a following line to your crontab (`crontab -e`)

```crontab
*/30 * * * * /usr/bin/kruuvi_readlog -l /tmp/kruuvi/readlog.log <ruuvitag bt addresses>
```

## Build Dependencies

- KDE/Plasma development packages
- KDE/Bluetooth development packages

## Installing
- `mkdir build`
- `cd build`
- `cmake -DCMAKE_BUILD_TYPE=Release ..`
- `make`
- `sudo make install`
