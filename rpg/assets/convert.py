import sys
import xml.etree.ElementTree as ET
from PIL import Image


def convert_png(name, is_pf_b):
    im = Image.open("%s.png" % name)
    rgb_im = im.convert('RGB')
    width, height = rgb_im.size

    palette = []
    palette.append((0, 0, 0))
    for y in range(height):
        for x in range(width):
            rgb = rgb_im.getpixel((x, y))
            if not rgb in palette:
                palette.append(rgb)

    if len(palette) > 16:
        print("Too many colors (%d)", len(palette))
        sys.exit(1)

    print("uint16_t %s_palette[16] = {" % name)
    for p in palette:
        if is_pf_b and not p == (0,0,0):
            a = 15
        else:
            a = 0
        r = p[0] >> 4
        g = p[1] >> 4
        b = p[2] >> 4
        print("0x%x%x%x%x," % (a,r,g,b))
    for i in range(16 - len(palette)):
        print("0x0000,")
    print("};")

    print("uint16_t %s_bitmap[] = {" % name)
    s = ""
    for y in range(height):
        for x in range(width):
            rgb = rgb_im.getpixel((x, y))
            index = palette.index(rgb)
            s = s + "%x" % index
            if x % 4 == 3:
                print("0x" + s + ",")
                s = ""
    print("};")

def convert_map(name):
    tree = ET.parse("%s.tmx" % name)
    root = tree.getroot()
    layer = root.findall("./layer")[0]
    print("#define %s_WIDTH %s" % (name.upper(), layer.attrib['width']))
    print("#define %s_HEIGHT %s" % (name.upper(), layer.attrib['height']))

    print("uint8_t %s_data[] = {" % name)
    print(layer.findall("./data")[0].text)
    print("};")

print("#ifndef ASSETS_H")
print("#define ASSETS_H")
print("#include <stdint.h>")

convert_png("tiles", False)
convert_png("bobs", True)
convert_map("map0")

print("#endif // ASSETS_H")
