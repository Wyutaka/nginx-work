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

const char ascii[128][3] = {
"00 ", "01 ", "02 ", "03 ", "04 ", "05 ", "06 ", "07 ", "08 ", "09 ", "0A ", "0B ", "0C ", "0D ", "0E ", "0F ", "10 ", 
"11 ", "12 ", "13 ", "14 ", "15 ", "16 ", "17 ", "18 ", "19 ", "1A ", "1B ", "1C ", "1D ", "1E ", "1F ", "20 ", "21 ", 
"22 ", "23 ", "24 ", "25 ", "26 ", "27 ", "28 ", "29 ", "2A ", "2B ", "2C ", "2D ", "2E ", "2F ", "30 ", "31 ", "32 ", 
"33 ", "34 ", "35 ", "36 ", "37 ", "38 ", "39 ", "3A ", "3B ", "3C ", "3D ", "3E ", "3F ", "40 ", "41 ", "42 ", "43 ", 
"44 ", "45 ", "46 ", "47 ", "48 ", "49 ", "4A ", "4B ", "4C ", "4D ", "4E ", "4F ", "50 ", "51 ", "52 ", "53 ", "54 ", 
"55 ", "56 ", "57 ", "58 ", "59 ", "5A ", "5B ", "5C ", "5D ", "5E ", "5F ", "60 ", "61 ", "62 ", "63 ", "64 ", "65 ", 
"66 ", "67 ", "68 ", "69 ", "6A ", "6B ", "6C ", "6D ", "6E ", "6F ", "70 ", "71 ", "72 ", "73 ", "74 ", "75 ", "76 ",
"77 ", "78 ", "79 ", "7A ", "7B ", "7C ", "7D ", "7E ", "7F "
};

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

    printf("%s", dmsg);	

	// for ( i = 0; i < 128; i++)
	// {
	// 	printf("\"%02X \", ", i);
	// }
	
    
}

void DumpHex(const void* data, char *dmsg, size_t size) {
	char *rdmsg =  (char *)calloc(size, sizeof(char));

    // printf("test %s\n", ascii['a']);
    // strncpy(dmsg, ascii['a'], 2);
	int sdmsg = 0; // sdmsg : dmsgの参照する位置 
    
	const int ol = 16;
	size_t i, j, k;
	for (i = 0; i < size; ++i) { 
		printf("%02X ", ((unsigned char*)data)[i]);
			rdmsg[i] = ((unsigned char*)data)[i];
    		strncpy(dmsg + sdmsg, ascii[((unsigned char*)data)[i]], sizeof(ascii[((unsigned char*)data)[i]]));
			sdmsg += sizeof(ascii[((unsigned char*)data)[i]]);
	
		if ((i+1) % 8 == 0 || i+1 == size) {
			printf(" ");  
    		strncpy(dmsg + sdmsg, " ", 1);
			sdmsg += 1;
			if ((i+1) % 16 == 0) {
			    char *str = (char *)calloc(ol, sizeof(char));
				int point = (((i+1)/16)-1) * 16; // nullの場合は別にする, (i+1)/16)-1)*16 その行の初めの位置
				for (k = 0; k < ol; k++)
				{
					if (((unsigned char*)data)[point + k] == 0x00)
					{
						str[point+ k] = 0x01;
					} else {
						str[point+ k] = ((unsigned char*)data)[point + k];
					}

			    	// strncpy(str + k, rdmsg + (((i+1)/16)-1)*16, 16); // (i+1)/16)-1)*16 その行の初めの位置 // そのままdmsgに代入すれば良いのでは？
				}
				
			    // strncpy(str, rdmsg + (((i+1)/16)-1)*16, 16);
				printf("|  %s \n", str);
    			strncpy(dmsg + sdmsg, "| ", 2);
				sdmsg += 2;
    			strncpy(dmsg + sdmsg, str, 16);
				sdmsg += 16;
    			strncpy(dmsg + sdmsg, " \n", 2);
				sdmsg += 2;
			} else if (i+1 == size) {   
				if ((i+1) % 16 <= 8) {
					printf(" ");  
    				strncpy(dmsg + sdmsg, " ", 1);
					sdmsg += 1;
				}
				for (j = (i+1) % 16; j < 16; ++j) { 
					printf("   ");
    				strncpy(dmsg + sdmsg, "   ", 3);
					sdmsg += 3;
				}

				
			    char *str = (char *)calloc((i+1) % 16, sizeof(char)); 
			    strncpy(str, rdmsg + size - ((i+1) % 16) , (i+1) % 16);
				printf("|  %s \n", str); 

				int point = size - (size % 16); // nullの場合は別にする, (i+1)/16)-1)*16 その行の初めの位置
				for (k = 0; k < (size) % 16; k++)
				{
					if (((unsigned char*)data)[point + k] == 0x00)
					{
						str[point+ k] = 0x01;
					} else {
						str[point+ k] = ((unsigned char*)data)[point + k];
					}
			    	// strncpy(str + k, rdmsg + (((i+1)/16)-1)*16, 16); // (i+1)/16)-1)*16 その行の初めの位置
				}
				// 
			    // strncpy(str, rdmsg + (((i+1)/16)-1)*16, 16);
				printf("|  %s \n", str);
    			strncpy(dmsg + sdmsg, "| ", 2);
				sdmsg += 2;
    			strncpy(dmsg + sdmsg, str, size % 16);
				sdmsg += size % 16;
    			strncpy(dmsg + sdmsg, " \n", 2);
				sdmsg += 2;

			}
		}
	}
}

