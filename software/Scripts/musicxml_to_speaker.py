#!/usr/bin/env python3
"""
musicxml_to_speaker_notes.py
Parse a MusicXML file and export a header with `Speaker_Note` entries.

Usage:
    python musicxml_to_speaker_notes.py input.musicxml output.h --name SWALLOW --bpm 120
"""

import argparse
import xml.etree.ElementTree as ET
import math
from collections import defaultdict

NOTE_NAMES_SHARP = ['C','CS','D','DS','E','F','FS','G','GS','A','AS','B']
STEP_TO_SEMITONE = {'C':0,'D':2,'E':4,'F':5,'G':7,'A':9,'B':11}
TYPE_TO_QUARTER = {
    'whole': 4.0, 'half': 2.0, 'quarter': 1.0, 'eighth': 0.5,
    '16th': 0.25, '32nd': 0.125, '64th': 0.0625
}

def find_tempo(root):
    # Try <sound tempo="...">
    x = root.find('.//sound[@tempo]')
    if x is not None and 'tempo' in x.attrib:
        try:
            return float(x.attrib['tempo'])
        except:
            pass
    # Try direction -> metronome/per-minute
    pm = root.find('.//direction//metronome//per-minute')
    if pm is not None and pm.text:
        try:
            return float(pm.text)
        except:
            pass
    return None

def get_divisions(root):
    d = root.find('.//attributes/divisions')
    if d is not None and d.text:
        try:
            return float(d.text)
        except:
            pass
    return 1.0

def pitch_to_macro_or_freq(step, alter, octave):
    # compute absolute semitone and midi
    semitone = STEP_TO_SEMITONE[step] + (alter or 0)
    semitone_abs = (octave + 1) * 12 + semitone
    # try to use NOTE_ macro for octaves 3..7 (matches common speaker.h macros)
    semitone_in_oct = semitone_abs % 12
    new_oct = (semitone_abs // 12) - 1  # invert midi formula
    macro_name = f'NOTE_{NOTE_NAMES_SHARP[semitone_in_oct]}{new_oct}'
    # If macro octave is in 3..7 return macro, else fallback to number
    if 3 <= new_oct <= 7:
        return macro_name, True
    # fallback numeric frequency
    freq = 440.0 * (2 ** ((semitone_abs - 69) / 12.0))
    return str(int(round(freq))), False

def duration_divisions_to_ms(divisions, divisions_per_quarter, tempo_bpm):
    quarter_ms = 60000.0 / tempo_bpm
    return (divisions / divisions_per_quarter) * quarter_ms

def parse_musicxml(path, part_index=0, tempo_override=None):
    tree = ET.parse(path)
    root = tree.getroot()
    # MusicXML root usually has namespace — simplify by removing namespace prefixes
    for elem in root.iter():
        if '}' in elem.tag:
            elem.tag = elem.tag.split('}',1)[1]

    tempo = tempo_override or find_tempo(root) or 120.0
    divisions_per_quarter = get_divisions(root)

    # We'll parse the requested part (part_index) or first
    parts = root.findall('part')
    if not parts:
        raise RuntimeError("No <part> elements found in MusicXML.")
    if part_index >= len(parts):
        raise IndexError("Part index out of range.")
    part = parts[part_index]

    # handle simple ties: accumulate durations for tied notes keyed by (voice, pitch_abs)
    ongoing_ties = {}
    notes_out = []

    for measure in part.findall('measure'):
        for note in measure.findall('note'):
            # skip grace notes
            if note.find('grace') is not None:
                continue

            # voice (optional)
            voice_elem = note.find('voice')
            voice = voice_elem.text.strip() if voice_elem is not None and voice_elem.text else '1'

            is_rest = note.find('rest') is not None
            # duration in divisions (preferred)
            dur_elem = note.find('duration')
            if dur_elem is not None and dur_elem.text:
                dur_divs = float(dur_elem.text)
                dur_ms = duration_divisions_to_ms(dur_divs, divisions_per_quarter, tempo)
            else:
                # fallback to type/dots
                type_elem = note.find('type')
                dur_quarters = 1.0
                if type_elem is not None and type_elem.text:
                    t = type_elem.text.strip()
                    dur_quarters = TYPE_TO_QUARTER.get(t, 1.0)
                # dots
                dots = len(note.findall('dot'))
                dur_quarters *= (1.0 + 0.5 * dots)
                dur_ms = dur_quarters * (60000.0 / tempo)

            # tie info (could be <tie type="start"/> or notations/tied)
            tie_types = [t.attrib.get('type') for t in note.findall('tie') if 'type' in t.attrib]
            notations = note.find('notations')
            if notations is not None:
                for tied in notations.findall('tied'):
                    if 'type' in tied.attrib:
                        tie_types.append(tied.attrib['type'])

            if is_rest:
                # simple rest
                notes_out.append(('0', int(round(dur_ms))))
                continue

            # pitch
            pitch = note.find('pitch')
            if pitch is None:
                # skip unknown note
                continue
            step = pitch.find('step').text.strip()
            alter_elem = pitch.find('alter')
            alter = int(alter_elem.text) if (alter_elem is not None and alter_elem.text) else 0
            octave = int(pitch.find('octave').text.strip())

            # compute absolute semitone key to identify ties
            semitone = STEP_TO_SEMITONE[step] + alter
            semitone_abs = (octave + 1) * 12 + semitone
            key = (voice, semitone_abs)

            # tie handling
            starts = any(t == 'start' for t in tie_types)
            stops = any(t == 'stop' for t in tie_types)

            if starts and not stops:
                # start or continue a tie
                ongoing_ties.setdefault(key, 0)
                ongoing_ties[key] += dur_ms
                # don't output yet
                continue
            elif stops and not starts:
                # conclude an ongoing tie (if exists)
                if key in ongoing_ties:
                    ongoing_ties[key] += dur_ms
                    total = ongoing_ties.pop(key)
                    macro_or_freq, used_macro = pitch_to_macro_or_freq(step, alter, octave)
                    notes_out.append((macro_or_freq, int(round(total))))
                else:
                    # tie stop without start: treat as single note
                    macro_or_freq, used_macro = pitch_to_macro_or_freq(step, alter, octave)
                    notes_out.append((macro_or_freq, int(round(dur_ms))))
                continue
            elif starts and stops:
                # single note that both starts and stops (rare middle note). treat like single note
                total = dur_ms
                macro_or_freq, used_macro = pitch_to_macro_or_freq(step, alter, octave)
                notes_out.append((macro_or_freq, int(round(total))))
                continue
            else:
                # normal untied note
                macro_or_freq, used_macro = pitch_to_macro_or_freq(step, alter, octave)
                notes_out.append((macro_or_freq, int(round(dur_ms))))

    return notes_out, tempo

def write_header(out_path, name, notes, author_comment=None):
    guard = f'_{name.upper()}_H_'
    with open(out_path, 'w', newline='\n') as f:
        f.write('// Generated by musicxml_to_speaker_notes.py\n')
        if author_comment:
            f.write(f'// {author_comment}\n')
        f.write(f'#ifndef {guard}\n#define {guard}\n\n')
        f.write('#include "speaker.h"\n\n')
        f.write(f'static const Speaker_Note {name}[] = {{\n')
        for freq, dur in notes:
            f.write(f'    {{{freq}, {dur}}},\n')
        f.write('};\n\n')
        f.write(f'static const uint16_t {name}_len = (uint16_t)(sizeof({name})/sizeof({name}[0]));\n\n')
        f.write(f'#endif /* {guard} */\n')

def main():
    ap = argparse.ArgumentParser(description='Convert MusicXML to Speaker_Note C header.')
    ap.add_argument('infile', help='input MusicXML file')
    ap.add_argument('outfile', help='output .h file')
    ap.add_argument('--name', default='melody', help='C array name (default: melody)')
    ap.add_argument('--part', type=int, default=0, help='part index to extract (default: 0)')
    ap.add_argument('--bpm', type=float, default=None, help='override BPM (tempo)')
    args = ap.parse_args()

    notes, tempo = parse_musicxml(args.infile, part_index=args.part, tempo_override=args.bpm)
    write_header(args.outfile, args.name, notes, author_comment=f'From {args.infile}, tempo={tempo}')
    print(f'Wrote {len(notes)} notes to {args.outfile} (tempo={tempo} bpm)')

if __name__ == '__main__':
    main()