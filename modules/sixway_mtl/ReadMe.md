# 🌫️ Six-Way Volumetric Lighting Shader for Blazium

A **Volumetric Lighting Shader** for Blazium using a **Six-Directional Lighting Model (SDLM)** designed to simulate realistic light interaction within semi-transparent volumes like cloud, smoke, fog and fire as C++ Module.

It's developed with artistic flexibility and lightweight performance in mind, It allows you to simulate light scattering and absorption effects using three precomputed 2D texture maps.

![Clouds-Demo](https://github.com/user-attachments/assets/15ffdc9d-4f07-4ce2-9779-acefb63e3db8)

---

## 🧠 How It Works

Unlike traditional single-source lighting, this shader breaks down the environment lighting into **six principal directions**, combining their influence dynamically in the shader:

- +X / -X
- +Y / -Y
- +Z / -Z

However, instead of using six separate textures, this shader uses **three dual-direction maps**:

- `RTB`: Right (+X), Top (+Y), Back (+Z)
- `LBF`: Left (−X), Bottom (−Y), Front (−Z)
- `TEA`: Transparency, Emission, Ambient Occlusion 

These maps are blended based on the light direction at runtime, simulating how light wraps and diffuses through volume.

---

## ✨ Shader Features

- 🧊 **Directional Light Simulation** using six-way blending
- 📦 **Three Texture Inputs** for compact directional encoding
- 🔥 **Support for Normal Maps** to enhance detail and light response
- 🌈 **Emission Ramp** support for colorful fire, glow, or mystical effects
- ☁️ Adjustable **Absorption**, **Scattering**, and **Density**
- 🧪 Experimental **Ambient Occlusion** and **Thickness** control
- 🖼️ Y-Axis Only and Full **Billboarding Support**
- 🎞️ Support for **Flip Book Animation** and **Sprite Sheet**
- ☀️ Support for **SDFGI**, **Volumetric Fog**, **Fog** and **IBL**

---

## 🧰 Usage

1. Add a `MeshInstance3D` node to your scene.
2. Create a new `SixWayLightingMaterial` and assign it to a `MeshInstance3D`.
3. Provide 2D textures for the six-way lighting maps and adjust lighting parameters to taste.
4. Optionally, enable `Billboard` for smoke-like behavior facing the camera.

---

## 📜 License

**Creative Commons Attribution 4.0 (CC BY 4.0)**  
You are free to use this shader for any purpose, including personal, educational, and commercial projects. Attribution is required, and you must give appropriate credit to the original creator. You may adapt and redistribute the shader freely under the same license terms.

---

## 🧑‍💻 Author

Developed by **Hamid Memar**  
Shader version: **v2.0**

---

## 📷 Preview

https://github.com/user-attachments/assets/f9bef5fc-f4d6-4439-87ca-5ea2da359478

---

## 🔮 Tips

- Baked directional lighting textures can be created in Blender, Houdini, EmberGen or other volumetric renderers.
- The shader works great for stylized clouds, fire, fantasy fog, explosions, and magical FX.
- This shader is optimized for performance, making it ideal for use on mobile devices.

---

Enjoy pushing the limits of volumetric visuals in Blazium! 🌌