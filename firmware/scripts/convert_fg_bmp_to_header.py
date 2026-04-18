Import("env")

import os
from PIL import Image

def png_to_data(img_dir, filename):
    file_stem = filename.split('.')[0]

    data = []

    img = Image.open(f"{img_dir}/{filename}")
    width, height = img.size

    # Convert image to RGBA mode if it isn't already
    img = img.convert("RGBA")

    # Get pixel data
    for y in range(height):
        for x in range(width):
            r, g, b, a = img.getpixel((x, y))
            data.append((a << 24) | (r << 16) | (g << 8) | b)

    return data, width, height

def data_argb_to_palette_al44(data):
    out_data = []

    # Convert data from ARGB8888 to AL44
    palette = [
        0xFFFFFF, 0x000000, 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00, 0xFF00FF,
        0x00FFFF, 0x808080, 0xB0B0B0, 0x404040,
    ]

    for pix in data:
        a = (pix >> 24) & 0xFF
        r = (pix >> 16) & 0xFF
        g = (pix >> 8) & 0xFF
        b = pix & 0xFF

        # Find the closest color in the palette
        closest_color = min(palette, key=lambda c: (r - ((c >> 16) & 0xFF))**2 + (g - ((c >> 8) & 0xFF))**2 + (b - (c & 0xFF))**2)

        # Get the index of the closest color in the palette
        index = palette.index(closest_color)

        # Convert to AL44 format
        al44_pix = (a & 0xF0) | index

        out_data.append(al44_pix)

    return out_data

def data_reorg(data, width, height):

    out_data = []

    for y in range(height):
        for x in range(width):
            out_data.append(data[x + (height - 1 - y) * width])

    return out_data

def data_to_header(header_dir, filename, data, width, height):
    file_stem = filename.split('.')[0]

    header = f"#ifndef {file_stem.upper()}_H\n#define {file_stem.upper()}_H\n\n"

    header += f"#include \"graphics.h\"\n\n"
    
    header += f"const uint8_t __attribute__((section(\".ext_rodata\"))) {file_stem.upper()}_DATA[] = {{\n"

    line_width = 8
    line_index = 0
    for pix in data:
        header += f"0x{pix:02x}, "
        line_index += 1
        if line_index == line_width:
            header += '\n'
            line_index = 0

    header += f"}};\n\n"
    
    header += f"const Graphics {file_stem.upper()} = {{\n    .width = {width},\n    .height = {height},\n    .data = {file_stem.upper()}_DATA,\n}};\n\n"
    
    header += f"#endif // {file_stem.upper()}_H"

    # Save to a header with the same stem
    header_file = open(f"{header_dir}/{file_stem}.h", "w")
    header_file.write(header)
    header_file.close()

def before_build(source, target, env):
    print("Converting PNGs to header data")

    if not os.path.exists("graphics"):
        print("Graphics folder does not exist, creating...")
        os.mkdir("graphics")

    if not os.path.exists("include/graphics"):
        print("Graphics header folder does not exist, creating...")
        os.mkdir("include/graphics")

    for path in os.listdir("graphics"):
        print(path)
        data, width, height = png_to_data("graphics", path)
        data = data_reorg(data, width, height)
        data = data_argb_to_palette_al44(data)
        data_to_header("include/graphics", path, data, width, height)

# env.AddPreAction("buildprog", before_build)
before_build(None, None, env)
