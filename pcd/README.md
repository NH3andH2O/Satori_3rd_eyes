English | [简体中文](README_zh-cn.md) | [繁體中文](README_zh-tw.md) | [日本語](README_ja.md)

# Mechanical Satori's Third Eye PCD

## Other Authors

`3rd_eyes` is modified from a design by R饼锐度不够 ([project page](https://oshwhub.com/flanker-e/jue-zhi-tong-esp32)).

Main changes:

- Removed the camera module
- Added gyroscope modules
- Changed the servo I/O assignments
- Changed the GC9A01 ribbon-cable pin order
- Changed the 5 V-to-3.3 V converter inductor from _4.7 uH_ to _3.9 uH_

## Online Preview

https://oshwhub.com/nh3_h2o/satori_3rd_eyes

> [!WARNING]
> The online preview was converted from Altium Designer to EasyEDA for convenient viewing and has not been verified. Use the Gerber files when ordering boards.

## Usage

Send `3rd_eyes_GERBER.zip` and `GC9A01_GERBER.zip` directly to the PCB manufacturer.  
After the boards are manufactured, solder the components according to `3rd_eyes_BOM.csv` and `GC9A01_BOM.csv`.

## Notes

### ESP32-S3 Selection

`ESP32-S3-WROOM-1-N16R8` is recommended. Other module variants may require changes to the `platformIO` configuration.

### Gyroscope Selection and Setup

This project uses gyroscopes from WitMotion. The following models are recommended:

- **Eye gyroscope (mounted on the PCD):** A 6-axis gyroscope such as the [JY61P](https://www.wit-motion.cn/proztmz/37.html)
- **Head gyroscope (connected with Dupont wires):** A 9-axis gyroscope such as the [JY901S](https://www.wit-motion.cn/proztmz/22.html)

The main firmware configures some gyroscope settings automatically. The following setting must be configured with the [WitMotion PC software](https://wit-motion.yuque.com/wumwnr/aqvq6y/qngktvx5grz81zkq):

- Magnetic-field calibration (not required for 6-axis models; instructions should be available in the software)

Upload the main firmware first to apply the automatic gyroscope settings, then upload the `tools\witread` tool so the PC software can be used for configuration.

> [!NOTE]
> The gyroscopes do not need to be installed if the gyroscope features are not required.

### Head Gyroscope Connection

Solder the angled pin header supplied by the manufacturer to the UART side of the head gyroscope. Do not solder the I2C side.  
Use female-to-female Dupont wires connected to `H4`; a length of 60–80 cm is recommended.

<img src="https://github.com/user-attachments/assets/d17616e4-5e87-48f3-a36b-859f3634cc94" width="300" />

### Power Switch Recommendation

Use a switch designed for a 7 mm keycap.

### Servo Connections

Use MG90S 180-degree servos and connect them to `H1`–`H3`.

> [!WARNING]
> The brand must be TIANKONGRC. Make sure the supplied servo horn is the version without reinforcing ribs.

### Display Connection

Use a bare GC9A01 display and solder it to `U1` on the `GC9A01` PCD.

> [!WARNING]
> Check the front and back orientation of the display pins before soldering. After soldering, the PCD should flip naturally so its back can be attached to the back of the display with double-sided tape.
