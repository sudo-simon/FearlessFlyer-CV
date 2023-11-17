/*
 *  Copyright (C) 2023 Simone Palmieri <github dot com/sudo-simon>
 *  All rights reserved.
 *
 *  This file is part of a project released under the GNU GENERAL PUBLIC LICENSE Version 3.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *  *  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  *  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <opencv2/core/mat.hpp>


// ----------------------------------------------------------------------



namespace p2b{



//? Bit masks to perform bitwise OR operations
const uint8_t OR_MASK_P1_0 = 0b01111111;
const uint8_t OR_MASK_P1_1 = 0b10111111;
const uint8_t OR_MASK_P1_2 = 0b11011111;
const uint8_t OR_MASK_P1_3 = 0b11101111;
const uint8_t OR_MASK_P1_4 = 0b11110111;
const uint8_t OR_MASK_P1_5 = 0b11111011;
const uint8_t OR_MASK_P1_6 = 0b11111101;
const uint8_t OR_MASK_P1_7 = 0b11111110;

const uint8_t OR_MASK_P2_0 = 0b00111111;
const uint8_t OR_MASK_P2_1 = 0b11001111;
const uint8_t OR_MASK_P2_2 = 0b11110011;
const uint8_t OR_MASK_P2_3 = 0b11111100;

const uint8_t OR_MASK_P4_0 = 0b00001111;
const uint8_t OR_MASK_P4_1 = 0b11110000;

//? Bit masks to perform bitwise AND operations
const uint8_t AND_MASK_P1_0 = 0b10000000;
const uint8_t AND_MASK_P1_1 = 0b01000000;
const uint8_t AND_MASK_P1_2 = 0b00100000;
const uint8_t AND_MASK_P1_3 = 0b00010000;
const uint8_t AND_MASK_P1_4 = 0b00001000;
const uint8_t AND_MASK_P1_5 = 0b00000100;
const uint8_t AND_MASK_P1_6 = 0b00000010;
const uint8_t AND_MASK_P1_7 = 0b00000001;

const uint8_t AND_MASK_P2_0 = 0b11000000;
const uint8_t AND_MASK_P2_1 = 0b00110000;
const uint8_t AND_MASK_P2_2 = 0b00001100;
const uint8_t AND_MASK_P2_3 = 0b00000011;

const uint8_t AND_MASK_P4_0 = 0b11110000;
const uint8_t AND_MASK_P4_1 = 0b00001111;


//? Constants used to initialize Bitmap objects
//const int BITMAP_TYPE_THRESHOLD = 0;


//? Constants used by the addBits function
const int DIR_UP = 0;
const int DIR_RIGHT = 1;
const int DIR_DOWN = 2;
const int DIR_LEFT = 3;



/**
* @brief The Bitmap class used to store informations, what the library revolves around
*/
class Bitmap{

    private:

        /*
        unsigned or int? Significant security issues involving overflows?
        */
        long rows;
        long cols;
        uint8_t pixel_size;
        uint8_t pixels_per_byte;
        uint8_t pixel_values;
        std::vector<uint8_t> thresholds_v;

        //? Values used to store informations about the last added image to the bitmap
        long last_add_r0;
        long last_add_c0;
        long last_add_height;
        long last_add_width;

        /*TODO
        We can include values, such as
            unsigned square_size
            unsigned last_added_row
            unsigned last_added_col
        to implement a bitmap that keeps in memory what is the last square it
        has added to itself (i and j) and proceed from there. It can use the square_size 
        value to estabilish the correct indexes of the square and the next one to add.
        */

        std::vector<std::vector<uint8_t>> vec;

    public:

        Bitmap();
        Bitmap(long rows, long cols, uint8_t pixel_size, const std::vector<uint8_t>& thresholds_v);
        ~Bitmap();
        
        long getRows();
        long getCols();
        uint8_t getPixelSize();
        uint8_t getPixelValues();
        std::vector<uint8_t> getThresholds();
        std::vector<std::vector<uint8_t>> getVec();

        int increaseSize(const long new_rows, const long new_cols, const int resize_direction);
        int doubleSize(const int resize_direction);

        int fromImage_linear(cv::Mat* img_ptr);
        int fromImage_parallel(cv::Mat* img_ptr);

        int updateFromImage(cv::Mat* update_img_ptr);
        int updateRegionFromImage(cv::Mat* update_img_ptr, long start_row, long start_col);

        int addImage(cv::Mat* add_img_ptr, const int add_direction, bool minimal_resizing);
        
        int toGrayscaleImage_linear(cv::Mat* dst_img, const std::vector<uint8_t>& grayscale_palette);
        int toGrayscaleImage_parallel(cv::Mat* dst_img, const std::vector<uint8_t>& grayscale_palette);

        //! toBGRImage functions do not work
        //TODO debug and fix
        int toBGRImage_linear(cv::Mat* dst_img, const std::vector<cv::Vec3b>& color_palette);
        int toBGRImage_parallel(cv::Mat* dst_img, const std::vector<std::vector<uint8_t>>& color_palette);

};






}   //? End of p2b namespace