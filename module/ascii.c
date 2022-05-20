#include <stdio.h>

#define COUNT 2

int main(void) {
    char tmp[COUNT + 1] = {0x00, 0x42};
    for (int i = 0; i < COUNT; i++) {
        if (tmp[i] == 0x00) {
            tmp[i] = 0x2D;
        }
    }
    tmp[COUNT] = 0x00;
    char buf[COUNT * 2];
    for (int i = 0; i < COUNT; i++) {
        sprintf(buf + i * 2, "%X", tmp[i]);
    }
    printf("%s | %s\n", buf, tmp);
    return 0;
}

