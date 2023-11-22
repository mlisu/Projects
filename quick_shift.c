#include <math.h>
#include <time.h>
#define STB_IMAGE_IMPLEMENTATION
#include "libs/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "libs/stb_image_write.h"
#include <stdlib.h>

#define MASK_R 10 // mask radius
#define PARENT_R 10 // mask radius by parents assignment
#define MAX_DIST 40 // max dist btw pixels for them to be considered parent and descendant
#define MAX_DIST_2 MAX_DIST * MAX_DIST
#define SIGMA 20
#define M_SIGMA2_SQRD_INV -1/(2 * SIGMA * SIGMA )
#define RGB 3

void adapt_window_to_frame(int *window_beg, int *window_end, int center, int size, int mask_R);

int main(){

float time;
int i, j, k = 0;

// load image:
int x_size, y_size, channels;
unsigned char *img = stbi_load("../zdj_noise_400x400.png", &x_size, &y_size, &channels, 0);
unsigned char data[y_size][x_size][RGB];

// variables to weight computation:
double sum_matrix[y_size][x_size];
double center[3];
double weights_sum;

// variables to parents assignment:
double unreach_dst2 = 255*255 + 255*255 + 255*255 + x_size + y_size; // max spacial distance squared is x_size-1 + y_size-1
int parents[y_size][x_size];
double dist;
double d_best = unreach_dst2;
double i_best;
double j_best;

// variables to clustering in superpixels
    // RGB values of labels:
double *Rs_of_labels = calloc(y_size*x_size + 1, sizeof(double)); //+ 1, cause labels starts from -1
double *Gs_of_labels = calloc(y_size*x_size + 1, sizeof(double));
double *Bs_of_labels = calloc(y_size*x_size + 1, sizeof(double));
    // number of pixels assigned to every labels:
int *pxs_of_labels = calloc(y_size*x_size + 1, sizeof(int));

int **descendants = malloc(y_size*x_size*sizeof(int*));
int *parent_ptr;
int label;
int desc_nr;

// window variables:
int y_original, x_original;
int window_beg_y, window_end_y, window_beg_x, window_end_x;

for (i = 0; i < y_size; i++)
	for (j = 0; j < x_size; j++)
		for (k = 0; k < RGB; k++)
			data[i][j][k] = img[i * x_size * RGB + RGB * j + k];

clock_t start = clock();

for (y_original = 0; y_original < y_size; y_original++){
	for (x_original = 0; x_original < x_size; x_original++){

	        center[0] = data[y_original][x_original][0];
        	center[1] = data[y_original][x_original][1];
        	center[2] = data[y_original][x_original][2];
        
       		adapt_window_to_frame(&window_beg_y, &window_end_y, y_original, y_size, MASK_R);
		adapt_window_to_frame(&window_beg_x, &window_end_x, x_original, x_size, MASK_R);
    
        	weights_sum = 0.0;
   
		for (i = window_beg_y; i <= window_end_y; i++)
			for (j = window_beg_x; j <= window_end_x; j++)

				weights_sum += exp(((data[i][j][0] - center[0])*(data[i][j][0] - center[0])
					    + (data[i][j][1] - center[1])*(data[i][j][1] - center[1])
					    + (data[i][j][2] - center[2])*(data[i][j][2] - center[2])
					    + (y_original - i)*(y_original - i)
					    + (x_original - j)*(x_original - j) )
					    * M_SIGMA2_SQRD_INV);

        sum_matrix[y_original][x_original] = weights_sum;
	}
}
label = -1;
for (y_original = 0; y_original < y_size; y_original++){
	for (x_original = 0; x_original < x_size; x_original++){

       		adapt_window_to_frame(&window_beg_y, &window_end_y, y_original, y_size, PARENT_R);
		adapt_window_to_frame(&window_beg_x, &window_end_x, x_original, x_size, PARENT_R);

		d_best = unreach_dst2;
		i_best = y_original;
		j_best = x_original;

		for (i = window_beg_y; i <= window_end_y; i++)
		    for (j = window_beg_x; j <= window_end_x; j++)
			if (sum_matrix[i][j] > sum_matrix[y_original][x_original]){

			    dist =  (data[i][j][0] - data[y_original][x_original][0]) *
				    (data[i][j][0] - data[y_original][x_original][0]) +
				    (data[i][j][1] - data[y_original][x_original][1]) *
				    (data[i][j][1] - data[y_original][x_original][1]) +
				    (data[i][j][2] - data[y_original][x_original][2]) *
				    (data[i][j][2] - data[y_original][x_original][2]) +
				    (y_original - i)*(y_original - i) +
				    (x_original - j)*(x_original - j);

			    if (dist <= MAX_DIST_2 && dist < d_best){
				d_best = dist;
				i_best = i;
				j_best = j;
			    }
			}
        dist = d_best;
        parents[y_original][x_original] = 
            dist == unreach_dst2 ? label-- : i_best * x_size + j_best;

    }
}
for (y_original = 0; y_original < y_size; y_original++)
	for (x_original = 0; x_original < x_size; x_original++){

		i = y_original;
		j = x_original;
		desc_nr = 0;

		while ( *(parent_ptr = parents[i] + j) >= 0 ){

		    descendants[desc_nr++] = parent_ptr;
		    i = *parent_ptr / x_size;
		    j = *parent_ptr % x_size;
		}

		Rs_of_labels[-*parent_ptr] += data[y_original][x_original][0];
		Gs_of_labels[-*parent_ptr] += data[y_original][x_original][1];
		Bs_of_labels[-*parent_ptr] += data[y_original][x_original][2];

		for (k = 0; k < desc_nr; k++)
		    *descendants[k] = *parent_ptr;

		pxs_of_labels[-*parent_ptr]++;  
	}

label = -label;

for (i = 1; i < label; i++){
        Rs_of_labels[i] /= pxs_of_labels[i];
        Gs_of_labels[i] /= pxs_of_labels[i];
        Bs_of_labels[i] /= pxs_of_labels[i];
}

for (i = 0; i < y_size; i++)
	for (j = 0; j < x_size; j++){

		data[i][j][0] = Rs_of_labels[ -parents[i][j] ];
		data[i][j][1] = Gs_of_labels[ -parents[i][j] ];
		data[i][j][2] = Bs_of_labels[ -parents[i][j] ];
    	}

stbi_write_png("zdj_noise_quick7.png", x_size, y_size, channels, data, x_size * channels);
printf("time: %f\n", (float)(clock() - start) / CLOCKS_PER_SEC);
printf("labels: %d\n", label);
}

void adapt_window_to_frame(int *window_beg, int *window_end, int center, int size, int mask_R){

	*window_beg = center - mask_R;
	if (*window_beg < 0)
		*window_beg = 0;
	*window_end = center + mask_R;
	if (*window_end >= size)
	*window_end = size - 1;
	
}
