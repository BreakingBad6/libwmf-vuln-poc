/*
 * POC for Issue #11: ReadBMPImage Integer Overflow
 * CRASH VERSION - Will trigger ASAN
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} wmfRGB;

typedef struct {
    uint32_t size;
    uint32_t number_colors;
} BMPInfo;

typedef struct {
    wmfRGB* rgb;
    uint32_t NColors;
} wmfBMPData;

int ReadBMPImage_vulnerable(BMPInfo bmp_info) {
    wmfBMPData data;
    uint32_t i;

    printf("[*] BMP Info Header:\n");
    printf("    size: %u\n", bmp_info.size);
    printf("    number_colors: %u\n", bmp_info.number_colors);

    // No validation of number_colors!
    data.NColors = bmp_info.number_colors;

    printf("\n[*] Allocating color table:\n");
    printf("    NColors: %u\n", data.NColors);
    printf("    sizeof(wmfRGB): %zu\n", sizeof(wmfRGB));

    size_t alloc_size = (size_t)data.NColors * sizeof(wmfRGB);
    printf("    Requested size: %zu bytes\n", alloc_size);

    data.rgb = (wmfRGB*)malloc(alloc_size);
    if (!data.rgb) {
        printf("[-] Allocation failed\n");
        return -1;
    }

    printf("[+] Allocated %zu bytes at %p\n", alloc_size, (void*)data.rgb);

    // Vulnerable loop - writes beyond allocated buffer
    printf("\n[*] Writing to color table (will overflow):\n");

    // Write more entries than allocated
    uint32_t write_count = data.NColors + 100;  // Intentionally overflow
    printf("[!] Attempting to write %u entries (allocated for %u)\n",
           write_count, data.NColors);
    printf("[!] This will overflow by %u entries (%zu bytes)\n",
           100, 100 * sizeof(wmfRGB));
    printf("[!] Triggering crash...\n\n");

    for (i = 0; i < write_count; i++) {
        data.rgb[i].r = i & 0xFF;
        data.rgb[i].g = (i >> 8) & 0xFF;
        data.rgb[i].b = (i >> 16) & 0xFF;

        if (i >= data.NColors) {
            printf("[!] Writing entry %u (OUT OF BOUNDS!)\n", i);
            if (i >= data.NColors + 5) {
                break;  // Stop after a few overflow writes
            }
        }
    }

    free(data.rgb);
    return 0;
}

int main() {
    printf("[*] POC for Issue #11: ReadBMPImage Integer Overflow\n");
    printf("[*] CRASH VERSION - Will trigger ASAN\n\n");

    BMPInfo bmp_info;

    // Test with small allocation but large write
    printf("=== Test: Small allocation with overflow writes ===\n");
    bmp_info.size = 40;
    bmp_info.number_colors = 10;  // Allocate for 10 colors only

    ReadBMPImage_vulnerable(bmp_info);

    printf("\n[!] ERROR: Should have crashed but didn't!\n");
    return 0;
}
