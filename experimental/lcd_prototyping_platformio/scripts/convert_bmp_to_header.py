import os

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

def data_to_header(header_dir, filename, data):
    file_stem = filename.split('.')[0]

    header = f"const uint32_t {file_stem.upper()}[] = {{\n"

    line_width = 4
    line_index = 0
    for pix in data:
        header += f"0x{pix:08x}, "
        line_index += 1
        if line_index == line_width:
            header += '\n'
            line_index = 0
    header += "};"

    # Save to a header with the same stem
    header_file = open(f"{header_dir}/{file_stem}.h", "w")
    header_file.write(header)
    header_file.close()


print("Converting BMPs to header data")
for path in os.listdir('images'):
    print(path)
    data = bmp_to_header('images', path)
    data = data_reorg(data)
    data_to_header('include/images', path, data)
        