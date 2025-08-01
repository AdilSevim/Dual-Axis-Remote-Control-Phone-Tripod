# Dual-Axis Remote Control Phone Tripod
> **Purpose:** A robust, IR‑remote–controlled, two‑axis (pan/tilt) phone mount designed to sit on a standard tripod and move your smartphone smoothly in 90° increments for framing, demos, time‑lapses, and STEM projects.  
> **Original build date:** **July 2024**
> **Author:** **Adil Sevim**

---

## Table of Contents
- [Key Features](#key-features)
- [Repository Layout](#repository-layout)
- [Bill of Materials (BOM)](#bill-of-materials-bom)
- [3D Printing](#3d-printing)
- [Mechanical Assembly](#mechanical-assembly)
- [Electronics & Wiring](#electronics--wiring)
- [Firmware & Setup](#firmware--setup)
- [Operation](#operation)
- [Calibration & Tuning](#calibration--tuning)
- [Safety Notes](#Safety-notes)
- [Troubleshooting](#troubleshooting)
- [License](#license)
- [Credits & Contact](#credits--contact)
- [Changelog](#changelog)
- [Disclaimer](#disclaimer)

---

## Key Features
- **IR Remote Control:** Use common NEC‑type IR remotes; arrow keys map to **exact 90° turns**.
- **Smooth Motion:** Microstepping + trapezoidal accel/decel for fluid, low‑jerk movement.
- **Two‑Axis Control:** Pan (X) and Tilt (Y) with independent stepper motors.
- **Learn Mode:** Built‑in IR code learning with EEPROM persistence (no hard‑coded key codes).
- **STEM‑Friendly:** Printable parts, approachable electronics, commented firmware.
- **Tripod‑Ready:** Base designed to mount on standard photo tripods (¼‑20 UNC or M5 adapter, depending on your insert).

---

## Repository Layout
```text
.
├─ firmware/
│  └─ Stepper_IR_Controller/
│     └─ Stepper_IR_Controller.ino      # Single-file sketch: IR learn + 90° moves
├─ 3d_parts/                             # STL/STEP files for all printed parts
│  ├─ base_mount.stl
│  ├─ phone_holder.stl
│  ├─ gear_pan.stl
│  ├─ gear_tilt.stl
│  ├─ bracket_pan.stl
│  ├─ bracket_tilt.stl
│  ├─ INFO.md                            # Printing & lubrication notes
│  └─ LICENSE                            # CC BY-NC 4.0 (non-commercial)
├─ docs/
│  ├─ assembly_overview.png              # Exploded/assembly view (placeholder)
│  └─ wiring_diagram.png                 # Wiring diagram (placeholder)
├─ LICENSE                               # PolyForm Noncommercial 1.0.0 (software)
├─ NOTICE
└─ README.md
```



## Bill of Materials (BOM)
> The quantities below are for one full build. Substitute equivalents as needed.  
> **Filament target:** up to **1 kg PLA** (actual usage commonly 350–600 g depending on slicer settings).

### Electronics
| Item | Qty | Notes |
|---|---:|---|
| **Arduino UNO** (or compatible) | 1 | Primary MCU. |
| **Stepper Motor Shield** (Adafruit Motor Shield V2 or L293D‑based shield) | 1 | Drives 2 steppers; V2 (I²C) recommended. |
| **NEMA‑17 Stepper Motor** | 2 | Typical 1.8° (200 steps/rev). 0.9° works with config change. |
| **IR Receiver** (e.g., VS1838B) | 1 | Signal → D2 by default. |
| **IR Remote** (NEC‑type) | 1 | Any standard 21/24‑key IR remote works. |
| **DC Power Supply** | 1 | 9–12 V, ≥2 A recommended (depends on motors). |
| **Jumper wires & connectors** | as needed | Dupont or JST. |
| **Heat‑set inserts** (optional) | assorted | M3/M5 inserts for cleaner assembly (optional). |

### Mechanical & Fasteners
| Item | Qty | Notes |
|---|---:|---|
| **Tripod** | 1 | Standard photo tripod. |
| **M5 bolt** (for tripod interface) | 1 | Depending on your base; some tripods use ¼‑20—use adapter if needed. |
| **M3 socket head screws** | 20+ | Mixed lengths (e.g., 8 mm, 12 mm, 16 mm, 20 mm). |
| **M3 nuts** | 20+ | Nylock preferred at high‑vibration points. |
| **M3 washers** | 20+ | Reduces friction & protects plastic. |
| **Set screws** for gears (M3) | 2–4 | To lock printed gears to motor shafts. |
| **Threadlocker** | small | Medium strength (blue) for metal‑to‑metal. |
| **Zip ties / cable clips** | as needed | Cable management. |
| **PTFE or silicone‑based lubricant** | small | For gear faces; apply lightly. |

> If you plan to use bearings/spacers, list them here according to your variant (e.g., 625ZZ).

---

## 3D Printing
**Material:** Parts can be printed in **PLA or ABS** with supports.  
**Validated baseline:** **PLA**. **No functional guarantee for ABS.**  
**General slicer profile (guideline):**
- Layer height: **0.20 mm** (0.16 mm for cosmetics)
- Perimeters (walls): **≥3**
- Infill: **25–40 %** (gears/brackets closer to **40 %+**)
- Supports: **Enabled** for overhangs (tuned per part)
- Top/bottom: 4–6 layers
- Orientation: Print gears flat for best tooth shape; avoid excessive bridging on brackets.

**Post‑processing:**
- Deburr holes and gear teeth lightly.
- **Light lubrication** on gear faces (PTFE/silicone). Keep grease away from electronics.
- Consider heat‑set inserts for repeated assembly/disassembly.

For part‑specific notes, see **`3d_parts/INFO.md`**.

---

## Electronics & Wiring
- **Motors:**  
  - **Pan (X):** Shield **PORT 1**  
  - **Tilt (Y):** Shield **PORT 2**
- **IR Receiver:**  
  - **Signal** → **D2** (default)  
  - **VCC** → **5 V**  
  - **GND** → **GND**
- **Learn‑Mode pin (optional):** **D4** (pull **LOW** on boot to enter Learn Mode)
- **Power:** 9–12 V DC, ≥2 A depending on your steppers and load. Power the motors through the shield’s motor supply jack/terminals.

---

## Firmware & Setup
1. Open **`firmware/Stepper_IR_Controller/Stepper_IR_Controller.ino`** in **Arduino IDE 2.x**.
2. Install libraries via **Library Manager**:  
   - `Adafruit_MotorShield` (V2), `IRremote` (v3+), `EEPROM`.
3. Select **Board:** *Arduino UNO* and the correct **Port**.
4. Upload the sketch.

**First‑run (IR Learn Mode):**
- If no codes exist in EEPROM, the controller prompts you (via Serial) to press **UP, DOWN, LEFT, RIGHT** on your remote.  
- Codes are saved to EEPROM and persist across reboots.

**Re‑learn:**  
- Hold **D4 = LOW** during power‑up, **or** send **`L`** over Serial to re‑enter Learn Mode.

---

## Operation
- **LEFT/RIGHT:** Pan −90°/+90°  
- **UP/DOWN:** Tilt +90°/−90°  
- Motion uses **microstepping** and a **trapezoidal profile** to minimize jerk.

> Short presses register a single 90° move. Repeat frames from a long press are ignored by design.

---

## Calibration & Tuning
- **Step angle:** If your motors are **1.8°** set `STEPS_PER_REV = 200`; for **0.9°** set `400`.
- **Microstepping:** `MICROSTEPS = 16` for Adafruit V2; adjust if your shield differs.
- **Feel tuning:** Increase `ACCEL_SEGMENT` and/or `START_DELAY_US` for smoother starts; lower `MIN_DELAY_US` for faster moves (ensure your motors can handle it).
- **Torque:** If slipping occurs, raise current capability (within safe limits), increase gear ratio, or reduce payload.

---

## Safety Notes
- Check tripod stability and clamp your phone securely before power‑up.
- Keep fingers clear of gears and rotating parts during motion.
- Prevent lubricant from contacting the phone or electronics.
- Provide ventilation for drivers; avoid sustained stalls that overheat motors.

---

## Troubleshooting
| Symptom | Likely Cause | Fix |
|---|---|---|
| No motor motion | Power/wiring/shield not seated | Reseat shield, verify motor power, check library install |
| Moves wrong axis | Ports swapped | Swap ports or update firmware axis mapping |
| Overshoot/undershoot vs. 90° | Wrong steps/rev or microsteps | Set `STEPS_PER_REV` and `MICROSTEPS` correctly |
| Jerky motion | Too aggressive timing | Increase `START_DELAY_US` / `ACCEL_SEGMENT` |
| Random behavior | IR noise or wiring cross‑talk | Separate IR receiver from motor wires; use shielded leads if needed |

---

## License
- **Software (code):** **PolyForm Noncommercial 1.0.0** — **commercial use is prohibited**. See **`/LICENSE`** for full text.  
- **3D models & documentation:** **CC BY‑NC 4.0** — **attribution required, commercial use prohibited**. See **`/3d_parts/LICENSE`** for full text.  
- Suggested attribution: **“Adil Sevim – 2‑Axis Phone Tripod with Remote Control”** (link to this repository).

> This project is intended for scientific/educational/STEM use. Commercial use is **not** permitted.

---

## Credits 
All **3D designs**, **firmware**, and **documentation** by **Adil Sevim**.  
Questions or collaboration inquiries: *[adilsevim18@gmail.com]*

---

## Changelog
- **v1.0.0 — July 2024:** Initial design, printable parts, IR control, smooth 90° motion.

---

## Disclaimer
This project is provided **“as is”** without warranty of any kind. You assume all risks associated with building and operating the device.

---

## Contact

**Adil Sevim** — Developer  
- GitHub: https://github.com/<AdilSevim>  
- LinkedIn: https://www.linkedin.com/in/adilsevim/  
- Email: <adilsevim18@gmail.com>

---

