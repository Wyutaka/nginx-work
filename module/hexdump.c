#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE (6)

#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE (6)

void DumpHex(const void* data, size_t size);

int main(void)
{
    char *hoges;
    // char str1[ARRAY_SIZE] = "abc";
    char str2[] = {0x63, 0x61, 0x74, 0x00, 0x75, 0x63, 0x61, 0x74, 0x00, 0x75, 0x63, 0x61, 0x74, 0x00, 0x75, 0x63, 0x61, 0x74, 0x00, 0x75};
    
    hoges = (char *)calloc(ARRAY_SIZE, sizeof(char));

    int i,k = 0; // j is index of new char*
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (str2[i] != 0x00) {
            hoges[k] = str2[i];// 0x63, 0x61, 0x74, 0x74
            // printf("%c\n", hoges[k]);
            k++;
        }
    }

    hoges[k] = 0x00;
    char e[16];
    char *hoges2 = str2;
    
    DumpHex(hoges2, sizeof(str2));
    
}


void DumpHex(const void* data, size_t size) {
	char *rdmsg =  (char *)calloc(size, sizeof(char));
	const int ol = 16;
	size_t i, j;
	for (i = 0; i < size; ++i) { 
		printf("%02X ", ((unsigned char*)data)[i]);
		if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
			rdmsg[i] = ((unsigned char*)data)[i];
		} else {
			rdmsg[i] = '.'; 
		}
		if ((i+1) % 8 == 0 || i+1 == size) {
			printf(" ");  
			if ((i+1) % 16 == 0) {
			    char *str = (char *)calloc(ol, sizeof(char));
			    strncpy(str, rdmsg + (((i+1)/16)-1)*16, 16);
				printf("|  %s \n", str);
			} else if (i+1 == size) {   
				if ((i+1) % 16 <= 8) { 
					printf(" ");
				}
				for (j = (i+1) % 16; j < 16; ++j) { 
					printf("   ");
				}
			    char *str = (char *)calloc((i+1) % 16, sizeof(char)); 
			    strncpy(str, rdmsg + size - ((i+1) % 16) , (i+1) % 16);
				printf("|  %s \n", str); 
			}
		}
	}
}

