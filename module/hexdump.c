/*!
 * dumphex.c
 *
 * written by www
 * 
 * 与えられたテキストのダンプを返す関数の実装.
 * やること
 * 文字列ポインタを与え，そのポインタにダンプ情報を書き込む．
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE (6)

void DumpHex(const void* data,  char *msg, size_t size);

int main(void)
{
    char *hoges;
    // char str1[ARRAY_SIZE] = "abc";
    char str2[] = {0x63, 0x61, 0x74, 0x00, 0x75, 0x63, 0x61, 0x74, 0x00, 0x75, 0x63, 0x61, 0x74, 0x00, 0x75, 0x63, 0x61, 0x74, 0x00 };
    
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
    char *dmsg;

    dmsg = (char *)calloc(sizeof(str2), sizeof(char));
    
    DumpHex(hoges2, dmsg, sizeof(str2));

    printf("dmsg is : %s", dmsg);
    
}

void DumpHex(const void* data, char *dmsg, size_t size) {
    char ascii[129][2];
    ascii['a'][0] = '6';
    ascii['a'][1] = '3';
    ascii['b'][0] = '6';
    ascii['b'][1] = '4';
	char *rdmsg =  (char *)calloc(size, sizeof(char));

    printf("test %s\n", ascii['a']);
    strncpy(dmsg, ascii['a'], 2);
    
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

