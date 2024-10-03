#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "c_img.h"
#include "c_img.c"

uint8_t calc_grad(struct rgb_img *im, int y, int x){                //this is a helper function that is used to calculate the gradient
    int width = im->width;
    int height = im->height;

    int drx = (get_pixel(im, y, ((x+1)%width), 0)) - (get_pixel(im, y, ((x-1+width)%width), 0));
    // printf("drx_x+1: %d\n",get_pixel(im, y, ((x+1)%width), 0));
    // printf("drx_x-1: %d\n",get_pixel(im, y, ((x-1+width)%width), 0));
    // printf("drx: %d\n",drx);
    int dgx = (get_pixel(im, y, ((x+1)%width), 1)) - (get_pixel(im, y, ((x-1+width)%width), 1));
    // printf("dgx_x+1: %d\n",get_pixel(im, y, ((x+1)%width), 1));
    // printf("dgx_x-1: %d\n",get_pixel(im, y, ((x-1+width)%width), 1));
    // printf("dgx: %d\n",dgx);
    int dbx = (get_pixel(im, y, ((x+1)%width), 2)) - (get_pixel(im, y, ((x-1+width)%width), 2));
    // printf("dbx_x+1: %d\n",get_pixel(im, y, ((x+1)%width), 2));
    // printf("dbx_x-1: %d\n",get_pixel(im, y, ((x-1+width)%width), 2));
    // printf("dbx: %d\n",dbx);

    int dry = (get_pixel(im, ((y+1)%height), x, 0)) - (get_pixel(im, ((y-1+height)%height), x, 0));
    // printf("dry: %d\n",dry);
    int dgy = (get_pixel(im, ((y+1)%height), x, 1)) - (get_pixel(im, ((y-1+height)%height), x, 1));
    // printf("dgy: %d\n",dgy);
    int dby = (get_pixel(im, ((y+1)%height), x, 2)) - (get_pixel(im, ((y-1+height)%height), x, 2));
    // printf("dby: %d\n",dby);

    int grad = (sqrt(drx*drx + dgx*dgx + dbx*dbx + dry*dry + dgy*dgy + dby*dby))/10;
    uint8_t GRAD = grad;
    //printf("%d",GRAD);
    return GRAD;
}

void calc_energy(struct rgb_img *im, struct rgb_img **grad)
{
    int width = im->width;
    int height = im->height;
    int tot_pixel = width *height;
    create_img(grad, height, width);

    for (int i = 0; i < tot_pixel; i++){
        int x = i%width;
        int y = i/width;
        ((*grad)->raster)[(i*3)+0] = (calc_grad(im, y, x));
        ((*grad)->raster)[(i*3)+1] = (calc_grad(im, y, x));
        ((*grad)->raster)[(i*3)+2] = (calc_grad(im, y, x));
    }
}

void dynamic_seam(struct rgb_img *grad, double **best_arr)
{
    int width = grad->width;
    int height = grad->height;
    int tot_pixel = width *height;      //this is basically the totaly number of pixels in the image
    *best_arr = malloc(tot_pixel * sizeof(double));

    int i;
    for (i = 0; i < tot_pixel; i++){     //for pixels less than the total number of pixels in the image
        int x = i%width;         //find the x and y coordinates of the pixel, this is similar to what we did in the previous function
        int y = i/width;
        if (y == 0){                  
            ((*best_arr)[i]) = (grad->raster)[(i*3)+0];
        }
        else{
            if (x == 0){
                ((*best_arr)[i]) = (grad->raster)[(i*3)] + fmin(((*best_arr)[i-width]), ((*best_arr)[i-width+1]));  
            }
            else if (x == width-1){
                ((*best_arr)[i]) = (grad->raster)[(i*3)] + fmin(((*best_arr)[i-width]), ((*best_arr)[i-width-1]));   //
            }
            else{
                ((*best_arr)[i]) = (grad->raster)[(i*3)] + fmin(((*best_arr)[i-width]), fmin(((*best_arr)[i-width-1]), ((*best_arr)[i-width+1])));  //min of the pixel above and the two pixels above and to the left and right
            }
        }
    }

}


void recover_path(double *best, int height, int width, int **path) {
    *path = malloc(height * sizeof(int)); // allocate memory for path array since we will be needing it

    int min_index = 0;
    int i;
    for (i = 1; i < width; i++) {  //i starts at 1 because we already set the min_index to 0
        if (best[(height-1)*width+i] < best[(height-1)*width+min_index]) {   //find the minimum value in the last row of the best array
            min_index = i;
        }
    }
    (*path)[height-1] = min_index;                            //set the last element of the path array to the minimum index

    int n;
    for (n = height-2; n >= 0; n--) {    //I start at height-2 because I already set the last element of the path array to the minimum index
        int j = (*path)[n+1];      
        int index = j;
        if (j != 0 && best[n*width+j-1] < best[n*width+index]) {     //if the index is not the first element in the row, then check the element to the left of the current element
            index = j - 1;
        }
        if (j != width-1 && best[n*width+j+1] < best[n*width+index]) {  //if the index is not the last element in the row, then check the element to the right of the current element
            index = j + 1; 
        }
        (*path)[n] = index;   //finally, set the path array to the index of the minimum value
    }
}


void remove_seam(struct rgb_img *src, struct rgb_img **dest, int *path) { 
    // The function creates the destination image, and writes to it the source image, with the seam removed.

    // get the height and width of the source image
    int height = src->height;
    int width = src->width;

    // create the destination image with the new width (width-1)
    create_img(dest, height, width-1);

    // loop over each row of the image
    int i;
    for (i = 0; i < height; i++) {
        // loop over each pixel in the row up to the seam
        int j;
        for (j = 0; j < path[i]; j++) {
            // set the pixel values in the destination image to the corresponding pixel in the source image
            set_pixel(*dest, i, j, get_pixel(src, i, j, 0), get_pixel(src, i, j, 1), get_pixel(src, i, j, 2));
        }
        // loop over each pixel in the row after the seam
        for (j = path[i]+1; j < width; j++) {
            // set the pixel values in the destination image to the corresponding pixel in the source image, shifted over by one column
            set_pixel(*dest, i, j-1, get_pixel(src, i, j, 0), get_pixel(src, i, j, 1), get_pixel(src, i, j, 2));
        }
    }
}



int main(){
    /*
    struct rgb_img *im;
    struct rgb_img *grad;
    read_in_img(&im, "6x5.bin");
    calc_energy(im,  &grad);
    print_grad(grad);
    double *best;   
    dynamic_seam(grad, &best);
    //print as a list of the values
    int i;
    printf("[");
    for (i = 0; i < 30; i++) {
        if (i == 29) {
            printf("%f", best[i]);
        } else {
            printf("%f, ", best[i]);
        }
        }
    printf("]");

    //test recover_path
    int *path;
    recover_path(best, 5, 6, &path);
    //print as a list of the values
    printf("[");
    for (i = 0; i < 5; i++) {
        if (i == 4) {
            printf("%d", path[i]);
        } else {
            printf("%d, ", path[i]);
        }
        }
    printf("]"); */

    struct rgb_img *im;
    struct rgb_img *cur_im;
    struct rgb_img *grad;
    double *best;
    int *path;

    read_in_img(&im, "HJoceanSmall.bin");
    
    for(int i = 0; i < 150; i++){
        //printf("i = %d\n", i);
        calc_energy(im,  &grad);
        dynamic_seam(grad, &best);
        recover_path(best, grad->height, grad->width, &path);
        remove_seam(im, &cur_im, path);
        /*
        char filename[200];
        sprintf(filename, "img%d.bin", i);
        write_img(cur_im, filename);

        */
        destroy_image(im);
        destroy_image(grad);
        free(best);
        free(path);
        im = cur_im;
    }
    char filename[200];
    sprintf(filename, "img%d.bin", 149);
    write_img(im, filename);

    destroy_image(im);
    destroy_image(grad);
    free(best);
    free(path);
}
    



    

