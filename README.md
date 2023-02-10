# Jobout
Various forms of products that contribute to better ergonomics and variety in workplaces are offered by one of Burde's brands, JobOut. A challenge faced by JobOut is to motivate users to use its products. It is believed that individuals and organizations can be motivated to achieve better results in their use by using sensors to measure how people vary their way of working. The goal is to learn about the different possibilities of carrying out measurements with the products and creating a prototype.


# Bill Of Material (BOM)
- Heltec-stick-lite (esp32 with LoRaWAN modem)
- LiPo battery with a micro JST 1.25 mm female 2 pins connector
- Option 1
    - FSR sensor
    - 4.7kOhm resistor
- Option 2
    - Velostat
    - Copper film
    - 4.7kOhm resistor
- Option 3
     - JYS-SBR-001
    - 4.7kOhm resistor

# FSR
An FSR (Force Sensitive Resistor) sensor works as a touch sensor by detecting changes in resistance due to pressure applied to its surface. The sensor consists of a conductive material, typically carbon mounted on a flexible substrate. When pressure is applied to the surface of the sensor, the conductive material compresses, and the resistance between the two conductive layers changes. This change in resistance is then converted into an electrical signal that can be read by a microcontroller or an analog-to-digital converter (ADC). The amount of pressure applied to the surface of the FSR sensor determines the magnitude of the resistance change, and therefore, the touch intensity can be determined. FSRs are commonly used in touchpads, touchscreens, and various human-machine interaction applications.

![](https://i.imgur.com/KtGk4Ps.jpg)

# Velostat
Velostat is a type of conductive polymer that is commonly used as a touch sensor. It works by detecting changes in electrical resistance when pressure is applied to its surface. The material is made of a thin layer of conductive material, typically copper, that is coated onto a flexible plastic substrate. When pressure is applied to the Velostat, the electrical resistance of the copper layer changes, causing a change in voltage that can be measured and interpreted as a touch. This simple yet effective principle makes Velostat a popular choice for creating touch sensors in a wide range of applications, from simple buttons to more complex touch-sensitive interfaces.

![](https://www.researchgate.net/publication/352928309/figure/fig1/AS:1041286920417280@1625273556211/Pressure-sensitive-conductive-sheet-of-piezoresistive-Velostat-film-a-and-related.ppm)

![](https://i.imgur.com/9hnC65b.jpg)
The materials surrounding the Velostat in the prototype above are two copper films. 

Note that the less sensitive the sensor is supposed to be, the less the copper area should be. 

# Pressure sensor
The auto seat sensor is a membrane touch point sensor commonly used in car seats to determine if a seat is empty or not. It works similarly to the previously discussed sensors. 

![](https://i.imgur.com/e6S50PC.jpg)


# Circuit diagram
All three sensors in the examples above are connected as follows:

![](https://camo.githubusercontent.com/c8d7a6fc626bb83e1a5855f3c989146803203f781473d66eee2044de9e739e1a/68747470733a2f2f692e696d6775722e636f6d2f6b7234587a53492e706e67)

Note that the circuit diagram above shows the Velostat connection, but the same connection would work for the other options. 


# Deep sleep
The device is normally in deep sleep and is triggered to wake up in two different ways. The first way is by external input, i.e. when one of the esp32 switches state from LOW to HIGH.
```arduino
esp_sleep_enable_ext0_wakeup(GPIO_NUM_28, 1); // 1 = High, 0 = Low
```

The other way is waking up the device by using a timer. 
```arduino
esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
esp_deep_sleep_start();
```

The flow of the program is as follows:

![](https://i.imgur.com/sUuO7xw.png)

Once the device is triggered to wake up, it increases a counter and sends a LoRaWAN frame. The counter is there to help determine when the device should be put to sleep and woken up by either external input or a timer. E.g. if a person uses a connected pad for 30 minutes, we don't want the device to constantly wake up and send LoRaWAN frames, instead we go to sleep and ignore all external input for `n` minutes. 


# Power profiling
The power consumption of this prototype is done using an Otii Arc. 

The image below shows the power consumption of the device for joining the network. The join sequence and TX-frame took `~12.2` seconds to execute and consumed around `748uWh`. A future improvement of this could be joining only once, by saving the LoRaWAN session keys in the device's `nvram` and remaining joined after waking up from deep sleep.

![](https://i.imgur.com/ZrgWsyA.png)

As for deep sleep, the device consumes `~26.5uA`.
![](https://i.imgur.com/WmgKVNZ.png)

Depending on how often the device will be transmitting, battery life can be approximated.

# Run this example

To run this example, install Visual Studio Code and the PlatformIO extension. Then, clone this project and change the `upload_port` to the port your device is connected to. Once done, the project can be built and uploaded to the device.

```c
[env:heltec_wireless_stick_lite]
platform = espressif32@3.1.1
framework = arduino
upload_port = /dev/cu.usbserial-0001
monitor_speed = 115200
board = heltec_wireless_stick_lite
lib_deps = 
    heltecautomation/Heltec ESP32 Dev-Boards@^1.1.1
    SPI
    Wire
    heltecautomation/ESP32_LoRaWAN@^2.1.1
build_flags = 
    -D REGION_EU868
    -D ACTIVE_REGION=LORAMAC_REGION_EU868
    -D LoRaWAN_DEBUG_LEVEL=1
```