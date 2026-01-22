#include "usb_update.h"
#include <SD.h>
#include <USBHost_t36.h>

// External USB filesystem object (should be defined in main.cpp)
extern USBFilesystem usb_filesystem;

// Helper function to check if a string ends with a given suffix
static bool ends_with(const char *str, const char *suffix) {
    if (!str || !suffix)
        return false;
    
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    
    if (suffix_len > str_len)
        return false;
    
    return strncmp(str + str_len - suffix_len, suffix, suffix_len) == 0;
}

bool usb_has_bins() {
    // Check if USB filesystem is available
    if (!usb_filesystem) {
        return false;
    }
    
    // Try to open the folder
    File dir = usb_filesystem.open(PATTERN_FOLDER_NAME);
    if (!dir) {
        return false;
    }
    
    // Check if it's actually a directory
    if (!dir.isDirectory()) {
        dir.close();
        return false;
    }
    
    // Look for at least one non-empty .bin file
    bool found_bin = false;
    File entry;
    
    while ((entry = dir.openNextFile())) {
        // Check if it's a file (not a directory)
        if (!entry.isDirectory()) {
            const char *name = entry.name();
            
            // Check if it ends with .bin and doesn't start with '.'
            if (ends_with(name, ".bin") && name[0] != '.') {
                // Check if it's not empty
                if (entry.size() > 0) {
                    found_bin = true;
                    entry.close();
                }
                Serial.print("   Found file: ");
                Serial.println(name);
            }
        }
        entry.close();
    }
    
    dir.close();
    return found_bin;
}

bool sd_remove_bins() {
    // Check if SD card is available
    if (!SD.begin(BUILTIN_SDCARD)) {
        return false;
    }
    
    // Open the root directory
    File root = SD.open("/");
    if (!root) {
        return false;
    }
    
    if (!root.isDirectory()) {
        root.close();
        return false;
    }
    
    // Iterate through all files in root
    File entry;
    bool success = true;
    
    while ((entry = root.openNextFile())) {
        // Only process files, not directories
        if (!entry.isDirectory()) {
            const char *name = entry.name();
            
            // Check if it ends with .bin
            if (ends_with(name, ".bin")) {
                entry.close();
                
                // Remove the file
                if (!SD.remove(name)) {
                    success = false;
                }
            } else {
                entry.close();
            }
        } else {
            entry.close();
        }
    }
    
    root.close();
    return success;
}

bool copy_bins_from_usb_to_sd() {
    // Check if USB filesystem is available
    if (!usb_filesystem) {
        return false;
    }
    
    // Check if SD card is available
    if (!SD.begin(BUILTIN_SDCARD)) {
        return false;
    }
    
    // Open the folder on USB
    File usb_dir = usb_filesystem.open(PATTERN_FOLDER_NAME);
    if (!usb_dir) {
        return false;
    }
    
    if (!usb_dir.isDirectory()) {
        usb_dir.close();
        return false;
    }
    
    // Iterate through all .bin files and copy them
    File entry;
    bool success = true;
    uint8_t buffer[512];  // Buffer for copying data
    
    while ((entry = usb_dir.openNextFile())) {
        // Only process files, not directories
        if (!entry.isDirectory()) {
            const char *name = entry.name();
            
            // Check if it ends with .bin and doesn't start with '.'
            if (ends_with(name, ".bin") && name[0] != '.') {
                Serial.print("   Copying file: ");
                Serial.println(name);
                // Extract just the filename (remove path if present)
                const char *filename = strrchr(name, '/');
                if (filename) {
                    filename++;  // Skip the '/'
                } else {
                    filename = name;
                }
                
                // Create/open destination file on SD card
                File dest = SD.open(filename, FILE_WRITE);
                if (dest) {
                    // Copy the file
                    size_t bytes_read;
                    while ((bytes_read = entry.read(buffer, sizeof(buffer))) > 0) {
                        size_t bytes_written = dest.write(buffer, bytes_read);
                        if (bytes_written != bytes_read) {
                            success = false;
                            break;
                        }
                    }
                    
                    dest.close();
                } else {
                    success = false;
                }
            }
        }
        entry.close();
    }
    
    usb_dir.close();
    return success;
}
