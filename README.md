# OSP2

This Quake 3 mod is an attempt to recover old OSP mod source codes and improve it.

Based on vanilla Q3 and OSP files analysis. 

Also used code from :

 - baseq3a from Cyrax: https://github.com/ec-/baseq3a
 - XQ3E: https://github.com/xq3e/engine, 
 - unlagged mod written by Nail Toronto 
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

See [docs/commands.md](docs/commands.md)

Example config: OSP2.cfg inside mod.

## How to install

- Install Quake3 and OSP mod v1.03
- copy pk3 file (and so/dll library if you want) into `osp` folder

## How to build

### Windows

Install [mingw](https://github.com/skeeto/w64devkit/releases) (just unzip and add bin folder to system PATH).

### pk3

   `cd build/qvm`

   `make` 

### dll/so

   `cd build/library`

   `make` 

