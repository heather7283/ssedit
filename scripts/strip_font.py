#!/usr/bin/env fontforge
import fontforge
import sys
import os

def load_keep_set(filepath):
    keep = set()
    with open(filepath, 'r', encoding='utf-8') as f:
        for raw in f:
            line = raw.strip()
            keep.add(line)
    return keep

def subset_font(in_path, out_path, keep_set):
    # Open font
    font = fontforge.open(in_path)
    # Iterate over a *copy* of the glyph list, since we'll be deleting
    for glyph in list(font.glyphs()):
        name = glyph.glyphname
        # unicode == -1 means glyph has no Unicode slot
        if name not in keep_set:
            font.removeGlyph(name)
    # Generate subset font
    font.generate(out_path)
    font.close()

def main():
    if len(sys.argv) != 4:
        print("Usage: fontforge -script subset_font.py IN_FONT OUT_FONT KEEP_CODEPOINTS_FILE")
        sys.exit(1)

    in_font, out_font, cp_file = sys.argv[1], sys.argv[2], sys.argv[3]
    if not os.path.isfile(in_font):
        print(f"Error: input font not found at {in_font}")
        sys.exit(1)
    if not os.path.isfile(cp_file):
        print(f"Error: codepoints file not found at {cp_file}")
        sys.exit(1)

    keep_set = load_keep_set(cp_file)
    if not keep_set:
        print("Warning: no valid codepoints loaded—output font will be empty.")
    subset_font(in_font, out_font, keep_set)
    print(f"Subset font saved as: {out_font}")

if __name__ == '__main__':
    main()

