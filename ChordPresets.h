#ifndef CHORD_PRESETS_H
#define CHORD_PRESETS_H

#define NOTES_PER_CHORD 4
#define CHORD_LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

// Preset settings.
struct ChordPreset {
  const char* name;
  const int (*chords)[NOTES_PER_CHORD];
  int length;
};

// Preset 1
const int chords1[][NOTES_PER_CHORD] = {
  { 60, 64, 67, 71 },  // CM7
  { 53, 57, 60, 64 }   // FM7
};
ChordPreset preset1 = { "IM7 > IVM7", chords1, CHORD_LENGTH(chords1) };

// Preset 2
const int chords2[][NOTES_PER_CHORD] = {
  { 62, 64, 69, -1 },
  { 55, 62, 69, -1 },
  { 60, 67, 71, -1 }
};
ChordPreset preset2 = { "IIm7 > V7 > IM7", chords2, CHORD_LENGTH(chords2) };

// Preset 3
const int chords3[][NOTES_PER_CHORD] = {
  { 60, 67, 74, -1 },
  { 58, 65, 72, -1 }
};
ChordPreset preset3 = { "Isus2 > bVIIsus2", chords3, CHORD_LENGTH(chords3) };

// Preset list
ChordPreset presets[] = {
  preset1,
  preset2,
  preset3
};

#endif  // CHORD_PRESETS_H
