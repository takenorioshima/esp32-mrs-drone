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

// Preset 1
const int chords1[][NOTES_PER_CHORD] = {
  { 60, 64, 67, 71 },  // CM7
  { 53, 57, 60, 64 }   // FM7
};
ChordPreset preset1 = { "IM7 > IVM7", chords1, NUM_CHORDS(chords1) };

// Preset 2
const int chords2[][NOTES_PER_CHORD] = {
  { 62, 64, 69, -1 },
  { 55, 62, 69, -1 },
  { 60, 67, 71, -1 }
};
ChordPreset preset2 = { "IIm7 > V7 > IM7", chords2, NUM_CHORDS(chords2) };

// Preset 3
const int chords3[][NOTES_PER_CHORD] = {
  { 60, 67, 74, -1 },
  { 58, 65, 72, -1 }
};
ChordPreset preset3 = { "Isus2 > bVIIsus2", chords3, NUM_CHORDS(chords3) };

// Preset 4
const int chords4[][NOTES_PER_CHORD] = {
  { 64, 72, 74, 83 },
  { 65, 67, 72, 76 },
  { 57, 65, 67, 72 },
  { 55, 65, 67, 76 },
};
ChordPreset preset4 = { "IIIm7 > IVM7 > VIIm7 > V7", chords4, NUM_CHORDS(chords4) };

// Preset 5
const int chords5[][NOTES_PER_CHORD] = {
  { 53, 69, 74, -1 },
  { 52, 67, 76, -1 },
  { 50, 67, 72, -1 },
  { 50, 67, 74, -1 },
};
ChordPreset preset5 = { "IV6 > IIIm > Gsus4/D > G/D", chords5, NUM_CHORDS(chords5) };

// Preset list
ChordPreset presets[] = {
  preset1,
  preset2,
  preset3,
  preset4,
  preset5,
};

#endif  // CHORD_PRESETS_H
