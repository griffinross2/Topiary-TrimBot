import cairo
from fontTools.ttLib import TTFont
from fontTools.pens.cairoPen import CairoPen
from fontTools.pens.boundsPen import BoundsPen
from PIL import Image, ImageTk
import tkinter as tk

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

    glyph_name = cmap[ord(char)]
    glyph = glyph_set[glyph_name]
    advance, lsb = hmtx[glyph_name]

    scale = size / units

    bounds_pen = BoundsPen(glyph_set)
    glyph.draw(bounds_pen)

    if bounds_pen.bounds is None:
        return None, 0, 0, 0, 0, advance*scale

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

    return surface, bounds_pen.bounds[0]*scale, bounds_pen.bounds[2]*scale, bounds_pen.bounds[1]*scale, bounds_pen.bounds[3]*scale, advance*scale

def save_char(char, pt_size=128):
    surface, xMin, xMax, yMin, yMax, advance = draw_glyph_cairo(cairo.Context(cairo.ImageSurface(cairo.FORMAT_ARGB32, 1, 1)), "fonts/arial.ttf", char, 0, 0, pt_size)
    # surface.write_to_png(f"glyphs/{char}_{'upper' if char.isupper() else 'lower'}.png")
    if surface is not None:
        glyphs[f'{char}'] = {'data': surface.get_data().tobytes(), 'width': pt_size, 'height': pt_size, 'xMin': xMin, 'xMax': xMax, 'yMin': yMin, 'yMax': yMax, 'advance': advance}
    else:
        glyphs[f'{char}'] = {'data': None, 'width': pt_size, 'height': pt_size, 'xMin': xMin, 'xMax': xMax, 'yMin': yMin, 'yMax': yMax, 'advance': advance}


def create_canvas(width, height):
    root = tk.Tk()
    canvas = tk.Canvas(root, width=width, height=height)
    canvas.pack()
    return root, canvas

for char in [bytes([code]).decode('ascii') for code in range(256)]:
    save_char(char, pt_size=256)

text_x, text_y = 50, 200
width, height = 800, 480
root, canvas = create_canvas(width, height)
canvas.create_line(text_x - 10, text_y, text_x + 10, text_y, fill="red")
canvas.create_line(text_x, text_y - 10, text_x, text_y + 10, fill="red")

ctx = {'current_pos': (text_x, text_y), 'canvas': canvas, 'pt_size': 64}

print(len(glyphs['A']['data']))

def draw_char_from_glyphs(char, msaa=4):
    glyph = glyphs[char]

    if glyph['data'] is None:
        ctx['current_pos'] = (ctx['current_pos'][0] + glyph['advance'] * ctx['pt_size'] / glyph['width'], ctx['current_pos'][1])
        return

    for x in range(ctx['pt_size']):
        for y in range(ctx['pt_size']):
            total_pixel = 0
            for mx in range(msaa):
                for my in range(msaa):
                    # Determine pixel coords according to rescaling
                    px = int(((x + mx/msaa) * glyph['width']) / ctx['pt_size'])
                    py = int(((y + my/msaa) * glyph['height']) / ctx['pt_size'])

                    offset = (py * glyph['width'] + px)*4
                    b = glyph['data'][offset]
                    g = glyph['data'][offset + 1]
                    r = glyph['data'][offset + 2]
                    a = glyph['data'][offset + 3]
                    if a > 0:
                        total_pixel += 255

            total_pixel = total_pixel // (msaa*msaa)
            canvas_x = int(ctx['current_pos'][0] + x)
            canvas_y = int(ctx['current_pos'][1] + (y - ctx['pt_size']))
            gray = 255 - total_pixel
            canvas.create_rectangle(canvas_x, canvas_y, canvas_x, canvas_y, fill=f"#{gray:02x}{gray:02x}{gray:02x}", outline="")
    ctx['current_pos'] = (ctx['current_pos'][0] + glyph['advance'] * ctx['pt_size'] / glyph['width'], ctx['current_pos'][1])

def draw_string_from_glyphs(string, msaa=4):
    for char in string:
        if char in glyphs:
            draw_char_from_glyphs(char, msaa=msaa)

draw_string_from_glyphs("Hello, World!")
ctx['current_pos'] = (text_x, text_y+ctx['pt_size']+10)
draw_string_from_glyphs("Hello, World!", msaa=1)

root.mainloop()