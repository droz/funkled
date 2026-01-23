
#include "cached_pattern.h"

cached_pattern_t cached_patterns[MAX_CACHED_PATTERN_NUMBER];
uint32_t num_cached_patterns = 0;

// List the name of the patterns available on the SD card
void load_cached_patterns() {
    Serial.println("Scanning SD card for cached patterns...");
    // First, collect all .bin filenames
    String filenames[MAX_CACHED_PATTERN_NUMBER];
    uint32_t num_files = 0;
    
    File root = SD.open("/");
    while (true) {
        File file = root.openNextFile();
        if (!file) {
            break;  // no more files
        }
        if (!file.isDirectory()) {
            const char* name = file.name();
            // simple *.bin match with a filter to remove .files
            const char* ext = strrchr(name, '.');
            if (ext && strcasecmp(ext, ".bin") == 0 && name[0] != '.') {
                if (num_files < MAX_CACHED_PATTERN_NUMBER) {
                    Serial.print("  Found pattern file: ");
                    Serial.println(name);
                    filenames[num_files++] = String(name);
                } else {
                    Serial.println("Warning: maximum cached pattern number reached during scanning");
                    break;
                }
            }
        }
        file.close();
    }
    root.close();
    
    // Sort filenames alphabetically
    for (uint32_t i = 0; i < num_files - 1; i++) {
        for (uint32_t j = i + 1; j < num_files; j++) {
            if (filenames[i].compareTo(filenames[j]) > 0) {
                String temp = filenames[i];
                filenames[i] = filenames[j];
                filenames[j] = temp;
            }
        }
    }
    
    // Now load files in alphabetical order
    num_cached_patterns = 0;
    for (uint32_t i = 0; i < num_files; i++) {
        File file = SD.open(filenames[i].c_str());
        if (file) {
            Serial.print("Loaded cached pattern file: ");
            Serial.println(filenames[i]);
            cached_pattern_t& pattern = cached_patterns[num_cached_patterns++];
            pattern.filepath = filenames[i];
            pattern.file = file;
            file.read(&pattern.header, sizeof(pattern.header));
            Serial.printf("\tmagic: %X\n", pattern.header.magic);
            Serial.printf("\tcolor_ordering: %d\n", pattern.header.color_ordering);
            Serial.printf("\tnum_pixels: %d\n", pattern.header.num_pixels);
            Serial.printf("\tanimation_steps: %d\n", pattern.header.animation_steps);
            Serial.printf("\tanimation_period_s: %d\n", pattern.header.animation_period_s);
        }
    }
}
