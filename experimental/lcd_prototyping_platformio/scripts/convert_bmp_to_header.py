import os

lut = [(0x00, 0x00, 0x00), (0xFF, 0xFF, 0xFF), (0x80, 0x80, 0x80), (0x58, 0x7D, 0x61),
           (0x55, 0x71, 0x70), (0xFF, 0x00, 0x00), (0x00, 0xFF, 0x00), (0x00, 0x00, 0xFF),
           (0xFF, 0xFF, 0x00), (0x00, 0xFF, 0xFF), (0xFF, 0x00, 0xFF), (0x00, 0x00, 0x00),
           (0x00, 0x00, 0x00), (0x00, 0x00, 0x00), (0x00, 0x00, 0x00), (0x00, 0x00, 0x00)]

def bmp_to_header(img_dir, filename):
    file_stem = filename.split('.')[0]

    data = []

    file = open(f"{img_dir}/{filename}", 'rb')

    # First get the starting offset from the header
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

    return data

def data_reorg(data):
    width = 800
    height = 480

    out_data = []

    for x in range(width):
        for y in range(height):
            out_data.append(data[y*width + x])

    return out_data

def data_argb_to_al44(data, lut):
    # Convert data from ARGB8888 to AL44 (4bit alpha, 4bit LUT)
    width = 800
    height = 480

    out_data = []

    for pix in data:
        a = (pix >> 24) & 0xFF
        r = (pix >> 16) & 0xFF
        g = (pix >> 8) & 0xFF
        b = pix & 0xFF

        # Find nearest color in LUT
        best_index = 0
        best_distance = 999999
        for i in range(len(lut)):
            lr, lg, lb = lut[i]
            distance = (r - lr) ** 2 + (g - lg) ** 2 + (b - lb) ** 2
            if distance < best_distance:
                best_distance = distance
                best_index = i

        l4 = best_index & 0x0F
        a4 = (a >> 4) & 0x0F

        out_data.append((a4 << 4) | l4)

    return out_data

def data_to_header(header_dir, filename, data):
    file_stem = filename.split('.')[0]

    # header = f"#ifndef {file_stem.upper()}_H\n#define {file_stem.upper()}_H\n\n#include <stdint.h>\nconst uint32_t {file_stem.upper()}[] = {{\n"
    header = f"#ifndef {file_stem.upper()}_H\n#define {file_stem.upper()}_H\n\n#include <stdint.h>\nconst uint8_t __attribute__((section(\".ext_rodata\"))) {file_stem.upper()}[] = {{\n"

    line_width = 8
    line_index = 0
    for pix in data:
        header += f"0x{pix:02x}, "
        line_index += 1
        if line_index == line_width:
            header += '\n'
            line_index = 0

    header += f"}};\n\n#endif // {file_stem.upper()}_H"

    # Save to a header with the same stem
    header_file = open(f"{header_dir}/{file_stem}.h", "w")
    header_file.write(header)
    header_file.close()

def lut_to_header(header_dir, lut):
    header = f"#ifndef LUT_H\n#define LUT_H\n\n#include <stdint.h>\nconst uint32_t __attribute__((section(\".ext_rodata\"))) LUT[] = {{\n"

    line_width = 4
    line_index = 0
    for r, g, b in lut:
        header += f"0x00{r:02x}{g:02x}{b:02x}, "
        line_index += 1
        if line_index == line_width:
            header += '\n'
            line_index = 0

    header += f"}};\n\n#endif // LUT_H"

    # Save to lut.h
    header_file = open(f"{header_dir}/lut.h", "w")
    header_file.write(header)
    header_file.close()

print("Converting BMPs to header data")

if not os.path.exists("images"):
    print("Images folder does not exist, creating...")
    os.mkdir("images")

if not os.path.exists("include/images"):
    print("Images header folder does not exist, creating...")
    os.mkdir("include/images")

for path in os.listdir("images"):
    print(path)
    data = bmp_to_header("images", path)
    data = data_reorg(data)
    data = data_argb_to_al44(data, lut)
    data_to_header("include/images", path, data)

print("Writing LUT header")
lut_to_header("include/images", lut)