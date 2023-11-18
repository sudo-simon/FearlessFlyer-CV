#include "core.hpp"
#include "bitmap.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>


using namespace std;


// ----------------------------------------------------------------------





p2b::Bitmap p2b::toBitmap(cv::Mat* img_ptr, uint8_t pixel_size, const std::vector<uint8_t>& thresholds_v, bool parallel){
    
    uint8_t pixels_per_byte = 8/pixel_size;

    size_t bm_rows = img_ptr->rows;
    size_t bm_cols = (img_ptr->cols + pixels_per_byte - 1)/pixels_per_byte;

    p2b::Bitmap ret_bm = Bitmap(bm_rows, bm_cols, pixel_size, thresholds_v);
    
    if (parallel) ret_bm.fromImage_parallel(img_ptr);
    else ret_bm.fromImage_linear(img_ptr);

    return ret_bm;

}








vector<vector<uint8_t>> p2b::toBits(cv::Mat* img_ptr, uint8_t pixel_size, const std::vector<uint8_t>& thresholds_v, bool parallel){
    
    uint8_t pixels_per_byte = 8/pixel_size;

    size_t bm_rows = img_ptr->rows;
    size_t bm_cols = (img_ptr->cols + pixels_per_byte - 1)/pixels_per_byte;

    p2b::Bitmap bm = Bitmap(bm_rows, bm_cols, pixel_size, thresholds_v);
    
    if (parallel) bm.fromImage_parallel(img_ptr);
    else bm.fromImage_linear(img_ptr);
    
    return bm.getVec();

}









int p2b::updateBitmap(Bitmap* bitmap_p, cv::Mat* update_img_ptr){
    return bitmap_p->updateFromImage(update_img_ptr);
}



int p2b::updateBitmapRegion(Bitmap *bitmap_ptr, cv::Mat *update_img_ptr, size_t start_row, size_t start_col){
    return bitmap_ptr->updateRegionFromImage(update_img_ptr, start_row, start_col);
}







int p2b::addBits(Bitmap* bitmap_p, cv::Mat* add_img_ptr, int add_direction, bool minimal_resizing){
    return bitmap_p->addImage(add_img_ptr, add_direction, minimal_resizing);
}