#!/usr/bin/env python3
"""
Mock Controller for IS Bed LCD
Sends valid frames to the LCD through the serial port for testing.
"""

import struct
import time
import argparse
import sys
from dataclasses import dataclass
from typing import List
import random
from pySerialTransfer import pySerialTransfer as txfer


# Protocol constants
NUM_ZONES = 4
PATTERN_NAME_SIZE = 16


@dataclass
class ColorRGB:
    """RGB color structure"""
    r: int
    g: int
    b: int


@dataclass
class ControllerToLcdFrame:
    """Frame sent from controller to LCD"""
    pattern_name: str
    pattern_index: int
    pattern_color_wheel: bool
    zone_colors: List[ColorRGB]

    def pack(self) -> bytes:
        """Pack the frame into bytes for serial transmission"""
        # Ensure pattern name is exactly 16 bytes
        name_bytes = self.pattern_name.encode('ascii')[:PATTERN_NAME_SIZE]
        name_bytes = name_bytes.ljust(PATTERN_NAME_SIZE, b'\x00')
        
        # Pack the structure
        data = struct.pack('B', self.pattern_index)
        data += name_bytes
        data += struct.pack('B', self.pattern_color_wheel)
        
        # Pack all zone colors (RGB)
        for color in self.zone_colors:
            data += struct.pack('BBB', color.r, color.g, color.b)
        
        return data


# Predefined patterns for testing
PATTERNS = [
    "Rotate",
    "Static",
    "Fade",
    "Blink",
    "Rainbow",
    "Fire",
    "Ocean",
    "Forest",
    "Sunset",
    "Aurora",
]

# Predefined color sets
COLOR_PRESETS = {
    "red": [ColorRGB(255, 0, 0)] * NUM_ZONES,
    "green": [ColorRGB(0, 255, 0)] * NUM_ZONES,
    "blue": [ColorRGB(0, 0, 255)] * NUM_ZONES,
    "white": [ColorRGB(255, 255, 255)] * NUM_ZONES,
    "rainbow": [
        ColorRGB(255, 0, 0),    # Cage: Red
        ColorRGB(255, 127, 0),  # Center: Orange
        ColorRGB(0, 255, 0),    # Front: Green
        ColorRGB(0, 0, 255),    # Headboard: Blue
    ],
    "warm": [
        ColorRGB(255, 100, 0),  # Warm orange
        ColorRGB(255, 50, 0),   # Deep orange
        ColorRGB(255, 150, 50), # Light orange
        ColorRGB(255, 75, 25),  # Red-orange
    ],
    "cool": [
        ColorRGB(0, 100, 255),  # Light blue
        ColorRGB(0, 200, 200),  # Cyan
        ColorRGB(100, 0, 255),  # Purple
        ColorRGB(0, 150, 255),  # Sky blue
    ],
}


def send_pattern_enumeration(transfer: txfer.SerialTransfer, delay: float = 0.1) -> None:
    """Send all pattern names to the LCD for enumeration"""
    print(f"\nEnumerating {len(PATTERNS)} patterns...")
    for idx, pattern_name in enumerate(PATTERNS):
        frame = ControllerToLcdFrame(
            pattern_name=pattern_name,
            pattern_index=idx,
            pattern_color_wheel= (idx == 0),  # Enable color wheel for first pattern
            zone_colors=[ColorRGB(0, 0, 0)] * NUM_ZONES
        )
        payload = frame.pack()
        print(f"  Sending pattern {idx}: {pattern_name}")
        print(f"    Frame size: {len(payload)} bytes")
        print(f"    Payload: {payload.hex(' ')}")
        transfer.tx_buff = payload
        transfer.send(len(payload))
    print("Pattern enumeration complete!")


def send_color_update(transfer: txfer.SerialTransfer, colors: List[ColorRGB], 
                      pattern_idx: int = 0) -> None:
    """Send a color update frame"""
    frame = ControllerToLcdFrame(
        pattern_name=PATTERNS[pattern_idx % len(PATTERNS)],
        pattern_index=pattern_idx,
        pattern_color_wheel=False,
        zone_colors=colors
    )
    payload = frame.pack()
    transfer.txBuff = list(payload)
    transfer.send(len(payload))


def main():
    parser = argparse.ArgumentParser(
        description="Mock Controller for IS Bed LCD - Send test frames via serial port"
    )
    parser.add_argument(
        '-p', '--port',
        help='Serial port'
    )
    parser.add_argument(
        '-b', '--baudrate',
        type=int,
        default=115200,
        help='Baud rate (default: 115200)'
    )
    
    args = parser.parse_args()
    
    print("="*60)
    print("IS Bed LCD Mock Controller")
    print("="*60)
    
    try:
        transfer = txfer.SerialTransfer(args.port, args.baudrate)
        transfer.open()
    except Exception as e:
        print(f"Error opening serial port: {e}") 
        sys.exit(1)

    print("Serial port opened successfully!")
    
    # Send pattern enumeration
    send_pattern_enumeration(transfer)
    
    #print(f"\nStarting demo mode (interval: {interval}s)...")
    #print("Press Ctrl+C to stop\n")
    #
    #preset_names = list(COLOR_PRESETS.keys())
    #pattern_idx = 0
    #preset_idx = 0
    #
    #try:
    #    while True:
    #        preset_name = preset_names[preset_idx]
    #        pattern_name = PATTERNS[pattern_idx]
    #        colors = COLOR_PRESETS[preset_name]
    #        
    #        print(f"Pattern: {pattern_name:12} | Colors: {preset_name:10} | "
    #              f"Zone colors: R{colors[0].r:3} G{colors[0].g:3} B{colors[0].b:3} ...")
    #        
    #        send_color_update(transfer, colors, pattern_idx)
    #        
    #        time.sleep(interval)
    #        
    #        # Cycle through patterns and presets
    #        preset_idx = (preset_idx + 1) % len(preset_names)
    #        if preset_idx == 0:
    #            pattern_idx = (pattern_idx + 1) % len(PATTERNS)
    #
    #except KeyboardInterrupt:
    #    print("\nDemo stopped.")
    
    transfer.close()
    print("Serial port closed.")

if __name__ == "__main__":
    main()


