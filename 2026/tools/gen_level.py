"""Extends FREEZ level 1.

The shipped level 1 has no end flag, and its lower-left hall (cols 6-71, below the big diagonal mass) was left
unfinished: it has no floor, so falling into it never ends.  This script builds it out:

  1. HALL   - a floor at row 37 continuing left from the existing platform (cols 6-71), with pits, spikes,
              blocks, enemies and pickups, ending at a shaft.
  2. LANE   - a long lower corridor (floor row 57) reached by the shaft, running left->right to the end flag.

It reads the original tmap1/emap1/bmap1 from ../../assets and writes the extended maps into ../assets.
"""
import struct, os, sys
HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, '..', '..', 'assets')
DST = os.path.join(HERE, '..', 'assets')
W = 128

tm = bytearray(open(os.path.join(SRC, 'tmap1.map'), 'rb').read())
en = bytearray(open(os.path.join(SRC, 'emap1.map'), 'rb').read())
bn = bytearray(open(os.path.join(SRC, 'bmap1.map'), 'rb').read())

# ---- tile ids (see the tile sheet)
FLOOR, DIRT = 126, 104          # grass-topped opaque tile / dirt fill
CORNER_L, CORNER_R = 124, 125   # grass top turning down at the left / right end
FACE_L_TOP, FACE_L, FACE_R_TOP, FACE_R = 1, 2, 3, 4   # transparent block side faces
SPIKES, SPRING, ISLAND = 55, 59, 17
DECOR = [53, 54, 56, 57, 58]
FLAG_RESTART_OFF, FLAG_END = 60, 62

def put(c, r, t):
    if 0 <= c < W and 0 <= r < 64:
        tm[r * W + c] = t

def fill(c0, c1, r0, r1, t):
    for r in range(r0, r1 + 1):
        for c in range(c0, c1 + 1):
            put(c, r, t)

rng_state = 12345
def rnd(n):
    global rng_state
    rng_state = (rng_state * 1103515245 + 12345) & 0x7FFFFFFF
    return (rng_state >> 8) % n

# ---- enemies / bonuses
next_enemy = 68                 # records 0-67 are the original level
next_bonus = 119
def enemy(ctrl, x, y, typ=0, frame=0, maxd=0, spd=0):
    global next_enemy
    assert next_enemy < 200, 'out of enemy slots'
    rec = struct.pack('<BBBhhBBBBbB', ctrl, 0, 0, int(x), int(y), typ, frame, 0, maxd, spd, 0)
    en[next_enemy * 13:(next_enemy + 1) * 13] = rec
    next_enemy += 1

def bonus(t, x, y):
    global next_bonus
    assert next_bonus < 400
    bn[next_bonus * 5:(next_bonus + 1) * 5] = struct.pack('<Bhh', t, int(x), int(y))
    next_bonus += 1

def ground(c, F, patrol=96, spd=1):      # walking enemy standing on floor row F
    enemy(1, c * 32, (F - 1) * 32, 1, rnd(32), patrol, spd)
def flyer(c, r, patrol=128, spd=1):
    enemy(2, c * 32, r * 32, 0, rnd(32), patrol, spd)
def acid(c, F):                          # invisible deadly spot on floor row F (spike tile drawn separately)
    enemy(3, c * 32 + 4, (F - 1) * 32 + 17)
def spring(c, F, power=24):
    put(c, F - 1, SPRING)
    enemy(4, c * 32 + 1, (F - 1) * 32 + 16, 0, 0, power)
def platform(x, y):                      # circling platform
    enemy(10, x, y, 17)
def coins(c0, r, n, gap=24, arc=0):      # row of coins, optionally arched
    for i in range(n):
        dy = 0
        if arc:
            m = (n - 1) / 2.0
            dy = -int(arc * (1 - ((i - m) / m) ** 2)) if m else 0
        bonus(2, c0 * 32 + i * gap, r * 32 + dy)
def gems(c0, r, n, gap=28):
    for i in range(n):
        bonus(1, c0 * 32 + i * gap, r * 32)

# restart flags first: the engine only resets the first 100 enemy records when a new flag is hit
enemy(7, 33 * 32, (37 - 1) * 32, FLAG_RESTART_OFF, 1)     # hall (face right on restart)
enemy(7, 66 * 32, (57 - 1) * 32, FLAG_RESTART_OFF, 1)     # lane, roughly halfway

# ---- terrain helpers
def decor(c0, c1, F, density=3):
    for c in range(c0, c1 + 1):
        if rnd(density) == 0 and tm[(F - 1) * W + c] == 0:
            put(c, F - 1, DECOR[rnd(len(DECOR))])

def floor(c0, c1, F, bottom):
    for c in range(c0, c1 + 1):
        put(c, F, FLOOR)
        for r in range(F + 1, bottom + 1):
            put(c, r, DIRT)

def block(c0, c1, F, h, bottom):
    """raised block h tiles above floor row F"""
    T = F - h
    for c in range(c0, c1 + 1):
        put(c, T, FLOOR)
        for r in range(T + 1, F):
            put(c, r, DIRT)
    put(c0 - 1, T, FACE_L_TOP); put(c1 + 1, T, FACE_R_TOP)
    for r in range(T + 1, F):
        put(c0 - 1, r, FACE_L); put(c1 + 1, r, FACE_R)

def pit(c0, c1, F, bottom, with_platform=True, cross_row=None):
    """gap in floor F with acid at the bottom; platforms carry you across"""
    for c in range(c0, c1 + 1):
        for r in range(F, bottom + 1):
            put(c, r, 0)
        put(c, F + 2, FLOOR)
        put(c, F + 1, SPIKES)
        for r in range(F + 3, bottom + 1):
            put(c, r, DIRT)
        acid(c, F + 2)
    if with_platform:
        n = (c1 - c0 + 1)
        cx = (c0 + c1 + 1) * 16
        platform(cx, F * 32 - 24)

def spikes(c0, c1, F):
    for c in range(c0, c1 + 1):
        put(c, F - 1, SPIKES)
        acid(c, F)

# ------------------------------------------------------------------------------------
# clear the build zones
# ------------------------------------------------------------------------------------
HALL_F = 37
fill(6, 71, 33, 62, 0)                 # the unfinished hall + everything below it
fill(72, 126, 43, 62, 0)               # the leftover ledge under the original level
for r in range(33, 64):
    put(5, r, FACE_R if r > 33 else FACE_R_TOP)   # keep the left wall face
fill(0, 4, 33, 63, DIRT)

# ====================================================================================
# 1. HALL (floor row 37, walked right -> left, cols 71 .. 6)
# ====================================================================================
BOT_H = 40                              # hall floor thickness down to row 39 (lane ceiling)
floor(6, 71, HALL_F, BOT_H)
# seal the hall's right edge to the original block (cols 72-79 rows 37-42 are already solid)
put(71, HALL_F, FLOOR)

decor(8, 70, HALL_F, 4)
# h1: warm-up
ground(64, HALL_F, 96, 1)
coins(66, 33, 5, 26, 20)
# h2: first pit with a platform  (cols 56-59)
pit(56, 59, HALL_F, BOT_H)
bonus(2, 57 * 32 + 8, 34 * 32); bonus(2, 58 * 32 + 8, 34 * 32)
# h3: floor with spikes
ground(51, HALL_F, 96, 1)
spikes(47, 48, HALL_F)
coins(46, 33, 4, 26, 26)
# h4: staircase of blocks
block(41, 44, HALL_F, 1, BOT_H)
block(36, 39, HALL_F, 2, BOT_H)
gems(36, 33 - 1, 3)
ground(42, HALL_F - 1, 64, 1)
flyer(38, 33, 128, 1)
# h5: restart flag + rest
decor(29, 34, HALL_F, 2)
bonus(3, 31 * 32, 35 * 32)                                     # energy
# h6: second pit, wider
pit(23, 27, HALL_F, BOT_H)
platform(25 * 32 + 16, HALL_F * 32 - 40)
coins(23, 34, 5, 30, 34)
# h7: spikes + flyers
ground(20, HALL_F, 96, 2)
flyer(17, 34, 96, 1)
spikes(16, 17, HALL_F)
# h8: the descent (shaft at cols 7-8)
ground(13, HALL_F, 64, 1)
for r in range(HALL_F, BOT_H + 1):
    put(7, r, 0); put(8, r, 0)
put(9, HALL_F, CORNER_R); put(6, HALL_F, CORNER_L)
for r in range(HALL_F + 1, BOT_H + 1):
    put(9, r, DIRT)
bonus(7, 11 * 32, 35 * 32)                                     # time bonus before the drop
for i in range(4):
    bonus(2, 7 * 32 + 20, (39 + 3 * i) * 32)

# ====================================================================================
# 2. LANE (floor row 57, walked left -> right, cols 6 .. 126)
# ====================================================================================
LF = 57
LB = 63
floor(6, 126, LF, LB)
decor(8, 125, LF, 5)
# landing area
coins(10, 55, 6, 26, 0)
ground(14, LF, 64, 1)
# l1: pit with platform
pit(18, 21, LF, LB)
coins(18, 54, 4, 32, 30)
# l2: ground baddies + spring shelf
ground(25, LF, 96, 2); ground(30, LF, 64, 1)
spring(34, LF, 24)
block(36, 41, LF, 2, LB)
block(43, 46, LF, 4, LB)           # a tall block: reach it with the spring from the left
gems(43, 52, 4)
bonus(6, 44 * 32 + 8, 52 * 32 - 16)    # rapid fire on top
# l3: flyers over a flat run
flyer(50, 52, 128, 1); flyer(56, 54, 128, 2)
ground(52, LF, 128, 2)
coins(50, 54, 8, 28, 20)
# l4: double pit
pit(62, 64, LF, LB)
pit(69, 72, LF, LB)
coins(62, 54, 12, 28, 26)
bonus(3, 67 * 32, 55 * 32)
# l5: spikes corridor
spikes(76, 77, LF); spikes(81, 83, LF)
ground(79, LF, 32, 1)
flyer(80, 53, 96, 1)
coins(75, 54, 9, 26, 20)
# l6: spring + high shelf with extra life
spring(87, LF, 28)
put(90, LF - 6, FLOOR); put(91, LF - 6, FLOOR); put(92, LF - 6, FLOOR); put(93, LF - 6, FLOOR)
put(89, LF - 6, FACE_L_TOP); put(94, LF - 6, FACE_R_TOP)
bonus(8, 91 * 32, (LF - 8) * 32)
gems(90, LF - 8, 3, 28)
# l7: gauntlet
ground(98, LF, 96, 1); ground(103, LF, 64, 2)
block(100, 102, LF, 1, LB)
pit(106, 108, LF, LB)
flyer(100, 52, 128, 2); flyer(110, 53, 96, 1)
coins(104, 54, 6, 28, 22)
# l8: finish: long flat, last time bonus and the end flag
bonus(7, 114 * 32, 55 * 32)
bonus(3, 116 * 32, 55 * 32)
ground(113, LF, 64, 1)
coins(117, 55, 3, 26, 0)
enemy(8, 123 * 32, (LF - 1) * 32, FLAG_END, 1)             # end of level flag

# ---- wall behind the end flag stays the original column 127
for r in range(43, 64):
    put(127, r, 2)

# restart flags must live in the first 100 enemy records (the original clears only those)
# -> they were appended after 68, which is < 100: ok
assert next_enemy <= 200

open(os.path.join(DST, 'tmap1.map'), 'wb').write(tm)
open(os.path.join(DST, 'emap1.map'), 'wb').write(en)
open(os.path.join(DST, 'bmap1.map'), 'wb').write(bn)
print('enemies used', next_enemy, 'bonuses used', next_bonus)

# level record: more time (the level is much longer now)
ld = bytearray(open(os.path.join(SRC, 'ldata.dat'), 'rb').read())
struct.pack_into('<H', ld, 468 + 8, 42000)
open(os.path.join(DST, 'ldata.dat'), 'wb').write(ld)
