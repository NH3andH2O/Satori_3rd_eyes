English | [简体中文](README_zh-cn.md) | [繁體中文](README_zh-tw.md) | [日本語](README_ja.md)

# Mechanical Satori's Third Eye Models

## Other Authors

- **3rd_eyes:** 牧鈴華SUZUKA ([project page](https://www.bilibili.com/video/BV1rN1gYJE3K))

## Usage

Print all `.stl` files with a 3D printer, then assemble them with reference to `STR-rc.prt`.

## Notes

- **Servos:** Use MG90S 180-degree servos. Center them at 90 degrees before installation.
  1. Use the program to move each servo to its center position.
     > [!TIP]
     > After flashing the main firmware, open the third-eye calibration page in the web interface and set all three servos to 90 degrees before installing the servo horns and gears.
  2. Install the servo horns and gears as shown below:  
     <img src="https://github.com/user-attachments/assets/966a090c-c51c-4c40-b57b-3a016ed3d949" width="300" />\
     Upper-eyelid servo horn position

     <img src="https://github.com/user-attachments/assets/13863228-87b3-4ce6-bbb7-370c8e705013" width="300" />\
     Eyeball servo horn position. The servo horn should be perpendicular to the servo.

     <img src="https://github.com/user-attachments/assets/527a5b8f-8182-4cc5-b2bd-1d44201bb876" width="300" /> \
     Lower-eyelid gear position. First adjust the gear shaft (yellow area) so that the rod notch faces directly outward, as indicated by the blue mark, and then mesh it with the gear on the servo.
- **About `pupil_GC9A01.prt`:** This is a slotted version of `pupil.prt`. A round transparent acrylic sheet with a thickness of `0.5 mm` and a diameter of `26.8 mm` can be fitted into the center.
- **About `heartShaped_dowm.prt`:** The bottom has mounting holes and can be secured to a headpiece with cable ties.
