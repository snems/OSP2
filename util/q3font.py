from PIL import Image, ImageFont, ImageDraw
file_in = "/home/snems/Downloads/ofont.ru_Diablo.ttf"
file_out = "/home/snems/tmp/diablo.png"
file_template = "./template.tga"
fontsize = 52

template_size_x = 1024
template_size_y = 1024
char_size_x = template_size_x / 16
char_size_y = template_size_y / 16

q3_ascii_table = \
"                " \
"                " \
" !\"#$%&'()*+,-./" \
"0123456789:;<=>?" \
"@ABCDEFGHIJKLMNO" \
"PQRSTUVWXYZ[\]^_" \
"`abcdefghijklmno" \
"pqrstuvwxyz{|}~ " \
"                " \
"  012345678     " \
" !\"#$%&'()*+,-./" \
"0123456789:;<=>?" \
"@ABCDEFGHIJKLMNO" \
"PQRSTUVWXYZ[\]^_" \
"'ABCDEFGHIJKLMNO" \
"PQRSTUVWXYZ{|}\" "

font = ImageFont.truetype(file_in, fontsize) 
image = Image.open(file_template) 
draw = ImageDraw.Draw(image)


x = 0
y = 0
i = 0
x_offset = 4
y_offset = 0

for char in q3_ascii_table:
    if char != ' ':
        if char == 'g':
            x_offset = 4
            y_offset = -4
        elif char == 'y':
            x_offset = 4
            y_offset = -4
        elif char == 'J':
            x_offset = 4
            y_offset = -4
        elif char == 'j':
            x_offset = 9
            y_offset = -4
        elif char == 'W':
            x_offset = -6
            y_offset = 0
        else:
            x_offset = 4
            y_offset = 0

        y_offset = y_offset + 4

        draw.text((x+x_offset, y+y_offset+4), char, (255, 255, 255, 255), font=font, anchor="la")

    i = i + 1
    x = x + char_size_x
    if x == template_size_x:
        x = 0
        y = y + char_size_y


image.save(file_out, "PNG")





