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
import colorsys
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
    pattern_type: int
    zone_colors: List[ColorRGB]

    def pack(self) -> bytes:
        """Pack the frame into bytes for serial transmission"""
        # Ensure pattern name is exactly 16 bytes
        name_bytes = self.pattern_name.encode('ascii')[:PATTERN_NAME_SIZE]
        name_bytes = name_bytes.ljust(PATTERN_NAME_SIZE, b'\x00')
        
        # Pack the structure
        data = struct.pack('B', self.pattern_index)
        data += name_bytes
        data += struct.pack('B', self.pattern_type)
        
        # Pack all zone colors (RGB)
        for color in self.zone_colors:
            data += struct.pack('BBB', color.r, color.g, color.b)
        
        return data


@dataclass
class LcdToControllerFrame:
    """Frame received from LCD to controller"""
    selected_pattern_index: int
    displayed_pattern_index: int
    zone_brightnesses: List[int]  # 4 zones, 0-100
    selected_color: ColorRGB
    frequency: int = 0  # Frerquency in Hz * 10

    @staticmethod
    def unpack(data: bytes) -> 'LcdToControllerFrame':
        """Unpack bytes into a frame structure"""
        if len(data) != 10:
            raise ValueError(f"Invalid frame size: {len(data)} bytes")
        
        selected_pattern_index = struct.unpack('B', data[0:1])[0]
        displayed_pattern_index = struct.unpack('B', data[1:2])[0]
        zone_brightnesses = list(struct.unpack('BBBB', data[2:6]))
        r, g, b = struct.unpack('BBB', data[6:9])
        selected_color = ColorRGB(r, g, b)
        frequency = struct.unpack('B', data[9:10])[0]
        
        return LcdToControllerFrame(
            selected_pattern_index=selected_pattern_index,
            displayed_pattern_index=displayed_pattern_index,
            zone_brightnesses=zone_brightnesses,
            selected_color=selected_color,
            frequency=frequency
        )



# Predefined patterns for testing
PATTERNS = [
    "Rotate",
    "Static",
    "Strobe",
    "Blink",
    "Rainbow",
    "Fire",
    "Ocean",
    "Forest",
    "Sunset",
    "Aurora",
]

def hsv_to_rgb(h: float, s: float = 1.0, v: float = 1.0) -> ColorRGB:
    """Convert HSV color to RGB (0-255 range)"""
    r, g, b = colorsys.hsv_to_rgb(h, s, v)
    return ColorRGB(int(r * 255), int(g * 255), int(b * 255))


def receive_lcd_status(transfer: txfer.SerialTransfer) -> None:
    """Check for and display messages from the LCD"""
    if transfer.available() == 10:
        # Extract the data
        data = bytes(transfer.rx_buff[:10])
        try:
            frame = LcdToControllerFrame.unpack(data)
            print(f"\n{'='*60}")
            print(f"LCD Status Received:")
            print(f"  Selected Pattern Index: {frame.selected_pattern_index} ({PATTERNS[frame.selected_pattern_index] if frame.selected_pattern_index < len(PATTERNS) else 'Unknown'})")
            print(f"  Displayed Pattern Index: {frame.displayed_pattern_index} ({PATTERNS[frame.displayed_pattern_index] if frame.displayed_pattern_index < len(PATTERNS) else 'Unknown'})")
            print(f"  Zone Brightnesses:")
            zone_names = ["Cage", "Center", "Front", "Headboard"]
            for i, brightness in enumerate(frame.zone_brightnesses):
                print(f"    {zone_names[i]:12s}: {brightness:3d}%")
            print(f"  Selected Color: RGB({frame.selected_color.r}, {frame.selected_color.g}, {frame.selected_color.b})")
            print(f"  Frequency: {frame.frequency / 10.0} Hz")
            print(f"{'='*60}\n")
        except Exception as e:
            print(f"Error unpacking LCD frame: {e}")


def continuous_update_loop(transfer: txfer.SerialTransfer, update_rate: float = 0.1) -> None:
    """Continuously send updates and receive status from LCD"""
    print("\n" + "="*60)
    print("Starting continuous update loop...")
    print("Press Ctrl+C to stop")
    print("="*60 + "\n")
    
    pattern_idx = 0
    hue = 0.0  # HSV hue (0.0 to 1.0)
    hue_step = 0.005  # Small step for smooth color transitions
    
    try:
        while True:
            # Generate smooth color gradient
            color = hsv_to_rgb(hue)
            
            # Create zone colors with slight variations
            zone_colors = [
                hsv_to_rgb((hue + 0.0) % 1.0),  # Cage
                hsv_to_rgb((hue + 0.25) % 1.0), # Center
                hsv_to_rgb((hue + 0.5) % 1.0),  # Front
                hsv_to_rgb((hue + 0.75) % 1.0), # Headboard
            ]
            
            # Send update to LCD
            frame = ControllerToLcdFrame(
                pattern_name=PATTERNS[pattern_idx],
                pattern_index=pattern_idx,
                pattern_type= pattern_idx if (pattern_idx == 1 or pattern_idx == 2) else 6,
                zone_colors=zone_colors
            )
            payload = frame.pack()
            transfer.tx_buff = payload
            transfer.send(len(payload))
            
            # Update hue for smooth color cycling
            hue = (hue + hue_step) % 1.0

            # Progress indicator
            print(f"Sending pattern {pattern_idx} ({PATTERNS[pattern_idx]}) ({frame.pattern_type}), hue: {hue:.2f}")
            
            # Check for incoming messages from LCD
            receive_lcd_status(transfer)
            
            # Cycle through patterns periodically
            pattern_idx = (pattern_idx + 1) % len(PATTERNS)
            
            time.sleep(update_rate)
            
    except KeyboardInterrupt:
        print("\n\nStopping continuous update loop...")



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
    parser.add_argument(
        '-r', '--rate',
        type=float,
        default=0.1,
        help='Update rate in seconds (default: 0.1)'
    )
    parser.add_argument(
        '--enumerate-only',
        action='store_true',
        help='Only send pattern enumeration and exit'
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
    #send_pattern_enumeration(transfer)
    
    if not args.enumerate_only:
        # Start continuous update loop
        continuous_update_loop(transfer, args.rate)

    transfer.close()
    print("Serial port closed.")

if __name__ == "__main__":
    main()



