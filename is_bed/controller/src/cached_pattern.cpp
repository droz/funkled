
#include "cached_pattern.h"

cached_pattern_t cached_patterns[MAX_CACHED_PATTERN_NUMBER];
uint32_t num_cached_patterns = 0;

// List the name of the patterns available on the SD card
void load_cached_patterns() {
    File root = SD.open("/");
    while (true) {
        File file = root.openNextFile();
        if (!file) {
            break;  // no more files
        }
        bool close_file = true;
        if (!file.isDirectory()) {
            const char* name = file.name();
            // simple *.bin match with a filter to remove .files
            const char* ext = strrchr(name, '.');
            if (ext && strcasecmp(ext, ".bin") == 0 && name[0] != '.') {
                Serial.print("Found cached pattern file: ");
                Serial.println(name);
                if (num_cached_patterns < MAX_CACHED_PATTERN_NUMBER) {
                    cached_pattern_t& pattern = cached_patterns[num_cached_patterns++];
                    pattern.filepath = String(name);
                    pattern.file = file;
                    close_file = false; // keep the file open for reading
                    file.read(&pattern.header, sizeof(pattern.header));
                    Serial.printf("\tmagic: %X\n", pattern.header.magic);
                    Serial.printf("\tcolor_ordering: %d\n", pattern.header.color_ordering);
                    Serial.printf("\tnum_pixels: %d\n", pattern.header.num_pixels);
                    Serial.printf("\tanimation_steps: %d\n", pattern.header.animation_steps);
                    Serial.printf("\tanimation_period_s: %d\n", pattern.header.animation_period_s);
                } else {
                    Serial.println("Warning: maximum cached pattern number reached");
                }
            }
        }
        if (close_file) {
            file.close();
        }
    }
    root.close();
}
