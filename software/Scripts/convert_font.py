#!/usr/bin/env python3
"""
convert_font.py

Convert a TTF/OTF font to a C source file compatible with the SSD1306 font format used in this repo.
Generates a .c file with a static uint16_t array (one uint16 per row) and an SSD1306_Font_t instance.

Usage:
  python convert_font.py input.ttf --size 11 [--width 11] [--fixed] [-o out.c]

Notes:
- Renders ASCII characters 32..126 by default.
- Each glyph is stored as `height` uint16_t values (rows top->bottom). Bit 15 is leftmost pixel.
- Maximum width is 16 (rows are 16-bit values). Widths >16 will be cropped.

Requires Pillow: pip install pillow
"""

import argparse
import glob
import os
import re
import sys
from datetime import datetime

try:
    from PIL import Image, ImageFont, ImageDraw
except Exception:
    print("Pillow is required. Install with: pip install pillow")
    sys.exit(1)

ASCII_START = 32
ASCII_END = 126


def sanitize(s):
    s = os.path.splitext(os.path.basename(s))[0]
    s = re.sub(r'[^0-9a-zA-Z_]', '_', s)
    if re.match(r'^[0-9]', s):
        s = '_' + s
    return s


def render_glyph_rows(font, ch, width, height, threshold=128):
    # Draw glyph to temporary image and return list of `height` 16-bit row ints (left-aligned)
    img_w = max(width, 1)
    im = Image.new('L', (img_w, height), 0)
    draw = ImageDraw.Draw(im)
    draw.text((0, 0), ch, font=font, fill=255)
    bbox = im.getbbox()
    rows = []
    if bbox is None:
        # empty glyph -> all zeros
        return [0] * height
    # crop left to right using bbox.left..bbox.right, but left-align into width
    left = bbox[0]
    right = bbox[2]
    glyph_w = right - left
    if glyph_w > width:
        # crop to width
        glyph_w = width
    # For each row, compute 16-bit value
    for y in range(height):
        rv = 0
        for x in range(width):
            src_x = left + x
            if src_x >= img_w:
                px = 0
            else:
                px = im.getpixel((src_x, y))
            if px >= threshold:
                rv |= (1 << (15 - x))
        rows.append(rv & 0xFFFF)
    return rows


def compute_char_width(font, ch, max_width, height):
    # Draw glyph and return used width (0..max_width)
    img_w = max_width
    im = Image.new('L', (img_w, height), 0)
    draw = ImageDraw.Draw(im)
    draw.text((0, 0), ch, font=font, fill=255)
    bbox = im.getbbox()
    if bbox is None:
        return 0
    left, top, right, bottom = bbox
    w = right - left
    return min(w, max_width)


def to_lower_camel(s):
    # normalize separators to spaces
    s = re.sub(r'[^0-9A-Za-z]+', ' ', s).strip()
    if not s:
        return ''
    # split camel-case and words (numbers preserved)
    parts = []
    for token in s.split():
        # split CamelCase tokens like 'BlenderPro' -> ['Blender','Pro']
        sub = re.findall(r'[A-Z]?[a-z]+|[0-9]+|[A-Z]+(?![a-z])', token)
        parts.extend(sub)
    if not parts:
        parts = [s]
    # produce lower camel: first word lower, others Title cased
    first = parts[0].lower()
    rest = ''.join(p.title() for p in parts[1:])
    base = first + rest
    # ensure it doesn't start with a digit (identifiers can't start with digit)
    if re.match(r'^[0-9]', base):
        base = '_' + base
    return base


def to_upper_underscore(s):
    # take basename and split into words, then join with underscores uppercase
    s = os.path.splitext(os.path.basename(s))[0]
    s = re.sub(r'[^0-9A-Za-z]+', ' ', s).strip()
    if not s:
        return ''
    parts = []
    for token in s.split():
        sub = re.findall(r'[A-Z]?[a-z]+|[0-9]+|[A-Z]+(?![a-z])', token)
        parts.extend(sub)
    if not parts:
        parts = [s]
    return '_'.join(p.upper() for p in parts)


def find_min_size_for_font(infile, max_search_size=72, threshold=128, min_glyph_width=None):
    """Find the smallest point size (1..max_search_size) for which glyphs meet the criteria.
    If min_glyph_width is None: return the smallest size with any non-zero glyph.
    If min_glyph_width is set: return smallest size whose computed max glyph width >= min_glyph_width.
    Returns the size or None if not found."""
    for s in range(1, max_search_size + 1):
        try:
            f = ImageFont.truetype(infile, size=s)
        except Exception:
            continue
        ascent, descent = f.getmetrics()
        height = ascent + descent
        max_w = 0
        for c in range(ASCII_START, ASCII_END + 1):
            w = compute_char_width(f, chr(c), 256, height)
            if w > max_w:
                max_w = w
        if min_glyph_width is None:
            if max_w > 0 and height > 0:
                return s
        else:
            # cap request to 16 since our glyphs are limited to 16px
            target = min(min_glyph_width, 16)
            if max_w >= target and height > 0:
                return s
    return None


def generate_font_files_for_size(infile, base_camel, base_raw, base_cap, size, width_arg, fixed, threshold, output_template=None):
    """Generate the .c/.h files for a single point size. Returns (outpath, outheader)."""
    try:
        font = ImageFont.truetype(infile, size=size)
    except Exception as e:
        print(f'Failed to load font size {size}:', e)
        return None, None

    ascent, descent = font.getmetrics()
    height = ascent + descent

    # compute maximum glyph width for ascii range for this point size
    max_w = 0
    for c in range(ASCII_START, ASCII_END + 1):
        ch = chr(c)
        w = compute_char_width(font, ch, 256, height)
        if w > max_w:
            max_w = w

    if width_arg is None:
        width = min(16, max_w)
    else:
        width = min(16, width_arg)
        if width_arg > 16:
            print('Warning: requested width > 16, cropping to 16')

    print('Generating size:', size, 'ascent/descent:', ascent, descent, '-> height', height)
    print('Computed max glyph width:', max_w, '-> using width:', width)

    glyphs = []  # list of lists (rows)
    char_widths = []
    for c in range(ASCII_START, ASCII_END + 1):
        ch = chr(c)
        rows = render_glyph_rows(font, ch, width, height, threshold)
        glyphs.append(rows)
        cw = compute_char_width(font, ch, width, height)
        if fixed:
            char_widths.append(width)
        else:
            char_widths.append(cw)

    # names include size to avoid collisions when generating many sizes
    data_name = f"fontData{base_cap}{size}pt{width}x{height}"
    font_name = f"font{base_cap}{size}pt{width}x{height}"
    char_width_name = f"charWidth{base_cap}{size}pt{width}x{height}"

    now = datetime.utcnow().isoformat() + 'Z'

    # choose output file paths
    if output_template:
        base_out = os.path.splitext(output_template)[0]
        # write to the specified output filename (no per-size suffix)
        outpath = f"{base_out}.c"
    else:
        outpath = os.path.join(os.getcwd(), 'Fonts', f"{base_camel}.c")
    outheader = os.path.splitext(outpath)[0] + '.h'

    base_macro = to_upper_underscore(base_raw)
    guard = f"SSD1306_{base_macro}_FONT_{width}x{height}"

    # write header file
    hdr_guard = (base_camel + f"_{size}pt_{width}x{height}_H").upper()
    with open(outheader, 'w', encoding='utf-8') as hf:
        hf.write(f"/* Generated by convert_font.py from {os.path.basename(infile)} on {now} */\n")
        hf.write('#ifndef %s\n' % hdr_guard)
        hf.write('#define %s\n\n' % hdr_guard)
        hf.write('#include "ssd1306_fonts.h"\n\n')
        hf.write('#ifdef __cplusplus\nextern "C" {\n#endif\n\n')
        hf.write(f"#ifdef {guard}\n")
        hf.write(f"extern const SSD1306_Font_t {font_name};\n")
        hf.write('#endif\n\n')
        hf.write('#ifdef __cplusplus\n}\n#endif\n\n')
        hf.write('#endif /* %s */\n' % hdr_guard)

    # write c file
    with open(outpath, 'w', encoding='utf-8') as f:
        f.write(f"/* Generated by convert_font.py from {os.path.basename(infile)} on {now} */\n")
        f.write(f'#include "%s"\n\n' % os.path.basename(outheader))

        f.write(f"#ifdef {guard}\n")

        f.write(f"static const uint16_t {data_name}[] = {{\n")
        # Write one glyph per line
        for idx, rows in enumerate(glyphs):
            ch = chr(ASCII_START + idx)
            row_strs = [f"0x{r:04X}" for r in rows]
            line = ', '.join(row_strs)
            if 32 <= ord(ch) <= 126 and ch not in ('\\', "'"):
                comment_char = ch
            else:
                comment_char = ''
            f.write(f"    {line},  // {ord(ch)} '{comment_char}'\n")
        f.write('};\n\n')

        if not fixed:
            f.write(f"static const uint8_t {char_width_name}[] = {{\n    ")
            cw_lines = []
            for i, cw in enumerate(char_widths):
                cw_lines.append(str(cw))
            for i in range(0, len(cw_lines), 16):
                f.write(', '.join(cw_lines[i:i+16]))
                f.write(',\n    ' if i+16 < len(cw_lines) else '\n')
            f.write('};\n\n')

        char_width_ptr = char_width_name if not fixed else 'NULL'
        f.write(f"const SSD1306_Font_t {font_name} = {{{width}, {height}, {data_name}, {char_width_ptr}}};\n")

        f.write('#endif\n')

    print('Wrote', outpath)
    print('Wrote', outheader)
    return outpath, outheader


def generate_font_block_strings(infile, base_camel, base_raw, base_cap, size, width_arg, fixed, threshold):
    """Generate the C block (static arrays + SSD1306_Font_t) and header extern line for a given size.
    Returns (c_block_str, header_extern_str, guard, font_name, width, height)."""
    try:
        font = ImageFont.truetype(infile, size=size)
    except Exception as e:
        print(f'Failed to load font size {size}:', e)
        return None, None, None, None, None, None

    ascent, descent = font.getmetrics()
    height = ascent + descent

    # compute maximum glyph width for ascii range for this point size
    max_w = 0
    glyphs = []
    char_widths = []
    for c in range(ASCII_START, ASCII_END + 1):
        ch = chr(c)
        w = compute_char_width(font, ch, 256, height)
        if w > max_w:
            max_w = w

    if width_arg is None:
        width = min(16, max_w)
    else:
        width = min(16, width_arg)
        if width_arg > 16:
            print('Warning: requested width > 16, cropping to 16')

    # build glyphs and widths
    for c in range(ASCII_START, ASCII_END + 1):
        ch = chr(c)
        rows = render_glyph_rows(font, ch, width, height, threshold)
        glyphs.append(rows)
        cw = compute_char_width(font, ch, width, height)
        if fixed:
            char_widths.append(width)
        else:
            char_widths.append(cw)

    data_name = f"fontData{base_cap}{size}pt{width}x{height}"
    font_name = f"font{base_cap}{size}pt{width}x{height}"
    char_width_name = f"charWidth{base_cap}{size}pt{width}x{height}"

    # family-specific guard
    base_macro = to_upper_underscore(base_raw)
    guard = f"SSD1306_{base_macro}_FONT_{width}x{height}"

    # build c block as string
    lines = []
    lines.append(f"/* size {size}pt -> {width}x{height} */")
    lines.append(f"static const uint16_t {data_name}[] = {{")
    for idx, rows in enumerate(glyphs):
        ch = chr(ASCII_START + idx)
        row_strs = [f"0x{r:04X}" for r in rows]
        line = ', '.join(row_strs)
        if 32 <= ord(ch) <= 126 and ch not in ('\\', "'"):
            comment_char = ch
        else:
            comment_char = ''
        lines.append(f"    {line},  // {ord(ch)} '{comment_char}'")
    lines.append('};\n')

    if not fixed:
        lines.append(f"static const uint8_t {char_width_name}[] = {{")
        for i in range(0, len(char_widths), 16):
            chunk = ', '.join(str(x) for x in char_widths[i:i+16])
            lines.append(f"    {chunk},")
        lines.append('};\n')

    char_width_ptr = char_width_name if not fixed else 'NULL'
    lines.append(f"const SSD1306_Font_t {font_name} = {{{width}, {height}, {data_name}, {char_width_ptr}}};\n")

    c_block = '\n'.join(lines)
    h_line = f"extern const SSD1306_Font_t {font_name};"
    return c_block, h_line, guard, font_name, width, height


def scan_font_headers_and_write_index(fonts_dir, core_inc_path, core_src_path):
    """Scan header files in fonts_dir for guarded extern declarations and regenerate
    a centralized fonts.c and fonts.h that list available fonts and provide lookup APIs."""
    entries = []  # tuples of (guard, symbol, width, height, name)

    # scan all .h files in fonts_dir
    for hfile in glob.glob(os.path.join(fonts_dir, '*.h')):
        content = open(hfile, 'r', encoding='utf-8').read()
        # find pairs of guard + extern symbol within the header
        matches = re.findall(r'#ifdef\s+(SSD1306_[A-Z0-9_]*_FONT_\d+x\d+).*?extern\s+const\s+SSD1306_Font_t\s+([A-Za-z0-9_]+)\s*;', content, flags=re.DOTALL)
        for guard, symbol in matches:
            m = re.search(r'(\d+)x(\d+)', guard)
            if m:
                w = int(m.group(1))
                h = int(m.group(2))
            else:
                w = 0
                h = 0
            entries.append((guard, symbol, w, h))

    # also include builtin fonts (always check these macros in config)
    builtin = [
        ("SSD1306_INCLUDE_FONT_6x8", "Font_6x8", 6, 8),
        ("SSD1306_INCLUDE_FONT_7x10", "Font_7x10", 7, 10),
        ("SSD1306_INCLUDE_FONT_11x18", "Font_11x18", 11, 18),
        ("SSD1306_INCLUDE_FONT_16x26", "Font_16x26", 16, 26),
        ("SSD1306_INCLUDE_FONT_16x24", "Font_16x24", 16, 24),
        ("SSD1306_INCLUDE_FONT_16x15", "Font_16x15", 16, 15),
    ]
    # prepend builtins so they take precedence in the default list
    entries = builtin + entries

    # write Core/Inc/fonts.h (basic API header)
    now = datetime.utcnow().isoformat() + 'Z'
    with open(core_inc_path, 'w', encoding='utf-8') as hf:
        hf.write(f"/* Auto-generated by convert_font.py on {now} */\n")
        hf.write('#ifndef __FONTS_H__\n')
        hf.write('#define __FONTS_H__\n\n')
        hf.write('#include "ssd1306_fonts.h"\n')
        hf.write('#include <stdint.h>\n#include <stddef.h>\n\n')
        hf.write('#ifdef __cplusplus\nextern "C" {\n#endif\n\n')
        hf.write('typedef struct {\n')
        hf.write('    const char *name;\n')
        hf.write('    const SSD1306_Font_t *font;\n')
        hf.write('    uint8_t width;\n')
        hf.write('    uint8_t height;\n')
        hf.write('} FontDescriptor;\n\n')
        hf.write('const FontDescriptor *fonts_get_all(size_t *out_count);\n')
        hf.write('const SSD1306_Font_t *fonts_find_by_size(uint8_t width, uint8_t height);\n')
        hf.write('const SSD1306_Font_t *fonts_get_default(void);\n\n')
        hf.write('#ifdef __cplusplus\n}\n#endif\n\n')
        hf.write('#endif /* __FONTS_H__ */\n')

    # write Core/Src/fonts.c
    with open(core_src_path, 'w', encoding='utf-8') as cf:
        cf.write(f"/* Auto-generated by convert_font.py on {now} */\n")
        cf.write('#include "fonts.h"\n#include <stddef.h>\n\n')
        cf.write('static const FontDescriptor fonts[] = {\n')
        for guard, symbol, w, h in entries:
            cf.write(f"#ifdef {guard}\n")
            name = symbol
            cf.write(f"    {{ \"{name}\", &{symbol}, {w}, {h} }},\n")
            cf.write('#endif\n')
        cf.write('};\n\n')
        cf.write('const FontDescriptor *fonts_get_all(size_t *out_count)\n{\n')
        cf.write('    if (out_count) *out_count = sizeof(fonts)/sizeof(fonts[0]);\n')
        cf.write('    return fonts;\n}\n\n')
        cf.write('const SSD1306_Font_t *fonts_find_by_size(uint8_t width, uint8_t height)\n{\n')
        cf.write('    size_t n = sizeof(fonts)/sizeof(fonts[0]);\n')
        cf.write('    for (size_t i = 0; i < n; ++i) {\n')
        cf.write('        if (fonts[i].width == width && fonts[i].height == height)\n')
        cf.write('            return fonts[i].font;\n')
        cf.write('    }\n    return NULL;\n}\n\n')
        cf.write('const SSD1306_Font_t *fonts_get_default(void)\n{\n')
        cf.write('#ifdef SSD1306_INCLUDE_FONT_9x12\n')
        cf.write('    return &fontBlenderProBold11pt9x12;\n')
        cf.write('#endif\n')
        cf.write('#ifdef SSD1306_INCLUDE_FONT_11x18\n')
        cf.write('    return &Font_11x18;\n')
        cf.write('#endif\n')
        cf.write('    size_t n = sizeof(fonts)/sizeof(fonts[0]);\n    if (n > 0) return fonts[0].font;\n    return NULL;\n}\n')

    print('Updated font index: wrote', core_inc_path, 'and', core_src_path)

    # Also update the driver's font header with externs for discovered fonts
    ssd1306_h = os.path.join(os.getcwd(), 'Drivers', 'SSD1306', 'ssd1306_fonts.h')
    update_ssd1306_fonts_h(entries, ssd1306_h)

    # Enable the discovered Blender Pro Bold font guards in config.h
    config_path = os.path.join(os.getcwd(), 'Core', 'Inc', 'config.h')
    update_config_enable_font_guards(fonts_dir, config_path)


def update_config_enable_font_guards(fonts_dir, config_path):
    """Regenerate a USER CODE block at the bottom of config.h that lists all available
    SSD1306 font guards (builtins + discovered). The block is marked with
    /* USER CODE BEGIN SSD1306_FONTS */ / /* USER CODE END SSD1306_FONTS */ so users
    can edit inside if needed. Any previously enabled guards (#define ...) are
    preserved (they will appear uncommented in the generated block)."""
    # builtin fonts we always care about
    builtin = [
        ("SSD1306_INCLUDE_FONT_6x8", 6, 8),
        ("SSD1306_INCLUDE_FONT_7x10", 7, 10),
        ("SSD1306_INCLUDE_FONT_11x18", 11, 18),
        ("SSD1306_INCLUDE_FONT_16x26", 16, 26),
        ("SSD1306_INCLUDE_FONT_16x24", 16, 24),
        ("SSD1306_INCLUDE_FONT_16x15", 16, 15),
    ]

    # discover guards from headers in fonts_dir
    guards = set(g for g, _, _ in builtin)
    for hfile in glob.glob(os.path.join(fonts_dir, '*.h')):
        content = open(hfile, 'r', encoding='utf-8').read()
        matches = re.findall(r'#ifdef\s+(SSD1306_[A-Z0-9_]*_FONT_\d+x\d+)', content)
        for g in matches:
            guards.add(g)

    if not guards:
        print('No font guards found in', fonts_dir)
        return

    # read config.h
    with open(config_path, 'r', encoding='utf-8') as f:
        cfg = f.read()

    # look for currently enabled guards (#define present anywhere)
    currently_enabled = set(re.findall(r'^[ \t]*#define\s+(SSD1306_[A-Z0-9_]*_FONT_\d+x\d+|SSD1306_INCLUDE_FONT_\d+x\d+)\b', cfg, flags=re.M))

    # remove any existing #define lines (enabled or commented with //) for these guards to avoid duplicates
    cfg = re.sub(r'^[ \t]*(?://[ \t]*)?#define[ \t]+(SSD1306_[A-Z0-9_]*_FONT_\d+x\d+|SSD1306_INCLUDE_FONT_\d+x\d+)\b.*$', '', cfg, flags=re.M)

    # also remove any previously inserted USER CODE block so we don't end up with multiple blocks
    cfg = re.sub(r'/\* USER CODE BEGIN SSD1306_FONTS \*/.*?/\* USER CODE END SSD1306_FONTS \*/\n?', '', cfg, flags=re.S)

    # build auto-generated USER CODE block
    now = datetime.utcnow().isoformat() + 'Z'
    lines = []
    lines.append('/* ---------------------------------------------------------------------------')
    lines.append(' * SSD1306 Font selection (auto-generated) - edit inside USER CODE block')
    lines.append(' * Generated by Scripts/convert_font.py on %s' % now)
    lines.append(' * To enable a font, remove the "// " prefix from the corresponding line below')
    lines.append(' */')
    lines.append('/* USER CODE BEGIN SSD1306_FONTS */')
    lines.append('')

    # organize guards: separate builtins and families and sort by size
    parsed = []  # (group, w, h, guard)
    for g in sorted(guards):
        m_incl = re.match(r'SSD1306_INCLUDE_FONT_(\d+)x(\d+)', g)
        m_fam = re.match(r'SSD1306_([A-Z0-9_]+)_FONT_(\d+)x(\d+)', g)
        if m_incl:
            parsed.append(('builtin', int(m_incl.group(1)), int(m_incl.group(2)), g))
        elif m_fam:
            parsed.append(('family', int(m_fam.group(2)), int(m_fam.group(3)), g))
        else:
            parsed.append(('other', 0, 0, g))
    parsed.sort(key=lambda x: (x[0], x[1], x[2], x[3]))

    # emit builtin group
    builtins_grp = [p for p in parsed if p[0] == 'builtin']
    if builtins_grp:
        lines.append('/* Built-in fonts */')
        for _, w, h, g in builtins_grp:
            if g in currently_enabled:
                lines.append(f'#define {g}')
            else:
                lines.append(f'// #define {g}')
        lines.append('')

    # emit family group
    fam_grp = [p for p in parsed if p[0] == 'family']
    if fam_grp:
        lines.append('/* 3rd-party / family fonts */')
        for _, w, h, g in fam_grp:
            if g in currently_enabled:
                lines.append(f'#define {g}')
            else:
                lines.append(f'// #define {g}')
        lines.append('')

    lines.append('/* USER CODE END SSD1306_FONTS */')
    block = '\n'.join(lines) + '\n'

    # insert the block before the trailing C++ block or final endif
    insert_marker = '\n#ifdef __cplusplus'
    pos = cfg.rfind(insert_marker)
    if pos == -1:
        pos = cfg.rfind('\n#endif /* CONFIG_H */')
    if pos == -1:
        pos = len(cfg)

    cfg = cfg[:pos] + '\n' + block + cfg[pos:]

    with open(config_path, 'w', encoding='utf-8') as f:
        f.write(cfg)

    print(f'Updated {config_path} with {len(guards)} font options (preserved {len(currently_enabled)} enabled)')


def update_ssd1306_fonts_h(entries, ssd1306_h_path):
    """Regenerate the auto-generated block of externs in the SSD1306 fonts header."""
    # entries: list of (guard, symbol, width, height)
    # build replacement block
    lines = []
    lines.append('// Auto-generated/3rd-party fonts (make accessible from project)')
    for guard, symbol, w, h in entries:
        lines.append(f'#ifdef {guard}')
        lines.append(f'extern const SSD1306_Font_t {symbol};')
        lines.append('#endif')
    block = '\n'.join(lines) + '\n'

    # read file
    with open(ssd1306_h_path, 'r', encoding='utf-8') as f:
        s = f.read()

    marker = '// Auto-generated/3rd-party fonts (make accessible from project)'
    if marker in s:
        start = s.index(marker)
        # replace until end of file guard
        end_marker = '\n#endif // __SSD1306_FONTS_H__'
        end = s.rfind(end_marker)
        if end != -1:
            new = s[:start] + block + s[end:]
            with open(ssd1306_h_path, 'w', encoding='utf-8') as f:
                f.write(new)
            print('Updated', ssd1306_h_path)
            return
    # fallback: append before final endif
    s = s.replace('\n#endif // __SSD1306_FONTS_H__', '\n' + block + '\n#endif // __SSD1306_FONTS_H__')
    with open(ssd1306_h_path, 'w', encoding='utf-8') as f:
        f.write(s)
    print('Appended auto-generated fonts to', ssd1306_h_path)


def main():
    p = argparse.ArgumentParser(description='Convert a TTF/OTF to SSD1306 C font')
    p.add_argument('infile', help='input .ttf or .otf font file')
    p.add_argument('-s', '--size', type=int, default=11, help='point size for rendering (default: 11)')
    p.add_argument('-w', '--width', type=int, default=None, help='fixed glyph width in pixels (<=16). If omitted, computed and capped at 16')
    p.add_argument('-f', '--fixed', action='store_true', help='generate a monospaced font (char_width=NULL)')
    p.add_argument('-o', '--output', default=None, help='output .c file (default: <infile>_<size>pt_<WxH>.c or for ranges <base>_<min>to<max>pt.c)')
    p.add_argument('-n', '--name', default=None, help='base name for generated font variables (default: input basename)')
    p.add_argument('-t', '--threshold', type=int, default=128, help='grayscale threshold (0-255) for pixel on (default 128)')
    p.add_argument('-G', '--generate-range', action='store_true', help='Generate multiple sizes starting from the minimum to N sizes up (use with --count). All sizes will be emitted into a single .c/.h pair.')
    p.add_argument('-c', '--count', type=int, default=6, help='Number of sizes to generate when used with --generate-range (default: 6)')
    p.add_argument('--max-search-size', type=int, default=72, help='Maximum size to search when finding minimum size (default 72)')
    p.add_argument('--min-width', type=int, default=None, help='Minimum glyph width (pixels) to start generation from (searches for smallest point size with glyphs this wide)')
    p.add_argument('--good-width', action='store_true', help='Start from a reasonable default width (5 px) instead of the tiny minimum')
    p.add_argument('--outdir', default=None, help='Output directory for generated font files (default: same as input file)')
    args = p.parse_args()

    infile = args.infile
    if not os.path.isfile(infile):
        print('Input file not found:', infile)
        sys.exit(1)

    base_raw = args.name if args.name else os.path.splitext(os.path.basename(infile))[0]
    base_camel = to_lower_camel(base_raw)
    base = sanitize(base_raw)
    if not base_camel:
        base_camel = base.lower()

    base_cap = base_camel[0].upper() + base_camel[1:]

    # if output specified and generating a range, treat it as a template base (we'll append size info)
    out_template = args.output

    if args.generate_range:
        # determine desired starting width if requested
        min_width_target = None
        if args.min_width is not None:
            min_width_target = args.min_width
        elif args.good_width:
            min_width_target = 5
        if min_width_target is not None and (min_width_target < 1 or min_width_target > 256):
            print('min-width should be in a sensible range (1..256). Clamping to valid range.')
            min_width_target = max(1, min(256, min_width_target))

        # first try to find a point size that reaches the requested min glyph width
        min_size = None
        if min_width_target is not None:
            if min_width_target > 16:
                print('Requested min-width > 16, capping to 16 (max glyph width).')
            capped = min(min_width_target, 16)
            print(f'Searching for smallest point size with glyph width >= {capped} px (max search {args.max_search_size})')
            min_size = find_min_size_for_font(infile, args.max_search_size, args.threshold, min_glyph_width=capped)
            if min_size is None:
                print(f'Could not find a size that meets glyph width >= {capped}. Falling back to smallest non-empty size.')

        # fallback: search for the tiniest non-empty glyph size
        if min_size is None:
            min_size = find_min_size_for_font(infile, args.max_search_size, args.threshold, None)

        if min_size is None:
            print(f'Could not find a usable minimum size up to {args.max_search_size}. Aborting.')
            sys.exit(1)

        sizes = [min_size + i for i in range(args.count)]

        blocks = []
        h_decls = []
        font_names = []
        for size in sizes:
            print('Font:', infile)
            print('Rendered size:', size)
            c_block, h_line, guard, font_name, w, h = generate_font_block_strings(infile, base_camel, base_raw, base_cap, size, args.width, args.fixed, args.threshold)
            if c_block is None:
                print('Failed to generate size', size)
                continue
            blocks.append((c_block, guard, w, h))
            h_decls.append((h_line, guard, w, h))
            font_names.append(font_name)

        if not blocks:
            print('No font sizes generated.')
            sys.exit(1)

        # determine combined output filenames
        outdir = args.outdir if args.outdir else os.path.join(os.getcwd(), 'Fonts')
        os.makedirs(outdir, exist_ok=True)
        if out_template:
            combined_c = out_template
            combined_h = os.path.splitext(out_template)[0] + '.h'
        else:
            combined_c = os.path.join(outdir, f"{base_camel}.c")
            combined_h = os.path.splitext(combined_c)[0] + '.h'

        now = datetime.utcnow().isoformat() + 'Z'
        hdr_guard = (os.path.splitext(os.path.basename(combined_h))[0] + '_H').upper()

        # write header
        with open(combined_h, 'w', encoding='utf-8') as hf:
            hf.write(f"/* Generated by convert_font.py from {os.path.basename(infile)} on {now} */\n")
            hf.write('#ifndef %s\n' % hdr_guard)
            hf.write('#define %s\n\n' % hdr_guard)
            hf.write('#include "ssd1306_fonts.h"\n\n')
            hf.write('#ifdef __cplusplus\nextern "C" {\n#endif\n\n')
            for decl, guard, w, h in h_decls:
                hf.write(f"#ifdef {guard}\n")
                hf.write(decl + '\n')
                hf.write('#endif\n')
            hf.write('\n#ifdef __cplusplus\n}\n#endif\n\n')
            hf.write('#endif /* %s */\n' % hdr_guard)

        # write combined c
        with open(combined_c, 'w', encoding='utf-8') as cf:
            cf.write(f"/* Generated by convert_font.py from {os.path.basename(infile)} on {now} */\n")
            cf.write(f'#include "%s"\n\n' % os.path.basename(combined_h))
            for b, guard, w, h in blocks:
                cf.write(f"#ifdef {guard}\n")
                cf.write(b + '\n')
                cf.write('#endif\n\n')

        print('Wrote', combined_c)
        print('Wrote', combined_h)

        # update centralized fonts index (Core/Inc/fonts.h and Core/Src/fonts.c)
        core_inc = os.path.join(os.getcwd(), 'Core', 'Inc', 'fonts.h')
        core_src = os.path.join(os.getcwd(), 'Core', 'Src', 'fonts.c')
        scan_font_headers_and_write_index(outdir, core_inc, core_src)
        return
    else:
        # single-size behavior (unchanged)
        for size in [args.size]:
            font = ImageFont.truetype(infile, size=size)
            ascent, descent = font.getmetrics()
            height = ascent + descent
            print('Font:', infile)
            print('Rendered size:', size, 'ascent/descent:', ascent, descent, '-> height', height)
            generate_font_files_for_size(infile, base_camel, base, base_cap, size, args.width, args.fixed, args.threshold, out_template)

    # scan fonts directory and update Core/Inc/fonts.h and Core/Src/fonts.c
    if args.outdir:
        fonts_dir = args.outdir
    else:
        fonts_dir = os.path.dirname(infile)

    core_inc_path = os.path.join(fonts_dir, 'Core', 'Inc', 'fonts.h')
    core_src_path = os.path.join(fonts_dir, 'Core', 'Src', 'fonts.c')
    scan_font_headers_and_write_index(fonts_dir, core_inc_path, core_src_path)


if __name__ == '__main__':
    main()
