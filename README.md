English | [简体中文](README_zh-cn.md) | [繁體中文](README_zh-tw.md) | [日本語](README_ja.md)

# Mechanical Satori's Third Eye

A mechanical version of Satori Komeiji's third eye from Touhou Project. It currently supports gyroscope tracking and web-based control.

## Demo

https://www.bilibili.com/video/BV137yHB4ERo

## Authors and Licenses

- Code: 一水合氨基酸 (NH3andH2O), licensed under `Apache-2.0`
- 3D models
    - Third-eye assembly `3rd_eyes`: 牧鈴華SUZUKA. The original author granted permission to upload it to this GitHub repository. Contact the original author before using it for any other purpose ([project page](https://www.bilibili.com/video/BV1rN1gYJE3K)).
    - Heart-shaped accessory `heart_shaped`: 一水合氨基酸, licensed under `CC BY 4.0`
- PCD
    - `3rd_eyes`: Modified from a design by R饼锐度不够, licensed under `CC BY-NC-SA 3.0` ([project page](https://oshwhub.com/flanker-e/jue-zhi-tong-esp32))
    - `GC9A01`: 一水合氨基酸, licensed under `CC BY-NC-SA 3.0`

## Asset Credits

- Third-eye web interface icon: 幽蝶_Zomgel

## Project Structure

- `3d_model`: 3D models
- `pcd`: PCD design files
- `code`: Source code
- `tools`: Tools for debugging device sensors

## Getting Started

Download this repository and read the README in each directory.  
Recommended order: [pcd](pcd/README.md) → [3d_model](3d_model/README.md) → [code](code/README.md)

## Controls

- After uploading the firmware, the device enters gyroscope-tracking mode by default. Attach the heart-shaped accessory to your head and secure the third eye to your body to control it.
- To change the settings, connect to the `3rd-Eyes` Wi-Fi network (no password), then open `3rdeyes.local` to change the control mode and settings.

## Roadmap

- Add autonomous third-eye movements, such as blinking and head shaking
- Add controller support

## Contact

Join the project QQ group: `727559409`