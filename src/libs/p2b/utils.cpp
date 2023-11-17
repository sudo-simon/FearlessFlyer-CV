#include "utils.hpp"


#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/imgproc.hpp>
#include <string>


using namespace std;

// ----------------------------------------------------------------------



void p2b::DEBUG_MSG(std::string msg){
    cout << "---- P2B DEBUG ----\n" << msg << endl;
}



void p2b::ERROR_MSG(std::string msg){
    cerr << "---- P2B ERROR ----\n" << msg << endl;
}



void p2b::PRINT_METRICS(cv::Mat img, p2b::Bitmap bitmap, long img2bmp_time_ms, long bmp2img_time_ms){

    const unsigned max_len = 512; //? If more chars will ever be needed
    char out_msg[max_len] = "\0";

    const int channels = img.channels();
    cv::Mat gs_img;
    cv::cvtColor(img, gs_img, cv::COLOR_BGR2GRAY);
    
    size_t img_size = 0;
    size_t gsc_size = 0;
    size_t bmp_size = 0;

    //? Original image size calculation
    img_size += (img.rows * img.cols * channels);
    switch (channels) {
        case 1:
            img_size += (img.rows * sizeof(img.at<uint8_t>(0)));
            break;
        case 2:
            img_size += (img.rows * sizeof(img.at<cv::Vec2b>(0)));
            break;
        case 3:
            img_size += (img.rows * sizeof(img.at<cv::Vec3b>(0)));
            break;
        case 4:
            img_size += (img.rows * sizeof(img.at<cv::Vec4b>(0)));
            break;
    }
    img_size += sizeof(img);

    //? Grayscale image size calculation
    gsc_size += (
        img.rows * img.cols +
        img.rows * sizeof(gs_img.at<uint8_t>(0)) +
        sizeof(img)
    );

    //? Bitmap object size calculation
    bmp_size += (
        bitmap.getRows() * bitmap.getCols() +
        bitmap.getRows() * sizeof(bitmap.getVec()[0]) +
        sizeof(bitmap)
    );

    //? Ratios
    float bmp_img_ratio = (float)bmp_size/img_size;
    float bmp_gsc_ratio = (float)bmp_size/gsc_size;


    snprintf(
        out_msg, max_len,
        "---- Bitmap parameters ----\n\n"
        "Pixel size = %d bit(s)\n"
        "Possible values per pixel = %d\n\n"

        "---- Memory metrics (of the first input image) ----\n\n"
        "Original image size = %ld bytes\n"
        "Grayscale image size = %ld bytes\n"
        "Bitmap size = %ld bytes\n\n"
        "Bitmap to image ratio = %.2f %%\nBitmap to grayscale ratio = %.2f %%\n\n"
        
        "---- Time metrics (of the first input image) ----\n\n"
        "Image to bitmap = %ld ms\n"
        "Bitmap to image = %ld ms\n",

        bitmap.getPixelSize(),
        bitmap.getPixelValues(),

        img_size,
        gsc_size,
        bmp_size,
        bmp_img_ratio*100, bmp_gsc_ratio*100,

        img2bmp_time_ms,
        bmp2img_time_ms
    );
    cout << out_msg << endl;

}



long p2b::MAX_SIZE(long size_1, long size_2){
    return (size_1 > size_2) ? size_1 : size_2;
}