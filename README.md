# Quack
A graphics project focusing on trying to **rasterize a scene as quickly as possible** on a CPU with a combination of
algorithmic (usage of scanlines, culling), data-driven (memory layout) and SIMD approaches. It is written in C, and the final
texture is rendered using a SDL3 render target.

## Screenshots
![quack-opt](https://github.com/user-attachments/assets/800b9f00-b73d-43ab-ae17-6c324040a04d)
![quack-opt](https://github.com/user-attachments/assets/8629bc04-9c52-4a3a-a0bc-4954a4e5ffba)

## Dependencies
### Libraries
- SDL3 ([source](https://github.com/libsdl-org/SDL), [license](https://github.com/libsdl-org/SDL?tab=Zlib-1-ov-file#readme))
- cgltf ([source](https://github.com/jkuhlmann/cgltf), [license](https://github.com/jkuhlmann/cgltf?tab=MIT-1-ov-file#readme))
- Pillow ([source](https://github.com/python-pillow/Pillow), [license](https://github.com/python-pillow/Pillow?tab=License-1-ov-file#readme))
- STB ([source](https://github.com/nothings/stb), [license](https://github.com/nothings/stb?tab=License-1-ov-file#readme))

### 3D models
- "Autumn House" (https://skfb.ly/o7KNs) by Zhenya is licensed under Creative Commons Attribution (http://creativecommons.org/licenses/by/4.0/).
- "Fox" (https://github.com/KhronosGroup/glTF-Sample-Assets/tree/main/Models/Fox): CC BY 4.0 International [SPDX license identifier: "CC-BY-4.0"]
- "Lantern" (https://github.com/KhronosGroup/glTF-Sample-Assets/blob/main/Models/Lantern): CC0 1.0 Universal [SPDX license identifier: "CC0-1.0"]
