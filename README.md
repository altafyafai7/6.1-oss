# Android Kernel Xiaomi Sky (SM4450) - Kernel 6.1 GKI

This repository contains the **Kernel 6.1 GKI** source code for the Xiaomi **sky** (Redmi 12 5G / Poco M6 Pro 5G) platform, ported from the original 5.10 vendor source.

## Technical Overview
- **Base Kernel:** Android Common Kernel (ACK) 6.1 (android14-6.1-staging)
- **Target SoC:** Snapdragon 4 Gen 2 (SM4450 / Ravelin / Parrot)
- **Architecture:** ARM64
- **GKI Version:** 2.0 (GKI-native)

## Features & Ported Components
This kernel has been specifically modified to support the hardware features of the Xiaomi `sky` device:
- **Touchscreen Drivers:** Ported Xiaomi touchfeature suite, including support for:
  - FocalTech FT8720
  - Novatek NT36672C (SPI)
  - Goodix Berlin Series (BRL)
- **Xiaomi Notifiers:** Full implementation of Panel, Touch, USB, and Headset notifiers.
- **Hardware Info:** Backported `hardware_info.ko` support for proper peripheral detection.
- **Modern GKI Support:** Fully compliant with Android 14/15 GKI requirements.

## Build Requirements
- **Toolchain:** Clang 19+ (LLVM-based build only)
- **LTO:** Thin LTO supported (Full LTO requires 16GB+ RAM)
- **CFI:** Kernel Control Flow Integrity (KCFI) enforced

## Configuration
The build uses a merged configuration:
1. `gki_defconfig` (Base)
2. `sky_GKI.fragment` (Vendor specific)

To generate the configuration manually:
```bash
make ARCH=arm64 LLVM=1 gki_defconfig sky_GKI.fragment
make ARCH=arm64 LLVM=1 olddefconfig
```

## Credits
- **Qualcomm:** Original Ravelin/SM4450 platform code.
- **Google:** Android Common Kernel (GKI) base.
- **Xiaomi:** Proprietary vendor driver source.
- **Altaf Yafai:** Upstreaming and maintenance.
- **Suvojeet Sengupta:** Integration and porting efforts.
