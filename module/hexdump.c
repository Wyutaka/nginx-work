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
	"77 ", "78 ", "79 ", "7A ", "7B ", "7C ", "7D ", "7E ", "7F "};

void DumpHex(const void *data, size_t size);

int main(void)
{
	char *hoges;
	// char str1[ARRAY_SIZE] = "abc";
	char str2[] = {0x38, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x75, 0x63, 0x61, 0x74, 0x00, 0x75, 0x63, 0x61, 0x74, 0x00};

	hoges = (char *)calloc(ARRAY_SIZE, sizeof(char));

	int i, k = 0; // j is index of new char*
	for (i = 0; i < ARRAY_SIZE; i++)
	{
		if (str2[i] != 0x00)
		{
			hoges[k] = str2[i]; // 0x63, 0x61, 0x74, 0x74
			// printf("%c\n", hoges[k]);
			k++;
		}
	}

	hoges[k] = 0x00;
	char e[16];
	char *hoges2 = str2;
	char *dmsg;

	dmsg = (char *)calloc(sizeof(str2), sizeof(char) * 5);

	DumpHex(hoges2, sizeof(str2));
	// printf("%s", dmsg);

	// for ( i = 0; i < 128; i++)
	// {
	// 	printf("\"%02X \", ", i);
	// }
}


// 16回毎に1回プリント
// すべて0x00の場合は無視


void DumpHex(const void *data, size_t size)
{
	int ls = size / 16;

	size_t i, j, k, l;
	for (l = 0; l < ls; l++)
	{
		char rdmsg[68];
		int sdmsg = 0; 
		int allnull = 0;
		for (i = 0; i < 16; ++i)
		{
			strncpy(rdmsg + sdmsg, ascii[((unsigned char *)data)[i + (l * 16)]], sizeof(ascii[0]));
			sdmsg += sizeof(ascii[0]);

			if ((i + 1) % 8 == 0)
			{
				strncpy(rdmsg + sdmsg, " ", 1);
				sdmsg += 1;
			}

			if (i + 1 == 16)
			{
				char *str = (char *)calloc(16, sizeof(char));
				strncpy(str, data + (l * 16), 16);
				for (k = 0; k < 16; k++)
				{
					if (((unsigned char *)data)[k + (l * 16)] == 0x00)
					{
						str[k] = 0x20;
					}
					else
					{
						allnull = 1;
						str[k] = ((unsigned char *)data)[k + (l * 16)];
					}
				}
				strncpy(rdmsg + sdmsg, "|  ", 3);
				sdmsg += 3;
				strncpy(rdmsg + sdmsg, str, 16);
				sdmsg += 16;
				strncpy(rdmsg + sdmsg, " \n", 2);
				sdmsg += 1;
			}
		}
		if (allnull != 0) printf("%s", rdmsg);
	}

	if (size % 16 > 0)
	{	
		char rdmsg[68];
		int sdmsg = 0; // sdmsg : dmsgの参照する位置
		int allnull = 0;
		for (i = 0; i < size % 16; i++)
		{
			strncpy(rdmsg + sdmsg, ascii[((unsigned char *)data)[i + (l * 16)]], sizeof(ascii[0]));
			sdmsg += sizeof(ascii[0]);

			if ((i + 1) % 8 == 0)
			{
				strncpy(rdmsg + sdmsg, " ", 1);
				sdmsg += 1;
			}
			if (i + 1 == size % 16)
			{
				// あまりの実装
				if (size % 16 <= 8)
				{
					strncpy(rdmsg + sdmsg, "  ", 2);
					sdmsg += 2;
				}
				for (j = (i + 1) % 16; j < 16; ++j)
				{
					strncpy(rdmsg + sdmsg, "   ", 3);
					sdmsg += 3;
				}
				
				char *str = (char *)calloc(size % 16, sizeof(char));
				strncpy(str, data + (l * 16), size % 16);
				for (k = 0; k < size % 16; k++)
				{
					if (((unsigned char *)data)[k + (l * 16)] == 0x00)
					{
						str[k] = 0x20;
					}
					else
					{
						str[k] = ((unsigned char *)data)[k + (l * 16)];
					}
				}
				strncpy(rdmsg + sdmsg, "|  ", 3);
				sdmsg += 3;
				strncpy(rdmsg + sdmsg, str, size % 16);
				sdmsg += size % 16;
				strncpy(rdmsg + sdmsg, " \n\0", 3);
				sdmsg += 1;
			}
		}
		if (allnull != 0) printf("%s", rdmsg);
	}
}
