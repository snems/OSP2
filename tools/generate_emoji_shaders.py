#!/usr/bin/python3
import os

emoji_path = '../assets/gfx/emoji/'
shader_path = '../assets/scripts/'
contents = os.listdir(emoji_path)

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





