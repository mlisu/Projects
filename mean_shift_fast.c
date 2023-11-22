#include <math.h>
#include <time.h>
#define STB_IMAGE_IMPLEMENTATION
#include "libs/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "libs/stb_image_write.h"

#define MASK_R 10
#define SIGMA 20.0
#define SIGMA2_SQRD_INV 1/(2 * SIGMA * SIGMA )
#define STOP_RGB_DIFF 1.5
#define STOP_RGB_DIFF2 STOP_RGB_DIFF * STOP_RGB_DIFF
#define MAX_ITER 30
#define RGB 3

void adapt_window_to_frame(int *window_beg, int *window_end, float center, int size);

int main(){

float time;

int x_size, y_size, channels;
int y_original, x_original, center_x_idx, center_y_idx;
int window_beg_y, window_end_y, window_beg_x, window_end_x;
float center_x, center_y;
double center[3], prev_ctr[3];
double rgb_sum[3];
double weights_sum, x_sum, y_sum;
float weight;
int count = 0;
float x;
long pec = 0x9eb195c9;
long x_bin;
int i, j, k = 0;

unsigned char *img = stbi_load("../zdj_noise_400x400.png", &x_size, &y_size, &channels, 0);
unsigned char data[y_size][x_size][RGB];
unsigned char data_out[y_size][x_size][RGB];

for (i = 0; i < y_size; i++)
	for (j = 0; j < x_size; j++)
		for (k = 0; k < RGB; k++)
			data[i][j][k] = img[i * x_size * RGB + RGB * j + k];

clock_t start = clock();

for (y_original = 0; y_original < y_size; y_original++){

	for (x_original = 0; x_original < x_size; x_original++){

		center_x = x_original;
		center_y = y_original;
		center[0] = data[y_original][x_original][0];
		center[1] = data[y_original][x_original][1];
		center[2] = data[y_original][x_original][2];
		count = 0;

        	do{

			adapt_window_to_frame(&window_beg_y, &window_end_y, center_y, y_size);
			adapt_window_to_frame(&window_beg_x, &window_end_x, center_x, x_size);
            
			weights_sum = 0.0;
			rgb_sum[0] = 0.0;
			rgb_sum[1] = 0.0;
			rgb_sum[2] = 0.0;
			x_sum = 0.0;
			y_sum = 0.0;
            
			for (i = window_beg_y; i <= window_end_y; i++)
				for (j = window_beg_x; j <= window_end_x; j++){

					x = ((data[i][j][0] - center[0])*(data[i][j][0] - center[0])
					    +(data[i][j][1] - center[1])*(data[i][j][1] - center[1])
					    +(data[i][j][2] - center[2])*(data[i][j][2] - center[2]) )
					    * SIGMA2_SQRD_INV + 1;
					x_bin = *(long*)&x;
            				x_bin = pec - 3*(x_bin >> 1);
					weight = *(float*)&x_bin;
	
					weights_sum += weight;
					rgb_sum[0] += weight * data[i][j][0];
					rgb_sum[1] += weight * data[i][j][1];
					rgb_sum[2] += weight * data[i][j][2];
					y_sum += weight * (i - center_y);
                    			x_sum += weight * (j - center_x);
				}

			prev_ctr[0] = center[0];
			prev_ctr[1] = center[1];
			prev_ctr[2] = center[2];
			center[0] = rgb_sum[0] / weights_sum;
			center[1] = rgb_sum[1] / weights_sum;
			center[2] = rgb_sum[2] / weights_sum;
			center_x += x_sum / weights_sum;
            		center_y += y_sum / weights_sum;
            
           		 count++;

		} while((prev_ctr[0] - center[0])*(prev_ctr[0] - center[0])
		       +(prev_ctr[1] - center[1])*(prev_ctr[1] - center[1])
		       +(prev_ctr[2] - center[2])*(prev_ctr[2] - center[2]) > STOP_RGB_DIFF2
			&& count < MAX_ITER);

		data_out[y_original][x_original][0] = center[0] + 0.5; //rounding float to int
		data_out[y_original][x_original][1] = center[1] + 0.5;
		data_out[y_original][x_original][2] = center[2] + 0.5;
	}
}

stbi_write_png("zdj_noise_mean_fast.png", x_size, y_size, channels, data_out, x_size * channels);
printf("time: %f\n", (float)(clock() - start) / CLOCKS_PER_SEC);
}

void adapt_window_to_frame(int *window_beg, int *window_end, float center, int size){

	int coord_idx = (int)(center + 0.5);
	*window_beg = coord_idx - MASK_R;
	if (*window_beg < 0)
		*window_beg = 0;
	*window_end = coord_idx + MASK_R;
	if (*window_end >= size)
	*window_end = size - 1;

}
