from math import exp, floor, sqrt, log
from PIL import Image
import numpy as np
from time import time

MASK_SIZE = 11 #assumed to be odd
SIGMA = 20
M_SIGMA2_SQRD = -2 * SIGMA**2
STOP_RGB_DIFF = 1.5
# STOP_COORD_DIFF = 2
MAX_ITER = 30
CUT_IMG_SIZE = 70
RGB = 3

class Quick_Shift:
    def __init__(self, data: np.ndarray, sigma: float,
                 mask_r: int, parent_r: int, max_dist: int):
        """
        data:       e.g. np.asarray(image, dtype=float) where
                    image is output of the PIL.Image.open() function.
                    After quick_shift computation the data is the output data
        sigma:      smoothing coefficient in exp function
        mask_r:     radius of a window used to density computation.
                    It equals to floor(window_size) where windo_size is odd
                    E.g. if mask_r = 1, the (e.g. R (red) part of) window is
                    as follows (default R values):
                    [123  64  174
                    45  123  245
                    68  234   32] (Blue and Green parts of window are analogus)
        parent_r:   radius of a window used to parents search, analogous as above
        max_dist:   max dist btw pixels for them to be considered parent and
                    descendant. Both RGB and spatial coordinates are factored in.
        """
        self.data = data.astype('int')
        self.mask_r = mask_r
        self.parent_r = parent_r
        self.max_dist2 = max_dist**2
        self.y_size, self.x_size, _ = data.shape
        self.m_sigma2_sqrd_inv = -1 / (2 * sigma**2)
        self.sum_matrix = np.empty( (self.y_size, self.x_size), dtype=float )
        self.parents = np.empty( (self.y_size, self.x_size), dtype=int )
        self.unreach_dst2 = 3 * 255**2 + self.y_size**2 + self.x_size**2
        self.label = -1

    def adapt_window_to_frame(self, center: int, img_size: int, r: int) -> tuple:
        """
        The function cuts a window along one axis,
        so it does not stick out of image frame

        center:     pixel coordinate along one axis
        img_size:   number of image pixels along one axis
        r:          window radius
        """
        window_beg = center - r
        if window_beg < 0:
            window_beg = 0
        window_end = center + r
        if window_end >= img_size:
            window_end = img_size - 1
        
        return (window_beg, window_end)

    def density(self):
        for y_original in range(0, self.y_size):
            for x_original in range(0, self.x_size):
                # print(x_original)
                center = self.data[y_original, x_original]
                window_beg_y, window_end_y =\
                    self.adapt_window_to_frame(y_original, self.y_size, self.mask_r)
                window_beg_x, window_end_x =\
                    self.adapt_window_to_frame(x_original, self.x_size, self.mask_r)

                weights_sum = 0.0

                for i in range(window_beg_y, window_end_y + 1):
                    for j in range(window_beg_x, window_end_x + 1):
                            weights_sum += exp( ( (data[i,j,0] - center[0])**2
                                                 +(data[i,j,1] - center[1])**2
                                                 +(data[i,j,2] - center[2])**2
                                                 +(i - y_original)**2
                                                 +(j - x_original)**2 )
                                                 *self.m_sigma2_sqrd_inv )
                                    
                self.sum_matrix[y_original, x_original] = weights_sum

    def parents_search(self):
        for y_original in range(0, self.y_size):
            for x_original in range(0, self.x_size):

                center = self.data[y_original, x_original]
                window_beg_y, window_end_y =\
                    self.adapt_window_to_frame(y_original, self.y_size, self.parent_r)
                window_beg_x, window_end_x =\
                    self.adapt_window_to_frame(x_original, self.x_size, self.parent_r)
                
                d_best = self.unreach_dst2
                i_best = y_original
                j_best = x_original

                for i in range(window_beg_y, window_end_y + 1):
                    for j in range(window_beg_x, window_end_x + 1):
                        if self.sum_matrix[i,j] > self.sum_matrix[y_original, x_original]:

                            dist = (self.data[i,j,0] - center[0])**2\
                                  +(self.data[i,j,1] - center[1])**2\
                                  +(self.data[i,j,2] - center[2])**2\
                                  +(i - y_original)**2\
                                  +(j - x_original)**2
                            
                            if dist <= self.max_dist2 and dist < d_best:
                                d_best = dist
                                i_best = i
                                j_best = j
                
                dist = d_best
                if dist == self.unreach_dst2:
                    self.parents[y_original, x_original] = self.label
                    self.label -= 1
                else:
                    self.parents[y_original, x_original] =\
                        i_best * self.x_size + j_best

    def cluster(self):
        self.label = -self.label
        RGBs_of_labels = np.zeros( (self.label + 1, 3), dtype=float)
        pxs_of_labels = np.zeros(self.label + 1, dtype=int)

        for y_original in range(0, self.y_size):
            print("cluster: ", y_original)
            for x_original in range(0, self.x_size):
                i = y_original
                j = x_original
                desc_nr = 0
                descendants = []
                parent = self.parents[i,j]

                while parent >= 0:
                    descendants.append( (i,j) )
                    i = parent // self.x_size
                    j = parent % self.x_size
                    parent = self.parents[i,j]
                
                RGBs_of_labels[-parent] += self.data[y_original][x_original]

                for dsc in descendants:
                    self.parents[dsc] = parent

                pxs_of_labels[-parent] += 1
        
        for i in range(1, self.label):
            RGBs_of_labels[i] /= pxs_of_labels[i]

        for i in range(0, self.y_size):
            for j in range(0, self.x_size):
                self.data[i,j] = RGBs_of_labels[ -self.parents[i,j] ] + 0.5

    def quick_shift(self):
        self.density()
        self.parents_search()
        self.cluster()
        q.data = q.data.astype('uint8')

start = time()

IMG_PATH = '../zdj_noise_80x80.png'
SIGMA = 20.0
MASK_R = 10
PARENT_R = 10
MAX_DIST = 40

image = Image.open(IMG_PATH)
data = np.asarray(image)

q = Quick_Shift(data, SIGMA, MASK_R, PARENT_R, MAX_DIST)
q.quick_shift()
img_out = Image.fromarray(q.data)
img_out.save('py_quick.png')
print(q.label)

tm = time() - start
print(tm)
