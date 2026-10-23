"""Generates mobile/assets/intro.tga: the "DIGITAL SYNAPSE presents" title card.

Output is a 320x200 8-bit colour-mapped TGA in exactly the layout the game's LoadTitle() reads
(18 byte header, 256 BGR palette at 0x12, raw top-down pixels at 0x312).  The palette is sorted by brightness
because the title sequence first tunes the picture in through a grey ramp (grey = index / 4) before fading to colour.

Needs Pillow and the Windows fonts Arial Black + Georgia Italic (the generated .tga is committed, so you only need
this to change the artwork).
"""
import math, os, random
from PIL import Image, ImageDraw, ImageFilter, ImageFont, ImageChops

HERE = os.path.dirname(os.path.abspath(__file__))
ASSETS = os.path.join(HERE, '..', 'assets')
FONTS = 'C:/Windows/Fonts/'
W, H, S = 320, 200, 3                  # final size, supersample factor
w, h = W * S, H * S
random.seed(7)

def lerp(a, b, t): return tuple(int(a[i] + (b[i] - a[i]) * t) for i in range(3))

# ---- background: deep blue gradient with a soft glow behind the text
bg = Image.new('RGB', (w, h))
px = bg.load()
for y in range(h):
    for x in range(w):
        t = y / h
        c = lerp((3, 5, 22), (10, 18, 54), t)
        d = math.hypot((x - w * .5) / (w * .55), (y - h * .47) / (h * .5))
        g = max(0.0, 1 - d) ** 2
        px[x, y] = (int(c[0] + 10 * g), int(c[1] + 40 * g), int(c[2] + 80 * g))

# ---- synapse network: nodes joined by curved links with bright "impulses"
net = Image.new('RGB', (w, h), (0, 0, 0))
nd = ImageDraw.Draw(net)
nodes = []
for _ in range(46):
    x, y = random.randint(int(w * .04), int(w * .96)), random.randint(int(h * .06), int(h * .94))
    if abs(x - w * .5) < w * .30 and h * .10 < y < h * .88:       # keep the middle clearer for the lettering
        continue
    nodes.append((x, y))
for i, (x1, y1) in enumerate(nodes):
    near = sorted(nodes, key=lambda n: (n[0] - x1) ** 2 + (n[1] - y1) ** 2)[1:3]
    for (x2, y2) in near:
        mx, my = (x1 + x2) / 2 + random.randint(-40, 40) * S / 2, (y1 + y2) / 2 + random.randint(-40, 40) * S / 2
        pts = [((1 - t) ** 2 * x1 + 2 * (1 - t) * t * mx + t * t * x2, (1 - t) ** 2 * y1 + 2 * (1 - t) * t * my + t * t * y2) for t in [k / 24 for k in range(25)]]
        nd.line(pts, fill=(40, 120, 190), width=S)
        k = random.randint(5, 18)                                    # an impulse travelling along the link
        nd.ellipse([pts[k][0] - 2 * S, pts[k][1] - 2 * S, pts[k][0] + 2 * S, pts[k][1] + 2 * S], fill=(190, 240, 255))
for (x, y) in nodes:
    nd.ellipse([x - 4 * S, y - 4 * S, x + 4 * S, y + 4 * S], fill=(60, 170, 230))
    nd.ellipse([x - 2 * S, y - 2 * S, x + 2 * S, y + 2 * S], fill=(220, 250, 255))
glow = net.filter(ImageFilter.GaussianBlur(5 * S))
bg = ImageChops.add(bg, ImageChops.add(glow, net.filter(ImageFilter.GaussianBlur(S // 2))))

# ---- lettering
def fit(text, path, size):
    f = ImageFont.truetype(path, size)
    return f

def chrome_text(img, text, cy, font, spread=0):
    """Chrome-filled text centred at cy with outline, shadow and glow."""
    d = ImageDraw.Draw(img)
    bbox = d.textbbox((0, 0), text, font=font)
    tw, th = bbox[2] - bbox[0], bbox[3] - bbox[1]
    x0, y0 = (w - tw) // 2 - bbox[0], int(cy - th / 2) - bbox[1]
    mask = Image.new('L', (w, h), 0)
    ImageDraw.Draw(mask).text((x0, y0), text, font=font, fill=255)
    # glow
    gl = Image.new('RGB', (w, h), (0, 0, 0)); gl.paste((50, 150, 255), mask=mask.filter(ImageFilter.GaussianBlur(7 * S)))
    img.paste(ImageChops.add(img, gl))
    # drop shadow
    sh = ImageChops.offset(mask, 2 * S, 3 * S).filter(ImageFilter.GaussianBlur(S))
    img.paste((0, 0, 8), mask=sh.point(lambda v: v * 3 // 4))
    # outline
    outline = mask.filter(ImageFilter.MaxFilter(2 * S + 1))
    img.paste((8, 24, 64), mask=outline)
    # chrome gradient fill
    top, bot = y0 + bbox[1], y0 + bbox[3]
    fill = Image.new('RGB', (w, h))
    fp = fill.load()
    for y in range(max(0, top - 2), min(h, bot + 2)):
        t = (y - top) / max(1, bot - top)
        if t < .5: c = lerp((245, 250, 255), (120, 175, 235), t / .5)
        elif t < .56: c = (20, 40, 90)                             # the dark horizon line of "chrome"
        else: c = lerp((70, 130, 210), (190, 230, 255), (t - .56) / .44)
        for x in range(w): fp[x, y] = c
    img.paste(fill, mask=mask)
    # top highlight
    hl = ImageChops.subtract(mask, ImageChops.offset(mask, 0, S * 2))
    img.paste((255, 255, 255), mask=hl.point(lambda v: v // 2))

title = ImageFont.truetype(FONTS + 'ariblk.ttf', 38 * S)
chrome_text(bg, 'DIGITAL', 62 * S, title)
chrome_text(bg, 'SYNAPSE', 104 * S, title)

# "presents": italic serif, soft light blue with a faint glow, flanked by thin rules
pf = ImageFont.truetype(FONTS + 'georgiai.ttf', 17 * S)
d = ImageDraw.Draw(bg)
bb = d.textbbox((0, 0), 'presents', font=pf)
tx, ty = (w - (bb[2] - bb[0])) // 2 - bb[0], 152 * S - bb[1]
m = Image.new('L', (w, h), 0); ImageDraw.Draw(m).text((tx, ty), 'presents', font=pf, fill=255)
g2 = Image.new('RGB', (w, h), (0, 0, 0)); g2.paste((70, 160, 255), mask=m.filter(ImageFilter.GaussianBlur(4 * S)))
bg = ImageChops.add(bg, g2)
d = ImageDraw.Draw(bg)
d.text((tx + S, ty + S), 'presents', font=pf, fill=(0, 4, 20))
d.text((tx, ty), 'presents', font=pf, fill=(200, 226, 255))
ry = 152 * S + (bb[3] - bb[1]) // 2
for side in (-1, 1):
    x1 = w // 2 + side * ((bb[2] - bb[0]) // 2 + 14 * S)
    x2 = w // 2 + side * ((bb[2] - bb[0]) // 2 + 70 * S)
    for i in range(S * 2):
        a = 1 - i / (S * 2)
        d.line([(x1, ry + i - S), (x2, ry + i - S)], fill=(int(60 + 130 * a), int(120 + 110 * a), int(200 + 55 * a)), width=1)

# vignette
vg = Image.new('L', (w, h), 255); vp = vg.load()
for y in range(h):
    for x in range(w):
        dd = math.hypot((x - w / 2) / (w * .62), (y - h / 2) / (h * .66))
        vp[x, y] = int(255 * (1 - min(1, max(0, dd - .55)) ** 1.5 * .65))
bg = ImageChops.multiply(bg, Image.merge('RGB', (vg, vg, vg)))

# ---- down-sample, quantise, sort the palette by brightness
small = bg.resize((W, H), Image.LANCZOS)
q = small.quantize(256, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.FLOYDSTEINBERG)
pal = q.getpalette()[:768]
pal += [0] * (768 - len(pal))
lum = lambda i: 0.299 * pal[i * 3] + 0.587 * pal[i * 3 + 1] + 0.114 * pal[i * 3 + 2]
order = sorted(range(256), key=lum)
remap = {old: new for new, old in enumerate(order)}
data = bytes(remap[v] for v in q.tobytes())
new_pal = bytearray()
for old in order:
    r, g, b = pal[old * 3:old * 3 + 3]
    new_pal += bytes((b, g, r))                                   # TGA palette is BGR

orig = open(os.path.join(ASSETS, 'freez.tga'), 'rb').read()
header = orig[:18]
open(os.path.join(ASSETS, 'intro.tga'), 'wb').write(header + bytes(new_pal) + data)
small.save(os.path.join(HERE, 'intro_preview.png'))
print('wrote intro.tga', len(header) + len(new_pal) + len(data), 'bytes')
