#include "bitmap.hpp"
#include "core.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/highgui.hpp>
#include <vector>
#include <opencv2/imgproc.hpp>



using namespace std;


// ----------------------------------------------------------------------



p2b::Bitmap::Bitmap(){
    this->rows = 1;
    this->cols = 1;
    this->pixel_size = 2;
    this->pixels_per_byte = 4;
    this->pixel_values = 3;
    this->thresholds_v = {125};
    this->last_add_r0 = -1;
    this->last_add_c0 = -1;
    this->last_add_height = -1;
    this->last_add_width = -1;
    this->vec = vector<vector<uint8_t>>(1,vector<uint8_t>(1));
}





p2b::Bitmap::Bitmap(long rows, long cols, uint8_t pixel_size, const vector<uint8_t>& thresholds_v){
    
    if (rows <= 0 || cols <= 0){
        ERROR_MSG("rows and cols are not both positive values");
        exit(1);
    }
    if ((pixel_size < 1) || (pixel_size > 4) || (pixel_size & (pixel_size-1)) != 0){    //! Maybe I could just check the 3 possible values...
        ERROR_MSG("pixel_size is not one of {1, 2, 4}");
        exit(1);
    }
    if (thresholds_v.size() != ((size_t) (1 << pixel_size) - 1)){
        ERROR_MSG("thresholds_v is not a vector of 8/pixel_size thresholds to apply");
        exit(1);
    }
    if (! is_sorted(thresholds_v.begin(), thresholds_v.end())){
        ERROR_MSG("thresholds_v is not sorted in ascending order");
        exit(1);
    }
    for (const uint8_t& threshold : thresholds_v){
        if (threshold < 0 || threshold > 255){
            ERROR_MSG("every threshold in thresholds_v must be between 0 and 255");
            exit(1);
        }
    }
    
    this->rows = rows;
    this->cols = cols;
    this->pixel_size = pixel_size;
    this->pixels_per_byte = 8/pixel_size;
    this->pixel_values = (1 << pixel_size) -1;
    this->thresholds_v = thresholds_v;

    this->last_add_r0 = -1;
    this->last_add_c0 = -1;
    this->last_add_height = -1;
    this->last_add_width = -1;

    this->vec = vector<vector<uint8_t>>(rows, vector<uint8_t>(cols,255));

}

p2b::Bitmap::~Bitmap(){
    this->thresholds_v.clear();
    this->vec.clear();
}





long p2b::Bitmap::getRows(){ return this->rows; }
long p2b::Bitmap::getCols(){ return this->cols; }
uint8_t p2b::Bitmap::getPixelSize(){ return this->pixel_size; }
uint8_t p2b::Bitmap::getPixelValues(){ return this->pixel_values; }
vector<uint8_t> p2b::Bitmap::getThresholds(){ return this->thresholds_v; }
vector<vector<uint8_t>> p2b::Bitmap::getVec(){ return this->vec; }





int p2b::Bitmap::increaseSize(const long new_rows, const long new_cols, const int resize_direction){
    if ((new_rows < this->rows) || (new_cols < this->cols)){
        ERROR_MSG("new_rows and new_cols must be greater than the existing rows and cols");
        return 1;
    }

    this->vec.resize(new_rows);
    for (vector<uint8_t>& row_v : this->vec){
        row_v.resize(new_cols);
    }

    for (long i=0; i<new_rows; ++i){
        for (long j=0; j<new_cols; ++j){
            if (i >= this->rows || j >= this->cols)
                this->vec[i][j] = 255;  //? Byte set to 0b11111111
        }
    }

    //? Moving original image if it has to be displaced
    uint8_t tmp_swap;
    long row_diff = new_rows - this->rows;
    long col_diff = new_cols - this->cols;

    switch (resize_direction) {

        case p2b::DIR_UP:
            for (long i=this->rows-1; i>=0; --i){
                for (long j=this->cols-1; j>=0; --j){
                    if (this->vec[i][j] != 255){
                        tmp_swap = this->vec[i][j];
                        this->vec[i][j] = this->vec[i+row_diff][j];
                        this->vec[i+row_diff][j] = tmp_swap;
                    }
                }
            }
            this->last_add_r0 += row_diff;   //? Now the r0 of the last image is increased
            break;
        
        case p2b::DIR_LEFT:
            for (long i=this->rows-1; i>=0; --i){
                for (long j=this->cols-1; j>=0; --j){
                    if (this->vec[i][j] != 255){
                        tmp_swap = this->vec[i][j];
                        this->vec[i][j] = this->vec[i][j+col_diff];
                        this->vec[i][j+col_diff] = tmp_swap; 
                    }
                }
            }
            this->last_add_c0 += col_diff;  //? Now the c0 of the last image is increased
            break;
    
    }

    this->rows = new_rows;
    this->cols = new_cols;
    return 0;
}




/*
    Wrapper for increaseSize(rows*2, cols*2, resize_direction)
*/
int p2b::Bitmap::doubleSize(const int resize_direction){
    return this->increaseSize(this->rows*2, this->cols*2, resize_direction);
}













int p2b::Bitmap::fromImage_linear(cv::Mat* img_ptr){

    size_t img_rows = img_ptr->rows;
    size_t img_cols = img_ptr->cols;
    cv::Mat gs_img = *img_ptr;
    if (img_ptr->channels() > 1){
        cv::cvtColor(*img_ptr, gs_img, cv::COLOR_BGR2GRAY);
    }

    size_t vec_j;
    uint8_t p_value;
    uint8_t l_shift;
    

    for (size_t i=0; i<img_rows; ++i){
        for (size_t j=0; j<img_cols; ++j){

            vec_j = j/this->pixels_per_byte;
            
            
            //? Because of how this piece of code works, if we want to keep the 1, 11, 1111 values reserved
            //? we have to ensure that the last threshold value is 255 for pixel_size != 1
            
            p_value = 0;
            for (uint8_t& threshold : this->thresholds_v){
                if (gs_img.at<uint8_t>(i,j) < threshold)
                    break;
                ++p_value;
            }

            //? Critical calculation of the left shift
            l_shift = (8-this->pixel_size) - ((j%this->pixels_per_byte) * this->pixel_size);

            //? This algorithm that performs bitwise operations only works
            //? if the pixels of the bitmap we're accessing are initialized
            //? in sequential order, like in this case, since the biwise OR
            //? operations together with the &= could alter previously initialized
            //? pixels if those are not 1, 11, 1111

            switch (this->pixel_size) {
                
                case 1:
                    switch (l_shift) {
                        case 7:
                            this->vec[i][vec_j] &= ((p_value << l_shift) | OR_MASK_P1_0);
                            break;
                        case 6:
                            this->vec[i][vec_j] &= ((p_value << l_shift) | OR_MASK_P1_1);
                            break;
                        case 5:
                            this->vec[i][vec_j] &= ((p_value << l_shift) | OR_MASK_P1_2);
                            break;
                        case 4:
                            this->vec[i][vec_j] &= ((p_value << l_shift) | OR_MASK_P1_3);
                            break;
                        case 3:
                            this->vec[i][vec_j] &= ((p_value << l_shift) | OR_MASK_P1_4);
                            break;
                        case 2:
                            this->vec[i][vec_j] &= ((p_value << l_shift) | OR_MASK_P1_5);
                            break;
                        case 1:
                            this->vec[i][vec_j] &= ((p_value << l_shift) | OR_MASK_P1_6);
                            break;
                        case 0:
                            this->vec[i][vec_j] &= ((p_value << l_shift) | OR_MASK_P1_7);
                            break;
                        default:
                            ERROR_MSG("unexpected l_shift value in fromImage");
                            exit(1);
                            break;
                    }
                    break;

                case 2:
                    switch (l_shift) {
                        case 6:
                            this->vec[i][vec_j] &= ((p_value << l_shift) | OR_MASK_P2_0);
                            break;
                        case 4:
                            this->vec[i][vec_j] &= ((p_value << l_shift) | OR_MASK_P2_1);
                            break;
                        case 2:
                            this->vec[i][vec_j] &= ((p_value << l_shift) | OR_MASK_P2_2);
                            break;
                        case 0:
                            this->vec[i][vec_j] &= ((p_value << l_shift) | OR_MASK_P2_3);
                            break;
                        default:
                            ERROR_MSG("unexpected l_shift value in fromImage");
                            exit(1);
                            break;
                    }
                    break;

                case 4:
                    switch (l_shift) {
                        case 4:
                            this->vec[i][vec_j] &= ((p_value << l_shift) | OR_MASK_P4_0);
                            break;
                        case 0:
                            this->vec[i][vec_j] &= ((p_value << l_shift) | OR_MASK_P4_1);
                            break;
                        default:
                            ERROR_MSG("unexpected l_shift value in fromImage");
                            exit(1);
                            break;
                    }
                    break;
                
                default:
                    ERROR_MSG("invalid pixel_size value in fromImage");
                    exit(1);
                    break;

            }

        }
    }

    this->last_add_r0 = 0;
    this->last_add_c0 = 0;
    this->last_add_height = img_rows;
    this->last_add_width = (img_cols+this->pixels_per_byte-1)/this->pixels_per_byte;
    return 0;    

}



int p2b::Bitmap::fromImage_parallel(cv::Mat* img_ptr){
    
    cv::Mat gs_img = *img_ptr;
    if (img_ptr->channels() > 1){
        cv::cvtColor(*img_ptr, gs_img, cv::COLOR_BGR2GRAY);
    }
    
    gs_img.forEach<uint8_t>(
        [this](uint8_t& pixel, const int* position) -> void {

            int i = position[0];
            int j = position[1];

            size_t vec_j = j/this->pixels_per_byte;
            uint8_t p_value = 0;
            uint8_t l_shift;

            for (uint8_t& threshold : this->thresholds_v){
                if (pixel < threshold)
                    break;
                ++p_value;
            }

            l_shift = (8-this->pixel_size) - ((j%this->pixels_per_byte) * this->pixel_size);

            switch (this->pixel_size) {
                
                case 1:
                    switch (l_shift) {
                        case 7:
                            this->vec[i][vec_j] &= ((p_value << l_shift) | OR_MASK_P1_0);
                            break;
                        case 6:
                            this->vec[i][vec_j] &= ((p_value << l_shift) | OR_MASK_P1_1);
                            break;
                        case 5:
                            this->vec[i][vec_j] &= ((p_value << l_shift) | OR_MASK_P1_2);
                            break;
                        case 4:
                            this->vec[i][vec_j] &= ((p_value << l_shift) | OR_MASK_P1_3);
                            break;
                        case 3:
                            this->vec[i][vec_j] &= ((p_value << l_shift) | OR_MASK_P1_4);
                            break;
                        case 2:
                            this->vec[i][vec_j] &= ((p_value << l_shift) | OR_MASK_P1_5);
                            break;
                        case 1:
                            this->vec[i][vec_j] &= ((p_value << l_shift) | OR_MASK_P1_6);
                            break;
                        case 0:
                            this->vec[i][vec_j] &= ((p_value << l_shift) | OR_MASK_P1_7);
                            break;
                        default:
                            ERROR_MSG("unexpected l_shift value in fromImage");
                            exit(1);
                            break;
                    }
                    break;

                case 2:
                    switch (l_shift) {
                        case 6:
                            this->vec[i][vec_j] &= ((p_value << l_shift) | OR_MASK_P2_0);
                            break;
                        case 4:
                            this->vec[i][vec_j] &= ((p_value << l_shift) | OR_MASK_P2_1);
                            break;
                        case 2:
                            this->vec[i][vec_j] &= ((p_value << l_shift) | OR_MASK_P2_2);
                            break;
                        case 0:
                            this->vec[i][vec_j] &= ((p_value << l_shift) | OR_MASK_P2_3);
                            break;
                        default:
                            ERROR_MSG("unexpected l_shift value in fromImage");
                            exit(1);
                            break;
                    }
                    break;

                case 4:
                    switch (l_shift) {
                        case 4:
                            this->vec[i][vec_j] &= ((p_value << l_shift) | OR_MASK_P4_0);
                            break;
                        case 0:
                            this->vec[i][vec_j] &= ((p_value << l_shift) | OR_MASK_P4_1);
                            break;
                        default:
                            ERROR_MSG("unexpected l_shift value in fromImage");
                            exit(1);
                            break;
                    }
                    break;
                
                default:
                    ERROR_MSG("invalid pixel_size value in fromImage");
                    exit(1);
                    break;

            }

        }
    );

    this->last_add_r0 = 0;
    this->last_add_c0 = 0;
    this->last_add_height = img_ptr->rows;
    this->last_add_width = (img_ptr->cols+this->pixels_per_byte-1)/this->pixels_per_byte;
    return 0;

}







/*
    Updates teh content of the bitmap according to the new img_ptr,
    a while loop can be used to call proper resizing of bitmap as it 
    returns 1 if dimensions do not suffice
*/
int p2b::Bitmap::updateFromImage(cv::Mat* update_img_ptr){
    /*
    if (
        (this->rows < update_img_ptr->rows) || 
        (this->cols < ((update_img_ptr->cols+this->pixels_per_byte-1)/this->pixels_per_byte))
    ){
        ERROR_MSG("bitmap dimensions are not enought to contain image, you may want to resize the bitmap");
        return 1;
    }
    */

    this->vec = p2b::toBits(update_img_ptr, this->pixel_size, this->thresholds_v);
    
    this->rows = update_img_ptr->rows;
    this->cols = (update_img_ptr->cols+this->pixels_per_byte-1)/this->pixels_per_byte;

    this->last_add_r0 = 0;
    this->last_add_c0 = 0;
    this->last_add_height = this->rows;
    this->last_add_width = this->cols;

    return 0;
}

/*
    Function to update only a region of the original bitmap,
    start_row and start_col are teh indexes from which to start updating,
    referring to image pixel indexes and not to bitmap's
*/
int p2b::Bitmap::updateRegionFromImage(cv::Mat* update_img_ptr, long start_row, long start_col){
    if (
        (start_row + update_img_ptr->rows > this->rows) || 
        ((start_col + update_img_ptr->cols +this->pixels_per_byte-1)/this->pixels_per_byte > this->cols)
    ){
        ERROR_MSG("total expected dimensions are bigger than bitmap dimensions");
        return 1;
    }

    vector<vector<uint8_t>> tmp_vec = p2b::toBits(update_img_ptr, this->pixel_size, this->thresholds_v);
    for (long i=0; i<update_img_ptr->rows; ++i){
        for (long j=0; j<update_img_ptr->cols; ++j){
            this->vec[i+start_row][j+start_col] = tmp_vec[i][j];
        }
    }

    return 0;
}








int p2b::Bitmap::addImage(cv::Mat* img_ptr, const int add_direction, bool minimal_resizing){
    if (add_direction < 0 || add_direction > 3){
        ERROR_MSG("invalid add_direction constant (UP=0, RIGHT=1, DOWN=2, LEFT=3)");
        return 1;
    }

    long img_rows = img_ptr->rows;
    long img_cols = (img_ptr->cols + this->pixels_per_byte - 1)/this->pixels_per_byte;

    long start_row;
    long start_col;
    vector<vector<uint8_t>> tmp_vec = p2b::toBits(img_ptr, this->pixel_size, this->thresholds_v);
    switch (add_direction) {
        
        case p2b::DIR_UP:
            while ((this->last_add_r0 - img_rows) < 0 || (img_cols + this->last_add_c0) > this->cols){
                (!minimal_resizing) ? this->doubleSize(add_direction) : 
                this->increaseSize(
                    MAX_SIZE(
                        this->rows, 
                        img_rows + this->rows - this->last_add_r0
                    ), 
                    MAX_SIZE(
                        this->cols, 
                        img_cols + this->cols - this->last_add_c0
                    ), 
                    add_direction
                );
            }

            start_row = this->last_add_r0 - img_rows;
            start_col = this->last_add_c0;
            for (long i=0; i<img_rows; ++i){
                for (long j=0; j<img_cols; ++j){
                    this->vec[i+start_row][j+start_col] = tmp_vec[i][j];
                }
            }
            break;

        case p2b::DIR_RIGHT:
            while ((this->last_add_c0 + this->last_add_width + img_cols) > this->cols || (this->last_add_r0 + img_rows) > this->rows){
                (!minimal_resizing) ? this->doubleSize(add_direction) : 
                this->increaseSize(
                    MAX_SIZE(
                        this->rows, 
                        img_rows + this->last_add_r0
                    ), 
                    MAX_SIZE(
                        this->cols, 
                        img_cols + this->last_add_c0 + this->last_add_width
                    ), 
                    add_direction
                );
            }

            start_row = this->last_add_r0;
            start_col = this->last_add_c0 + this->last_add_width;
            for (long i=0; i<img_rows; ++i){
                for (long j=0; j<img_cols; ++j){
                    this->vec[i+start_row][j+start_col] = tmp_vec[i][j];
                }
            }
            break;

        case p2b::DIR_DOWN:
            while ((this->last_add_r0 + this->last_add_height + img_rows) > this->rows || (this->last_add_c0 + img_cols) > this->cols){
                (!minimal_resizing) ? this->doubleSize(add_direction) 
                : this->increaseSize(
                    MAX_SIZE(
                        this->rows, 
                        img_rows + this->last_add_r0 + this->last_add_height
                    ), 
                    MAX_SIZE(
                        this->cols, 
                        img_cols + this->last_add_c0
                    ), 
                    add_direction
                );
            }

            start_row = this->last_add_r0 + this->last_add_height;
            start_col = this->last_add_c0;
            for (long i=0; i<img_rows; ++i){
                for (long j=0; j<img_cols; ++j){
                    this->vec[i+start_row][j+start_col] = tmp_vec[i][j];
                }
            }
            break;

        case p2b::DIR_LEFT:
            while ((this->last_add_c0 - img_cols) < 0 || (img_rows + this->last_add_r0) > this->rows){
                (!minimal_resizing) ? this->doubleSize(add_direction) : 
                this->increaseSize(
                    MAX_SIZE(
                        this->rows, 
                        img_rows + this->rows - this->last_add_r0
                    ), 
                    MAX_SIZE(
                        this->cols, 
                        img_cols + this->cols - this->last_add_c0
                    ), 
                    add_direction
                );
            }

            start_row = this->last_add_r0;
            start_col = this->last_add_c0 - img_cols;
            for (long i=0; i<img_rows; ++i){
                for (long j=0; j<img_cols; ++j){
                    this->vec[i+start_row][j+start_col] = tmp_vec[i][j];
                }
            }
            break;
    
    }

    this->last_add_r0 = start_row;
    this->last_add_c0 = start_col;
    this->last_add_height = img_rows;
    this->last_add_width = img_cols;

    return 0;

}









int p2b::Bitmap::toGrayscaleImage_linear(cv::Mat* dst_img, const vector<uint8_t>& grayscale_palette){
    
    if (grayscale_palette.size() != this->pixel_values){
        ERROR_MSG("grayscale_palette size doesn't match pixel_values");
        return 1;
    }

    size_t img_rows = this->rows;
    size_t img_cols = this->cols * this->pixels_per_byte;
    dst_img->create(img_rows,img_cols,CV_8UC1);

    size_t vec_j;
    uint8_t p_value;
    uint8_t r_shift;

    for (size_t i=0; i<img_rows; ++i){
        for (size_t j=0; j<img_cols; ++j){

            vec_j = j/this->pixels_per_byte;
            r_shift = (8-this->pixel_size) - ((j%this->pixels_per_byte) * this->pixel_size);

            switch (this->pixel_size) {
            
                case 1:
                    switch (r_shift) {
                        case 7:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P1_0) >> r_shift;
                            break;
                        case 6:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P1_1) >> r_shift;
                            break;
                        case 5:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P1_2) >> r_shift;
                            break;
                        case 4:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P1_3) >> r_shift;
                            break;
                        case 3:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P1_4) >> r_shift;
                            break;
                        case 2:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P1_5) >> r_shift;
                            break;
                        case 1:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P1_6) >> r_shift;
                            break;
                        case 0:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P1_7) >> r_shift;
                            break;
                        default:
                            ERROR_MSG("unexpected r_shift value in toGrayscaleImage");
                            exit(1);
                            break;
                    }
                    break;

                case 2:
                    switch (r_shift) {
                        case 6:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P2_0) >> r_shift;
                            break;
                        case 4:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P2_1) >> r_shift;
                            break;
                        case 2:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P2_2) >> r_shift;
                            break;
                        case 0:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P2_3) >> r_shift;
                            break;
                        default:
                            ERROR_MSG("unexpected r_shift value in toGrayscaleImage");
                            exit(1);
                            break;
                    }
                    break;

                case 4:
                    switch (r_shift) {
                        case 4:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P4_0) >> r_shift;
                            break;
                        case 0:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P4_1) >> r_shift;
                            break;
                        default:
                            ERROR_MSG("unexpected r_shift value in toGrayscaleImage");
                            exit(1);
                            break;
                    }
                    break;
                
                default:
                    ERROR_MSG("invalid pixel_size value in toGrayscaleImage");
                    exit(1);
                    break;

            }

            dst_img->at<uint8_t>(i,j) = (p_value!=this->pixel_values) ? grayscale_palette[p_value] : 0;

        }
    }

    return 0;

}



int p2b::Bitmap::toGrayscaleImage_parallel(cv::Mat* dst_img, const std::vector<uint8_t>& grayscale_palette){

    if (grayscale_palette.size() != this->pixel_values){
        ERROR_MSG("grayscale_palette size doesn't match pixel_values");
        return 1;
    }

    size_t img_rows = this->rows;
    size_t img_cols = this->cols * this->pixels_per_byte;
    dst_img->create(img_rows,img_cols,CV_8UC1);

    dst_img->forEach<uint8_t>(
        [this, grayscale_palette](uint8_t& pixel, const int* position) -> void {

            int i = position[0];
            int j = position[1];

            size_t vec_j = j/this->pixels_per_byte;
            uint8_t p_value;
            uint8_t r_shift = (8-this->pixel_size) - ((j%this->pixels_per_byte) * this->pixel_size);

            switch (this->pixel_size) {
            
                case 1:
                    switch (r_shift) {
                        case 7:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P1_0) >> r_shift;
                            break;
                        case 6:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P1_1) >> r_shift;
                            break;
                        case 5:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P1_2) >> r_shift;
                            break;
                        case 4:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P1_3) >> r_shift;
                            break;
                        case 3:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P1_4) >> r_shift;
                            break;
                        case 2:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P1_5) >> r_shift;
                            break;
                        case 1:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P1_6) >> r_shift;
                            break;
                        case 0:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P1_7) >> r_shift;
                            break;
                        default:
                            ERROR_MSG("unexpected r_shift value in toGrayscaleImage");
                            exit(1);
                            break;
                    }
                    break;

                case 2:
                    switch (r_shift) {
                        case 6:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P2_0) >> r_shift;
                            break;
                        case 4:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P2_1) >> r_shift;
                            break;
                        case 2:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P2_2) >> r_shift;
                            break;
                        case 0:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P2_3) >> r_shift;
                            break;
                        default:
                            ERROR_MSG("unexpected r_shift value in toGrayscaleImage");
                            exit(1);
                            break;
                    }
                    break;

                case 4:
                    switch (r_shift) {
                        case 4:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P4_0) >> r_shift;
                            break;
                        case 0:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P4_1) >> r_shift;
                            break;
                        default:
                            ERROR_MSG("unexpected r_shift value in toGrayscaleImage");
                            exit(1);
                            break;
                    }
                    break;
                
                default:
                    ERROR_MSG("invalid pixel_size value in toGrayscaleImage");
                    exit(1);
                    break;

            }

            pixel = (p_value!=this->pixel_values) ? grayscale_palette[p_value] : 0;
            

        }
    );

    return 0;

}











int p2b::Bitmap::toBGRImage_linear(cv::Mat* dst_img, const vector<cv::Vec3b>& BGR_palette){
    
    if (BGR_palette.size() != this->pixel_values){
        ERROR_MSG("BGR_palette size doesn't match pixel_values");
        return 1;
    }

    size_t img_rows = this->rows;
    size_t img_cols = this->cols * this->pixels_per_byte;
    dst_img->create(img_rows,img_cols,CV_8SC3);

    size_t vec_j;
    uint8_t p_value;
    uint8_t r_shift;

    for (size_t i=0; i<img_rows; ++i){
        for (size_t j=0; j<img_cols; ++j){

            vec_j = j/this->pixels_per_byte;
            r_shift = (8-this->pixel_size) - ((j%this->pixels_per_byte) * this->pixel_size);

            switch (this->pixel_size) {
            
                case 1:
                    switch (r_shift) {
                        case 7:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P1_0) >> r_shift;
                            break;
                        case 6:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P1_1) >> r_shift;
                            break;
                        case 5:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P1_2) >> r_shift;
                            break;
                        case 4:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P1_3) >> r_shift;
                            break;
                        case 3:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P1_4) >> r_shift;
                            break;
                        case 2:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P1_5) >> r_shift;
                            break;
                        case 1:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P1_6) >> r_shift;
                            break;
                        case 0:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P1_7) >> r_shift;
                            break;
                        default:
                            ERROR_MSG("unexpected r_shift value in toBGRImage");
                            exit(1);
                            break;
                    }
                    break;

                case 2:
                    switch (r_shift) {
                        case 6:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P2_0) >> r_shift;
                            break;
                        case 4:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P2_1) >> r_shift;
                            break;
                        case 2:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P2_2) >> r_shift;
                            break;
                        case 0:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P2_3) >> r_shift;
                            break;
                        default:
                            ERROR_MSG("unexpected r_shift value in toBGRImage");
                            exit(1);
                            break;
                    }
                    break;

                case 4:
                    switch (r_shift) {
                        case 4:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P4_0) >> r_shift;
                            break;
                        case 0:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P4_1) >> r_shift;
                            break;
                        default:
                            ERROR_MSG("unexpected r_shift value in toBGRImage");
                            exit(1);
                            break;
                    }
                    break;
                
                default:
                    ERROR_MSG("invalid pixel_size value in toBGRImage");
                    exit(1);
                    break;

            }

            //TODO: why is the pixel set to a generic gray value?
            dst_img->at<cv::Vec3b>(i,j) = (p_value!=this->pixel_values) ? BGR_palette[p_value] : cv::Vec3b(0,0,0);

        }
    }

    return 0;

}



int p2b::Bitmap::toBGRImage_parallel(cv::Mat* dst_img, const std::vector<std::vector<uint8_t>>& BGR_palette){

    if (BGR_palette.size() != this->pixel_values){
        ERROR_MSG("BGR_palette size doesn't match pixel_values");
        return 1;
    }

    size_t img_rows = this->rows;
    size_t img_cols = this->cols * this->pixels_per_byte;
    dst_img->create(img_rows,img_cols,CV_8SC3);

    dst_img->forEach<cv::Vec3b>(
        [this, BGR_palette](cv::Vec3b& pixel, const int* position) -> void {

            int i = position[0];
            int j = position[1];

            size_t vec_j = j/this->pixels_per_byte;
            uint8_t p_value;
            uint8_t r_shift = (8-this->pixel_size) - ((j%this->pixels_per_byte) * this->pixel_size);

            switch (this->pixel_size) {
            
                case 1:
                    switch (r_shift) {
                        case 7:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P1_0) >> r_shift;
                            break;
                        case 6:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P1_1) >> r_shift;
                            break;
                        case 5:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P1_2) >> r_shift;
                            break;
                        case 4:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P1_3) >> r_shift;
                            break;
                        case 3:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P1_4) >> r_shift;
                            break;
                        case 2:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P1_5) >> r_shift;
                            break;
                        case 1:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P1_6) >> r_shift;
                            break;
                        case 0:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P1_7) >> r_shift;
                            break;
                        default:
                            ERROR_MSG("unexpected r_shift value in toBGRImage");
                            exit(1);
                            break;
                    }
                    break;

                case 2:
                    switch (r_shift) {
                        case 6:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P2_0) >> r_shift;
                            break;
                        case 4:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P2_1) >> r_shift;
                            break;
                        case 2:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P2_2) >> r_shift;
                            break;
                        case 0:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P2_3) >> r_shift;
                            break;
                        default:
                            ERROR_MSG("unexpected r_shift value in toBGRImage");
                            exit(1);
                            break;
                    }
                    break;

                case 4:
                    switch (r_shift) {
                        case 4:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P4_0) >> r_shift;
                            break;
                        case 0:
                            p_value = (this->vec[i][vec_j] & AND_MASK_P4_1) >> r_shift;
                            break;
                        default:
                            ERROR_MSG("unexpected r_shift value in toBGRImage");
                            exit(1);
                            break;
                    }
                    break;
                
                default:
                    ERROR_MSG("invalid pixel_size value in toBGRImage");
                    exit(1);
                    break;

            }

            //TODO: why is the pixel set to a generic gray value?
            if (p_value != this->pixel_values){
                pixel[0] = BGR_palette[p_value][0];
                pixel[1] = BGR_palette[p_value][1];
                pixel[2] = BGR_palette[p_value][2];
            }
            else {
                pixel[0] = 0;
                pixel[1] = 0;
                pixel[2] = 0;
            }
            //pixel = (p_value!=this->pixel_values) ? BGR_palette[p_value] : cv::Vec3b(0,0,0);

        }
    );

    return 0;

}