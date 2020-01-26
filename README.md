# water-meter

## Forewords

This project aims at getting an analog signal from a water meter and use it for a house automatization system to control the water consumption.

## Specifications

This project uses :

* CNY70 reflective sensor (that includes an infrared emitter and phototransistor)
* Arduino Pro Micro 3.3v
* Solar Panel
* TP4056 (Lithium Battery Charger Module)
* E45-TTL-100 (Wireless transceiver module with LoRa spread-spectrum technology)

## Precision

|      Timestamp     | Raw water meter value | Total liters consumed (from sensor) |  Real water comsumption |  Sensor water  consumption |
|:------------------:|:---------------------:|:-----------------------------------:|:-----------------------:|----------------------------|
| 2020/01/26 @ 11h25 |        2064 937       |                                     |                         |                            |

## 3D CNY70 Holder

I have designed a [3D CNY70 holder](model3d/watermeter_cny70_holder.stl) that fits on my watermeter.

![3D CNY70 holder](images/watermeter_cny70_holder.jpg)

## In real life

* CNY70 Sensor + custom 3D printed holder on water meter

![CNY70 sensor](images/watermeter_cny70.jpg)

* Arduino + LoRa transceiver 

![CNY70 sensor](images/watermeter_sensor.jpg)

