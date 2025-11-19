import os

def bmp_to_header(filename):
    file_stem = filename.split('.')[0]
    file_stem = filename.split('/')[-1]

    header = f"const uint8_t {file_stem.upper()}[] = {{\n"

    file = open(filename, 'rb')

    # First get the starting offset from the header
    file.seek(10)
    int.from_bytes(file.read(4), 'little')

    # Now get the data until the end of the file
    while True:
        pixel_bytes = file.read(4)
        if len(pixel_bytes) != 4:
            break

        x = int(pixel_bytes[0])
        b = int(pixel_bytes[1])
        g = int(pixel_bytes[2])
        r = int(pixel_bytes[3])
