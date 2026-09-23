#!/usr/bin/env python3
"""
sprite_gen.py - generates sketch/sprite.h and figures/sprite_frames.png

The four-frame dancing sprite that animates on the OLED now-playing screen.

At 24x24 there is no room for anti-aliasing or clever drawing primitives -
a limb rendered by a line-drawing routine merges into the torso and the
figure reads as a blob. The frames are therefore authored as explicit pixel
art, one character per pixel, and this script only validates and packs them.

Packing follows what Adafruit_GFX::drawBitmap() expects:
    byte b of row r holds columns 8b..8b+7, most-significant bit leftmost.
    24 px wide  ->  3 bytes/row  ->  72 bytes/frame.

Frame D is the horizontal mirror of frame B, so the dance is symmetric.

Usage:  python3 tools/sprite_gen.py
"""

import os
from PIL import Image

W = H = 24

# --------------------------------------------------------------------------
#  Frame A - feet together, arms hanging. The "rest" pose, on the downbeat.
# --------------------------------------------------------------------------
FRAME_A = [
    "........................",
    ".......##########.......",   # headphone band
    "........########........",
    "......############......",   # ear cups + head
    "......####.##.####......",   # eyes
    "......####.##.####......",
    "......############......",
    "........###..###........",   # mouth
    "........########........",
    "..........####..........",   # neck
    ".........######.........",   # torso
    "........########........",   # shoulders + upper arms
    "........########........",
    ".......#.######.#.......",   # forearms
    ".......#.######.#.......",
    "......#..######..#......",   # hands
    "..........####..........",   # hips
    ".........##..##.........",   # legs
    ".........##..##.........",
    ".........##..##.........",
    ".........##..##.........",
    ".........##..##.........",
    "........###..###........",   # feet
    "........................",
]

# --------------------------------------------------------------------------
#  Frame B - lean right, left arm thrown up, left leg kicked out.
# --------------------------------------------------------------------------
FRAME_B = [
    "........................",
    "........##########......",
    ".........########.......",
    ".......############.....",
    ".......####.##.####.....",
    ".......####.##.####.....",
    ".......############.....",
    ".....#...###..###.......",   # raised hand
    "......#..########.......",
    ".......#...####.........",
    "........#.######........",
    ".........########.......",
    "..........#######.......",
    "..........######.#......",
    "..........######.#......",
    "..........######........",
    "...........####.........",
    "..........##..##........",
    ".........##...##........",
    "........##....##........",
    ".......##.....##........",
    "......##......##........",
    ".....###......###.......",
    "........................",
]

# --------------------------------------------------------------------------
#  Frame C - both arms out wide, feet apart. The peak of the bar.
# --------------------------------------------------------------------------
FRAME_C = [
    "........................",
    ".......##########.......",
    "........########........",
    "......############......",
    "......####.##.####......",
    "......####.##.####......",
    "......############......",
    "........###..###........",
    "........########........",
    "....##....####....##....",   # hands out wide
    "......#..######..#......",
    ".......##########.......",   # arms level with the shoulders
    ".........######.........",
    ".........######.........",
    ".........######.........",
    ".........######.........",
    "..........####..........",
    ".........##..##.........",
    "........##....##........",
    "........##....##........",
    ".......##......##.......",
    ".......##......##.......",
    "......###......###......",
    "........................",
]

FRAMES = [FRAME_A, FRAME_B, FRAME_C, None]   # D is filled in as mirror(B)
NAMES = ["A (rest)", "B (lean right)", "C (arms wide)", "D (lean left)"]


def mirror(rows):
    return [r[::-1] for r in rows]


def validate(rows, name):
    if len(rows) != H:
        raise ValueError(f"{name}: {len(rows)} rows, expected {H}")
    for i, r in enumerate(rows):
        if len(r) != W:
            raise ValueError(f"{name}: row {i} is {len(r)} px, expected {W}")
        bad = set(r) - {"#", "."}
        if bad:
            raise ValueError(f"{name}: row {i} has stray characters {bad}")


def to_image(rows):
    img = Image.new("1", (W, H), 0)
    px = img.load()
    for y, r in enumerate(rows):
        for x, c in enumerate(r):
            px[x, y] = 1 if c == "#" else 0
    return img


def pack(rows):
    out = []
    for r in rows:
        for bx in range(0, W, 8):
            b = 0
            for bit in range(8):
                x = bx + bit
                if x < W and r[x] == "#":
                    b |= 0x80 >> bit
            out.append(b)
    return out


def main():
    here = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    FRAMES[3] = mirror(FRAME_B)

    for rows, name in zip(FRAMES, NAMES):
        validate(rows, name)
        print(f"--- frame {name} ---")
        print("\n".join(rows))
        print()
    print(f"all {len(FRAMES)} frames valid: {W}x{H}, "
          f"{len(pack(FRAMES[0]))} bytes each")

    # contact sheet, also used as a report figure
    scale, gap = 8, 10
    sheet = Image.new("L", (len(FRAMES) * W * scale + (len(FRAMES) + 1) * gap,
                            H * scale + 2 * gap), 255)
    for i, rows in enumerate(FRAMES):
        big = to_image(rows).convert("L").point(
            lambda v: 0 if v else 255).resize((W * scale, H * scale),
                                              Image.NEAREST)
        sheet.paste(big, (gap + i * (W * scale + gap), gap))
    os.makedirs(os.path.join(here, "figures"), exist_ok=True)
    sheet_path = os.path.join(here, "figures", "sprite_frames.png")
    sheet.save(sheet_path)

    lines = ['''/************************************************************
  sprite.h - AUTO-GENERATED, do not edit by hand.
  Regenerate with:  python3 tools/sprite_gen.py
  Frame artwork lives in that script, as pixel art.

  Four-frame 24x24 dancing sprite for the OLED now-playing screen.
  Packed for Adafruit_GFX::drawBitmap(): 3 bytes per row, 24 rows,
  72 bytes per frame, most-significant bit = leftmost pixel.
  Total cost: 288 bytes of flash.

  Frames advance one per beat, so the sprite always dances at the
  tempo of the song that is playing. The frame index is
  player.beatIndex(now) % SPRITE_FRAMES, computed in
  renderNowPlaying() - it comes from the song clock, not a timer.
************************************************************/

// The guard is deliberately not SPRITE_H - that name is taken by the
// sprite's pixel height below.
#ifndef SPRITE_BITMAPS_H
#define SPRITE_BITMAPS_H

#include <Arduino.h>

#define SPRITE_W 24
#define SPRITE_H 24
#define SPRITE_FRAMES 4
''']

    for i, rows in enumerate(FRAMES):
        data = pack(rows)
        body = "\n".join(
            "  " + " ".join(f"0x{b:02X}," for b in data[r:r + 12])
            for r in range(0, len(data), 12)).rstrip(",")
        lines.append(f"""
// frame {i} - {NAMES[i]}
const unsigned char PROGMEM danceFrame{i}[] = {{
{body}
}};""")

    lines.append("""
const unsigned char *const danceFrames[SPRITE_FRAMES] = {
  danceFrame0, danceFrame1, danceFrame2, danceFrame3
};

#endif  // SPRITE_BITMAPS_H""")

    out_path = os.path.join(here, "sketch", "sprite.h")
    with open(out_path, "w") as f:
        f.write("\n".join(lines) + "\n")
    print(f"wrote {out_path}")
    print(f"wrote {sheet_path}")


if __name__ == "__main__":
    main()
