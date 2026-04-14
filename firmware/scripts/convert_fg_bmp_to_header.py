Import("env")

import os

def bmp_to_header(img_dir, filename):
    file_stem = filename.split('.')[0]

    data = []

    file = open(f"{img_dir}/{filename}", 'rb')

    # First get the width and height
    file.seek(18)
    width = int.from_bytes(file.read(4), 'little')
    height = int.from_bytes(file.read(4), 'little')

    # Then get the starting offset from the header
    file.seek(10)
    start = int.from_bytes(file.read(4), 'little')
    file.seek(start)

    # Now get the data until the end of the file
    while True:
        pixel_bytes = file.read(4)
        if len(pixel_bytes) != 4:
            break

        # a = int(pixel_bytes[0])
        a = 0xFF
        # a = 0x0
        b = int(pixel_bytes[1])
        g = int(pixel_bytes[2])
        r = int(pixel_bytes[3])

        data.append((a << 24) | (r << 16) | (g << 8) | b)

    file.close()

    return data, width, height

def data_reorg(data, width, height):

    int_data = []

    for x in range(width):
        for y in range(height):
            int_data.append(data[x*height + y])

    out_data = []

    for y in range(height):
        for x in range(width):
            out_data.append(data[(width-x-1) + y*width])

    return out_data

def data_to_header(header_dir, filename, data, width, height):
    file_stem = filename.split('.')[0]

    header = f"#ifndef {file_stem.upper()}_H\n#define {file_stem.upper()}_H\n\n"

    header += f"#include \"graphics.h\"\n\n"
    
    header += f"const uint8_t __attribute__((section(\".ext_rodata\"))) {file_stem.upper()}_DATA[] = {{\n"

    line_width = 8
    line_index = 0
    for pix in data:
        bw = 0
        b = pix & 0xFF
        g = (pix >> 8) & 0xFF
        r = (pix >> 16) & 0xFF

        if ((r**2 + g**2 + b**2)**0.5) > 127:
            bw = 0x00
        else:
            bw = 0xF1

        header += f"0x{bw:02x}, "
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
    print("Converting BMPs to header data")

    if not os.path.exists("graphics"):
        print("Graphics folder does not exist, creating...")
        os.mkdir("graphics")

    if not os.path.exists("include/graphics"):
        print("Graphics header folder does not exist, creating...")
        os.mkdir("include/graphics")

    for path in os.listdir("graphics"):
        print(path)
        data, width, height = bmp_to_header("graphics", path)
        data = data_reorg(data, width, height)
        data_to_header("include/graphics", path, data, width, height)

# env.AddPreAction("buildprog", before_build)
before_build(None, None, env)
