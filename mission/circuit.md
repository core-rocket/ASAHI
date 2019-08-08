# ミッション部回路

## Arduino nano
|ピン|接続先|
|-|-|
|D2|フライトピン|
|D3|TWE-Lite RX|
|D4|TWE-Lite TX|
|A4|SDA(5V)|
|A5|SCL(5V)|

## フライトピン
|刺すとこ|接続先|
|-|-|
|穴1|nano D2|
|穴2|GND|

## TWE-Lite DIP
|ピン|接続先|
|-|-|
|GND|GND|
|VCC|3.3V|
|RX|nano D3|
|TX|nano D4|

## FXMA2102(レベル変換)
- 左側
|ピン|接続先|
|-|-|
|VCCA|3.3V|
|A0|SCL(3.3V)|
|A1|SDA(3.3V)|
|OE|3.3V|

- 右側
|ピン|接続先|
|-|-|
|VCCB|5V|
|B0|SCL(5V)|
|B1|SDA(5V)|
|GND|GND|

## BMP280
|ピン|接続先|
|-|-|
|VCC|3.3V|
|GND|GND|
|SCL|SCL(3.3V)|
|SDA|SDA(3.3V)|
