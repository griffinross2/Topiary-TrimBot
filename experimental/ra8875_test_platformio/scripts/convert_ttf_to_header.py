import cairo
from fontTools.ttLib import TTFont
from fontTools.pens.cairoPen import CairoPen
from fontTools.pens.boundsPen import BoundsPen
import os

glyphs = {}

def draw_glyph_cairo(ctx, font_path, char, x, y, size):
    """
    Draw a filled glyph at (x, y) using Cairo.
    (x, y) is the baseline position.
    """
    font = TTFont(font_path)
    glyph_set = font.getGlyphSet()
    cmap = font.getBestCmap()
    hmtx = font['hmtx']
    units = font["head"].unitsPerEm
    descent = font["hhea"].descent

    try:
        glyph_name = cmap[ord(char)]
    except KeyError:
        return None, '', 0, 0, 0, 0, 0
    
    glyph = glyph_set[glyph_name]
    advance, lsb = hmtx[glyph_name]

    scale = size / units

    bounds_pen = BoundsPen(glyph_set)
    glyph.draw(bounds_pen)

    if bounds_pen.bounds is None:
        return None, glyph_name, 0, 0, 0, 0, advance*scale

    w, h = bounds_pen.bounds[2] - bounds_pen.bounds[0], bounds_pen.bounds[3] - bounds_pen.bounds[1]

    # Create a Cairo surface and context
    surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, size, size)
    ctx = cairo.Context(surface)
    ctx.set_source_rgb(0, 0, 0)  # Black background

    ctx.save()
    ctx.scale(scale, -scale)     # scale and flip Y
    ctx.translate(0, -units-descent)

    pen = CairoPen(glyph_set, ctx)
    glyph.draw(pen)

    ctx.restore()
    ctx.fill()

    return surface, glyph_name, bounds_pen.bounds[0]*scale, bounds_pen.bounds[2]*scale, bounds_pen.bounds[1]*scale, bounds_pen.bounds[3]*scale, advance*scale

def save_char(font_path, char, pt_size=128):
    surface, glyph_name, xMin, xMax, yMin, yMax, advance = draw_glyph_cairo(cairo.Context(cairo.ImageSurface(cairo.FORMAT_ARGB32, 1, 1)), font_path, char, 0, 0, pt_size)
    # surface.write_to_png(f"glyphs/{char}_{'upper' if char.isupper() else 'lower'}.png")
    if surface is not None:
        glyphs[f'{char}'] = {'data': surface.get_data().tobytes(), 'name': glyph_name, 'width': pt_size, 'height': pt_size, 'xMin': xMin, 'xMax': xMax, 'yMin': yMin, 'yMax': yMax, 'advance': advance}
    else:
        glyphs[f'{char}'] = {'data': None, 'name': glyph_name, 'width': pt_size, 'height': pt_size, 'xMin': xMin, 'xMax': xMax, 'yMin': yMin, 'yMax': yMax, 'advance': advance}


def header_start(font_name: str):
    header = f'#ifndef {font_name.upper()}_H\n#define {font_name.upper()}_H\n\n#include "font.h"\n\n'
    return header

def header_end(header, font_name: str):
    header += f'\n#endif // {font_name.upper()}_H'
    return header

def header_pixels_to_array(header, glyph):
    width = glyph['width']
    height = glyph['height']
    array = [0]*((width*height+1)//8)
    for x in range(width):
        for y in range(height):
            offset_img = (y * glyph['width'] + x)*4
            alpha = glyph['data'][offset_img + 3]
            offset_array_pixel = (x * glyph['height'] + y)
            offset_array_byte = offset_array_pixel//8
            offset_array_bit = offset_array_pixel%8
            if alpha > 0:
                array[offset_array_byte] |= (0x1 << offset_array_bit)

    byte_array = bytes(array)
    for by in byte_array:
        header += f'0x{by:02x},  '

    return header

def header_char(header, char):
    glyph_name = glyphs[char]['name']
    advance = glyphs[char]['advance']
    if glyph_name != '':
        glyph_struct_name = f'char_{glyph_name}'
        if glyphs[char]['data'] is not None:
            glyph_data_name = f'{glyph_struct_name}_data'
            header += f'const uint8_t {glyph_data_name}[] = {{'
            header = header_pixels_to_array(header, glyphs[char])
            header += f'}};\n\n'
            header += f'const Glyph {glyph_struct_name} = {{\n    .advance = {int(advance)},\n    .data = {glyph_data_name},\n}};\n\n'
        else:
            header += f'const Glyph {glyph_struct_name} = {{\n    .advance = {int(advance)},\n    .data = (uint8_t*)0,\n}};\n\n'
    else:
        hex = char.encode('ascii')[0]
        glyph_struct_name = f'char_codepoint_0x{hex:02x}'
        header += f'const Glyph {glyph_struct_name} = {{\n    .advance = {int(advance)},\n    .data = (uint8_t*)0,\n}};\n\n'
    return header, glyph_struct_name

def header_font_manifest(header, pt_size, glyph_struct_names, font_name: str):
    header += f'const Font {font_name.upper()} = {{\n    .width = {pt_size},\n    .height = {pt_size},\n    .glyphs = {{\n'
    for name in glyph_struct_names:
        header += f'        &{name},\n'
    header += '    },\n};\n'
    return header
    
def write_header(header, font_name: str):
    # Make directory if nonexistent
    if not os.path.isdir('include/fonts'):
        os.mkdir('include/fonts')
    
    with open(f'include/fonts/{font_name.lower()}.h', 'w') as hf:
        hf.write(header)

print("Converting TTF fonts to bitmap headers")

if not os.path.isdir("fonts"):
    print("No fonts directory, creating...")
    try:
        os.mkdir("fonts")
    except OSError:
        quit()

for font_file in os.listdir('fonts'):
    font_name = font_file.split('.')[0].lower()
    print(f'Converting {font_file}...')

    pt_size = 128
    header = header_start(font_name)
    glyph_structs = []
    for char in [bytes([code]).decode('ascii') for code in range(128)]:
        save_char(f'fonts/{font_file}', char, pt_size=pt_size)
        header, glyph_struct_name = header_char(header, char)
        glyph_structs.append(glyph_struct_name)
    header = header_font_manifest(header, pt_size, glyph_structs, font_name)
    header = header_end(header, font_name)
    write_header(header, font_name)

    print(f'Finished converting {font_file}!')