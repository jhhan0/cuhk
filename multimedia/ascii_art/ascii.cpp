/*
* CSCI3280 Introduction to Multimedia Systems 
* --- Declaration --- 
* I declare that the assignment here submitted is original except for source
* material explicitly acknowledged. I also acknowledge that I am aware of
* University policy and regulations on honesty in academic work, and of the
* disciplinary guidelines and procedures applicable to breaches of such policy
* and regulations, as contained in the website
* http://www.cuhk.edu.hk/policy/academichonesty/ 
* Assignment 1
* Name : HAN, Jihun
* Student ID : 1155128719
* Email Addr : 1155128719@link.cuhk.edu.hk
*/


#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "malloc.h"
#include "memory.h"
#include "math.h"
#include "bmp.h"


#define MAX_SHADES 8

char shades[MAX_SHADES] = { '@','#','%','*','|','-','.',' ' };

#define SAFE_FREE(p)  { if(p){ free(p);  (p)=NULL;} }


int main(int argc, char** argv)
{
	//
	//	1. Open BMP file
	//
	Bitmap image_data(argv[2]);

	if (image_data.getData() == NULL)
	{
		printf("unable to load bmp image!\n");
		return -1;
	}

	//
	//	2. Obtain Luminance
	//
	int width = image_data.getWidth();
	int height = image_data.getHeight();
	unsigned char red, green, blue;
	unsigned char* gray = (unsigned char*)malloc((width * height) * sizeof(256));

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			image_data.getColor(x, y, red, green, blue);
			gray[x + y * width] = (int)(0.299 * red + 0.587 * green + 0.114 * blue);
		}
	}

	//
	//  3. Resize image
	//
	int resized_width, resized_height;
	int* resized_image;

	if (argc > 2)
	{
		char* ptr, * context;
		ptr = strtok_s(argv[3], ",", &context);
		int target_width = atoi(ptr);
		ptr = strtok_s(NULL, ",", &context);
		int target_height = atoi(ptr);
		if (target_height == 0) {
			target_height = target_width;
		}

		double block_width = ceil((double)width / target_width);
		double block_height = ceil((double)height / target_height);

		resized_width = (int)round((double)width / block_width);
		resized_height = (int)round((double)height / block_height);

		resized_image = (int*)malloc(resized_width * resized_height * sizeof(int));

		for (int y = 0; y < resized_height; y++) {
			for (int x = 0; x < resized_width; x++) {
				resized_image[x + y * resized_width] = 0;
				for (int j = 0; j < block_height; j++) {
					for (int i = 0; i < block_width; i++) {
						resized_image[x + y * resized_width] += gray[(int)(x * block_width) + i + (int)(y * block_height * width) + (j * width)];
					}
				}
				resized_image[x + y * resized_width] /= (block_height * block_width);
			}
		}
	}

	//
	//	4. Quantization
	//
	for (int y = 0; y < resized_height; y++) {
		for (int x = 0; x < resized_width; x++) {
			resized_image[x + y * resized_width] /= (256 / 8);
		}
	}

	//
	//  5. ASCII Mapping and printout
	//
	for (int y = 0; y < resized_height; y++) {
		for (int x = 0; x < resized_width; x++) {
			if (argv[1][0] == 'p')
			{
				printf("%c", shades[resized_image[x + y * resized_width]]);
			}
			else if (argv[1][0] == 's')
			{
				printf("%c", shades[8 - resized_image[x + y * resized_width]]);
			}
		}
		printf("\n");
	}

	//
	//  6. ASCII art txt file
	//
	if (argc > 3)
	{
		FILE* foutput = fopen(argv[4], "w");
		for (int y = 0; y < resized_height; y++) {
			for (int x = 0; x < resized_width; x++) {
				if (argv[1][0] == 'p') {
					fputc(shades[resized_image[x + y * resized_width]], foutput);
				}
				else if (argv[1][0] == 's') {
					fputc(shades[7 - resized_image[x + y * resized_width]], foutput);
				}
			}
			fputc('\n', foutput);
		}
		fclose(foutput);
		free(foutput);
	}

	//
	//  Free memory
	//
	SAFE_FREE(gray)
	SAFE_FREE(resized_image)
	
	return 0;
}
