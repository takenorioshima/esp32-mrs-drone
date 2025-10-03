#ifndef CHORD_PRESETS_H
#define CHORD_PRESETS_H

#define NOTES_PER_CHORD 4
#define NUM_CHORDS(arr) (sizeof(arr) / sizeof(arr[0]))

// Preset settings.
struct ChordPreset {
  const char* name;
  const int (*chords)[NOTES_PER_CHORD];
  int numChords;
};

/*
MIDI Note Numbers

Note |   C2   |   C3   |   C4   |   C5
------------------------------------------
C    |   36   |   48   |   60   |   72
Db   |   37   |   49   |   61   |   73
D    |   38   |   50   |   62   |   74
Eb   |   39   |   51   |   63   |   75
E    |   40   |   52   |   64   |   76
F    |   41   |   53   |   65   |   77
Gb   |   42   |   54   |   66   |   78
G    |   43   |   55   |   67   |   79
Ab   |   44   |   56   |   68   |   80
A    |   45   |   57   |   69   |   81
Bb   |   46   |   58   |   70   |   82
B    |   47   |   59   |   71   |   83
*/

// Preset chords

// IM7 > IVM7
const int chords1[][NOTES_PER_CHORD] = {
  { 60, 64, 71, -1 },
  { 53, 64, 69, -1 }
};

// IIm7 > V7 > IM7
const int chords2[][NOTES_PER_CHORD] = {
  { 62, 65, 69, -1 },
  { 55, 65, 69, -1 },
  { 60, 67, 71, -1 }
};

// Isus2 > bVIIsus2
const int chords3[][NOTES_PER_CHORD] = {
  { 60, 67, 74, -1 },
  { 58, 65, 72, -1 }
};

// IIIm7 > IVM7 > VIIm7 > V7
const int chords4[][NOTES_PER_CHORD] = {
  { 64, 72, 74, 83 },
  { 65, 67, 72, 76 },
  { 57, 65, 67, 72 },
  { 55, 65, 67, 76 },
};

// IV6 > IIIm > Gsus4/D > G/D
const int chords5[][NOTES_PER_CHORD] = {
  { 53, 69, 74, -1 },
  { 52, 67, 76, -1 },
  { 57, 67, 72, -1 },
  { 55, 67, 74, -1 },
};

// IM7 > IIIbM7
const int chords6[][NOTES_PER_CHORD] = {
  { 60, 64, 67, 71 },
  { 63, 67, 70, 74 }
};

// VIm9 > V7 > IVM7 
const int chords7[][NOTES_PER_CHORD] = {
  { 57, 67, 71, 72 },
  { 55, 62, 71, 74 },
  { 53, 64, 69, -1 },
  { 53, 64, 67, -1 },
};

// Preset list
ChordPreset presets[] = {
  { "IM7 > IVM7", chords1, std::size(chords1) },
  { "IIm7 > V7 > IM7", chords2, std::size(chords2) },
  { "VIm9 > V7 > IVM7", chords7, std::size(chords2) },
  { "Isus2 > bVIIsus2", chords3, std::size(chords3) },
  { "IM7 > IIIbM7", chords6, std::size(chords6) },
  { "IIIm7 > IVM7 > VIIm7 > V7", chords4, std::size(chords4) },
  { "IV6 > IIIm > Gsus4/D > G/D", chords5, std::size(chords5) },
};

#endif  // CHORD_PRESETS_H
