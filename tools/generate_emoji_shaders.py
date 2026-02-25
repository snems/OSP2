#!/usr/bin/python3
import os

emoji_path = '../assets/gfx/emoji/'
shader_path = '../assets/scripts/'
contents = os.listdir(emoji_path)

emoji_built_in = [
('gauntlet', 'icons/iconw_gauntlet.tga'),
('mg', 'icons/iconw_machinegun.tga'),
('sg', 'icons/iconw_shotgun.tga'),
('gl', 'icons/iconw_grenade.tga'),
('rl', 'icons/iconw_rocket.tga'),
('lg', 'icons/iconw_lightning.tga'),
('rg', 'icons/iconw_railgun.tga'),
('pg', 'icons/iconw_plasma.tga'),
('bfg', 'icons/iconw_bfg.tga'),
('hook', 'icons/iconw_grapple.tga'),
('ga', 'icons/iconr_green.tga'),
('ya', 'icons/iconr_yellow.tga'),
('ra', 'icons/iconr_red.tga'),
('shard', 'icons/iconr_shard.tga'),
('mh', 'icons/iconh_mega.tga'),
('50hp', 'icons/iconh_red.tga'),
('25hp', 'icons/iconh_yellow.tga'),
('5hp', 'icons/iconh_green.tga'),
('quad', 'icons/quad.tga'),
('regen', 'icons/regen.tga'),
('invis', 'icons/invis.tga'),
('battlesuit', 'icons/envirosuit.tga'),
('flight', 'icons/flight.tga'),
('haste', 'icons/haste.tga'),
('medkit', 'icons/medkit.tga'),
('teleporter', 'icons/teleporter.tga'),
('accuracy', 'menu/medals/medal_accuracy.tga'),
('assist', 'menu/medals/medal_assist.tga'),
('capture', 'menu/medals/medal_capture.tga'),
('defend', 'menu/medals/medal_defend.tga'),
('excellent', 'menu/medals/medal_excellent.tga'),
('frags', 'menu/medals/medal_frags.tga'),
('humiliation', 'menu/medals/medal_gauntlet.tga'),
('impressive', 'menu/medals/medal_impressive.tga'),
('victory', 'menu/medals/medal_victory.tga'),
('anarki', 'models/players/anarki/icon_default.tga'),
('biker', 'models/players/biker/icon_default.tga'),
('hossman', 'models/players/biker/icon_hossman.tga'),
('cadavre', 'models/players/biker/icon_cadavre.tga'),
('bitterman', 'models/players/bitterman/icon_default.tga'),
('bones', 'models/players/bones/icon_default.tga'),
('crash', 'models/players/crash/icon_default.tga'),
('doom', 'models/players/doom/icon_default.tga'),
('phobos', 'models/players/doom/icon_phobos.tga'),
('grunt', 'models/players/grunt/icon_default.tga'),
('stripe', 'models/players/grunt/icon_stripe.tga'),
('hunter', 'models/players/hunter/icon_default.tga'),
('harpy', 'models/players/hunter/icon_harpy.tga'),
('keel', 'models/players/keel/icon_default.tga'),
('keelpm', 'models/players/keel/icon_pm.tga'),
('klesk', 'models/players/klesk/icon_default.tga'),
('flisk', 'models/players/klesk/icon_flisk.tga'),
('lucy', 'models/players/lucy/icon_default.tga'),
('angel', 'models/players/lucy/icon_angel.tga'),
('major', 'models/players/major/icon_default.tga'),
('daemia', 'models/players/major/icon_daemia.tga'),
('mynx', 'models/players/mynx/icon_default.tga'),
('orbb', 'models/players/orbb/icon_default.tga'),
('ranger', 'models/players/ranger/icon_default.tga'),
('wrack', 'models/players/ranger/icon_wrack.tga'),
('razor', 'models/players/razor/icon_default.tga'),
('razor-patriot', 'models/players/razor/icon_patriot.tga'),
('razor-id', 'models/players/razor/icon_id.tga'),
('sarge', 'models/players/sarge/icon_default.tga'),
('krusade', 'models/players/sarge/icon_krusade.tga'),
('roderic', 'models/players/sarge/icon_roderic.tga'),
('slash', 'models/players/slash/icon_default.tga'),
('yuriko', 'models/players/slash/icon_default.tga'),
('grrl', 'models/players/slash/icon_grrl.tga'),
('sorlag', 'models/players/sorlag/icon_default.tga'),
('tankjr', 'models/players/tankjr/icon_default.tga'),
('tankjrpm', 'models/players/tankjr/icon_pm.tga'),
('uriel', 'models/players/uriel/icon_default.tga'),
('zael', 'models/players/uriel/icon_zael.tga'),
('visor', 'models/players/visor/icon_default.tga'),
('gorre', 'models/players/visor/icon_gorre.tga'),
('xaero', 'models/players/xaero/icon_default.tga'),
('quake3', 'menu/art/skill4.tga'),
('nightmare', 'menu/art/skill5.tga')]

def print_shader(name, path):
    return '\n'\
           'gfx/emoji/' + name + '\n' \
           '{\n' \
           '    nopicmip\n' \
           '    nomipmaps\n' \
           '  {\n' \
           '    map ' + path + '\n'\
           '    blendfunc blend\n' \
           '    rgbgen vertex\n' \
           '  }\n' \
           '}\n'  

def print_doc(name):
    return '`:' + name + ':`\n'

files_list = []
for item in contents:
    full_path = os.path.join(emoji_path, item)
    if os.path.isfile(full_path) and (full_path.endswith('.png') or full_path.endswith('.PNG')) :
        files_list.append(item)


emoji_config = ""
emoji_shaders = ""
emoji_doc = "# Available emoji\n\n"

# built in emoji
for name, filename in emoji_built_in:
    if emoji_config:
        emoji_config = emoji_config + '\n'
    emoji_config = emoji_config + name
    emoji_shaders = emoji_shaders + print_shader(name, filename)
    emoji_doc = emoji_doc + print_doc(name)

# emoji from files
for filename in files_list:
    no_ext = filename[:-4]
    if emoji_config:
        emoji_config = emoji_config + '\n'
    emoji_config = emoji_config + no_ext
    emoji_shaders = emoji_shaders + print_shader(no_ext, 'gfx/emoji/' + filename)
    emoji_doc = emoji_doc + print_doc(no_ext)

with open(emoji_path + "emoji.cfg", 'w') as file:
    file.write(emoji_config)

with open(shader_path + "emoji.shader", 'w') as file:
    file.write(emoji_shaders)

with open('../docs/emoji.md', 'w') as file:
    file.write(emoji_doc)





