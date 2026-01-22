#ifndef USB_UPDATE_H
#define USB_UPDATE_H

#include <stdint.h>
#include <stdbool.h>

#define PATTERN_FOLDER_NAME "is_bed_patterns"

/**
 * Check if a folder named "is_bed_patterns" exists on the USB drive
 * and contains at least one non-empty .bin file.
 * 
 * @return true if the folder exists and contains at least one non-empty .bin file, false otherwise
 */
bool usb_has_bins();

/**
 * Remove all .bin files from the SD card root directory.
 * 
 * @return true if successful (or no files to remove), false on error
 */
bool sd_remove_bins();

/**
 * Copy all .bin files from the "is_bed_patterns" folder on the USB drive
 * to the root of the SD card.
 * 
 * @return true if successful, false on error
 */
bool copy_bins_from_usb_to_sd();

#endif // USB_UPDATE_H
