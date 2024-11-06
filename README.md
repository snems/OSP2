# OSP2

---

![OSP2 Screenshot](https://github.com/snems/OSP2/blob/master/docs/shot.jpg?raw=true)  
[![Mod State](https://img.shields.io/badge/Mod%20State-Development-yellow)](https://github.com/snems/OSP2)

---

**OSP2** is a Quake 3 mod aimed at recovering old OSP mod source codes and improving them.

Based on vanilla Q3 and OSP files analysis, the project also incorporates code from several other mods.

---

### Code Sources
> - **[baseq3a from Cyrax](https://github.com/ec-/baseq3a)**
> - **[XQ3E Engine](https://github.com/xq3e/engine)**
> - Unlagged mod written by Nail Toronto
> - **[Ratmod](https://github.com/rdntcntrl/ratoa_gamecode)**

---

## Features

- **VQ3/Promode physics support**
- **CPMA-like SuperHUD**
- **Proportional fonts**
- **Multilevel hitsounds and fragsound**
- **Crosshair decoration and animations**
- **Client-side /mute for players**
  
And many other small features!

---

## Mod State
> Only **cgame** is available now.

---

## Contacts

- **Telegram channel**: [Join here](https://t.me/q3osp2)
- **Telegram chat**: [Join the conversation](https://t.me/q3_osp2)

---

## New Commands

See the list of commands in: [docs/commands.md](docs/commands.md)  
Example config: **OSP2.cfg** inside the mod folder.

---

## How to Install

1. Install Quake3 and OSP mod v1.03.
2. Copy the required packs or libraries into the `osp` folder.

---

## How to Build

Currently, only **Linux QVM** is supported. Other builds are not supported yet.

#### Linux QVM

```bash
cd build/linux-qvm
make
