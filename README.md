# OSP2

This Quake 3 mod is an attempt to recover old OSP mod source codes and improve it.

Based on vanilla Q3 and OSP files analysis. 

Also used code from :

 - baseq3a from Cyrax: https://github.com/ec-/baseq3a
 - XQ3E: https://github.com/xq3e/engine, 
 - unlagged mod written by Neil Toronto 
 - Ratmod https://github.com/rdntcntrl/ratoa_gamecode


# Features

- VQ3/Promode physics support
- CPMA like SuperHUD
- Proportional fonts
- Multilevel hitsounds and fragsound
- Crosshair decoration and animations
- Client side /mute for players

And many other small features

![alt text](https://github.com/snems/OSP2/blob/master/docs/shot.jpg?raw=true)

## Mod state

Only cgame now.


## Contacts

Telegram [channel](https://t.me/q3osp2).

Telegram [chat](https://t.me/q3_osp2).

## New commands

See [docs/osp2-q3-ClientReadme.md](docs/osp2-q3-ClientReadme.md)

Example config: OSP2.cfg inside mod.

## How to install

> [!CAUTION]
> **OSP 1.03a** files are required for this mod to run. Download them from [ModDB](https://www.moddb.com/mods/osp/downloads) if you don't have classic OSP installed.

Download `osp2_pk3.zip` from **Releases** and extract `zz-osp-pak8.pk3` into `osp` folder, located in your Quake 3 Arena directory. Original **OSP 1.03a** should also be stored in `osp` folder.

### Native libraries

> [!NOTE]
> If you don't know what are "native libraries", you don't need them. This section is for advanced users.

Download `osp2_windows_mingw.zip` or `osp2_linux.zip`, depending on your system, and extract `cgame.so`/`cgame.dll` into `osp` folder.

Native libraries are not loaded by default. Enable them through `vm_cgame 0` and `sv_pure 0`:
```bash
./q3client +set fs_game "osp" +set sv_pure 0 +set vm_cgame 0
```

The logs should show successful load of **native** game modules. Example on linux:
```logs
Try loading dll file ./osp/cgamex86_64.so
Loading DLL file: ./osp/cgamex86_64.so
Sys_LoadGameDll(./osp/cgamex86_64.so) found vmMain function at 0x7f50e7410269
```

Note: **game** and **ui** may load as qvm. **OSP2** currently does not implement them.

# How to build

Follow instructions corresponding to your system:
  - [Building under linux](#linux)
  - [Building under windows](#windows-mingw)

## Linux

Install `cmake`, `gcc` and `make`. For systems with `apt` packet manager:
```bash
sudo apt install cmake make gcc
```

### Configure the project

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

### Compile

Limit thread count with `--parallel <threads>` option and compile the mod:

```bash
cmake --build build --parallel 8
```

### Install
`build/Release/osp` now contains:
 - `cgame.so` - native library build.
 - `zz-osp-pak8.pk3` - assets and qvms.

Install it as described in [installing instructions](#how-to-install).

## Windows (MinGW)

Install [MinGW](https://github.com/skeeto/w64devkit/releases), [CMake](https://cmake.org/cmake/download) and [Ninja](https://github.com/ninja-build/ninja/releases). Add their binary folders to system's PATH. It should look somewhat like this:
```cmd
set PATH=%PATH%;<MinGW>/bin;<CMake>/bin;<Ninja>
```

Ninja is a portable application, so its binary folder is whatever folder you extracted it in.

### Configure the project

```cmd
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
```

### Compile

Limit thread count with `--parallel <threads>` option and compile the mod:

```bash
cmake --build build --parallel 8
```

### Install
`build/Release/osp` now contains:
 - `cgame.dll` - native library build.
 - `zz-osp-pak8.pk3` - assets and qvms.

Install it as described in [installing instructions](#how-to-install).
