#include "Canvas.hpp"
#include "modules/BlockingQueue.hpp"
#include <algorithm>
#include <cmath>
#include <opencv2/core.hpp>
#include <opencv2/core/base.hpp>
#include <opencv2/core/hal/interface.h>
#include <iostream>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

//? AUX function to check if all pixel values are empty
bool isPixelEmpty(cv::Vec4b& pixel){
    return (
        (pixel[0] == 0) &&
        (pixel[1] == 0) &&
        (pixel[2] == 0)
    );
}



void Canvas::stitch() {

    if(this->stitched_frames == 0){
        cv::Rect ROI = cv::Rect(
            (long) ((window_w - this->next_frame.cols) / 2),    //? should be 0
            (long) ((window_h - this->next_frame.rows) / 2),    //? should be 0
            this->next_frame.cols,
            this->next_frame.rows
        );
        this->next_frame.copyTo(this->canvas(ROI));
        return;
    }

    // Detect ORB keypoints and descriptors
    cv::Ptr<cv::ORB> orb = cv::ORB::create();
    std::vector<cv::KeyPoint> keypoints1, keypoints2;
    cv::Mat descriptors1, descriptors2;

    orb->detectAndCompute(
        this->canvas(cv::Rect(
            this->window_x,
            this->window_y,
            this->window_w,
            this->window_h
        )),
        cv::noArray(),
        keypoints1,
        descriptors1
    );
    orb->detectAndCompute(
        this->next_frame,
        cv::noArray(),
        keypoints2,
        descriptors2
    );

    // Match keypoints using a descriptor matcher (e.g., BFMatcher)
    cv::BFMatcher matcher(cv::NORM_HAMMING);
    std::vector<cv::DMatch> matches;
    matcher.match(descriptors1, descriptors2, matches);

    // Sort matches based on their distances
    std::sort(matches.begin(), matches.end());

    // Keep the top N matches (e.g., 50)
    int numMatchesToKeep = (int) (matches.size() * 0.75);
    matches.resize(numMatchesToKeep);

    // Find homography matrix using RANSAC
    std::vector<cv::Point2f> points1, points2;
    for (const auto& match : matches) {
        points1.push_back(keypoints1[match.queryIdx].pt);
        points2.push_back(keypoints2[match.trainIdx].pt);
    }

    cv::Mat H = cv::findHomography(points1, points2, cv::RANSAC, 3.0);
    long x = (long) std::round(-H.at<double>(0, 2));
    long y = (long) std::round(-H.at<double>(1, 2));
    //std::cout << "[STITCH] x = "<<x<<", y = "<<y<< std::endl;

    // Warp image1 to the perspective of image2 using the homography matrix
    cv::Mat warpedImage;
    cv::warpPerspective(
        this->canvas(cv::Rect(
            this->window_x,
            this->window_y,
            this->window_w,
            this->window_h
        )),
        warpedImage,
        H,
        cv::Size(this->window_w+std::abs(x)+100,this->window_h+std::abs(y)+100)
    );

    this->next_frame.copyTo(warpedImage(cv::Rect(0, 0, this->next_frame.cols, this->next_frame.rows)));
    warpedImage = this->removeBlackBorders(warpedImage);

    this->moveWindow(x,y,warpedImage.cols,warpedImage.rows);

    long horiz_region = this->canvas_w - this->window_x;
    long vert_region = this->canvas_h - this->window_y;

    /*
    std::cout <<
        "warped.cols = "<<warpedImage.cols <<"\n"<<
        "warped.rows = "<<warpedImage.rows <<"\n"<<
        "region w = "<< horiz_region <<"\n"<<
        "region h = "<< vert_region <<"\n"<<
    std::endl;
    */

    warpedImage.forEach<cv::Vec4b>(
        [this,horiz_region,vert_region](cv::Vec4b& pixel, const int* position) -> void {
            int i = position[0];
            int j = position[1];
            if (!isPixelEmpty(pixel) && i<vert_region && j<horiz_region) {
                this->canvas(cv::Rect(
                this->window_x,
                this->window_y,
                horiz_region,
                vert_region
                )).at<cv::Vec4b>(i,j) = pixel;
            }
        }
    );
    return;

}












Canvas::Canvas(long display_w, long display_h, long window_w, long window_h, BlockingQueue<cv::Mat>* buffer){
    if (
        display_w < 0 ||
        display_h < 0 ||
        window_w < 0 ||
        window_h < 0
    ){
        std::cerr << "[ERROR] Invalid Canvas initialization parameters" << std::endl;
        return;
    }
    
    this->canvas = cv::Mat(window_h,window_w,CV_8UC4);
    this->canvas.setTo(cv::Scalar(0,0,0, 0));
    this->canvas_w = window_w;
    this->canvas_h = window_h;
    
    this->display = cv::Mat(display_h,display_w,CV_8UC4);
    this->display.setTo(cv::Scalar(0,0,0, 0));
    this->display_w = display_w;
    this->display_h = display_h;
    
    this->window_w = window_w;
    this->window_h = window_h;
    this->window_x = 0;
    this->window_y = 0;


    this->mapBuffer = buffer;
    this->stitched_frames = 0;

}



int Canvas::stitchFrame(cv::Mat newFrame) {


    this->next_frame = newFrame;

    this->stitch();
    
    this->updateDisplay();

    this->stitched_frames++;

    return 0;
}





cv::Mat Canvas::removeBlackBorders(cv::Mat& srcImg){
    long new_left_border = 0;
    long new_right_border = srcImg.cols;
    long new_top_border = 0;
    long new_bottom_border = srcImg.rows;
    
    bool changed_border = false;
    for (long j=srcImg.cols-1; j>0; --j){
        for (long i=0; i<srcImg.rows; ++i){
            if (!isPixelEmpty(srcImg.at<cv::Vec4b>(i,j))){
                new_right_border = j+1;
                changed_border = true;
                break;
            }
        }
        if (changed_border) break;
    }
    
    changed_border = false;
    for (long j=0; j<srcImg.cols; ++j){
        for (long i=0; i<srcImg.rows; ++i){
            if (!isPixelEmpty(srcImg.at<cv::Vec4b>(i,j))){
                new_left_border = j;
                changed_border = true;
                break;
            }
        }
        if (changed_border) break;
    }

    changed_border = false;
    for (long i=srcImg.rows-1; i>0; --i){
        for (long j=0; j<srcImg.cols; ++j){
            if (!isPixelEmpty(srcImg.at<cv::Vec4b>(i,j))){
                new_bottom_border = i+1;
                changed_border = true;
                break;
            }
        }
        if (changed_border) break;
    }

    changed_border = false;
    for (long i=0; i<srcImg.rows; ++i){
        for (long j=0; j<srcImg.cols; ++j){
            if (!isPixelEmpty(srcImg.at<cv::Vec4b>(i,j))){
                new_top_border = i;
                changed_border = true;
                break;
            }
        }
        if (changed_border) break;
    }

    cv::Mat newImg = srcImg(cv::Rect(
        new_left_border,
        new_top_border,
        (new_right_border - new_left_border),
        (new_bottom_border - new_top_border)
    )).clone();

    return newImg;

}



int Canvas::moveWindow(long x, long y, long newImage_cols, long newImage_rows) {

    long left_border = ((x < 0) ? (-x) : 0);    //? Moving LEFT
    long right_border = ((x > 0) ? x : 0);      //? Moving RIGHT
    long top_border = ((y < 0) ? (-y) : 0);     //? Moving UP
    long bottom_border = ((y > 0) ? y : 0);     //? Moving DOWN

    /*
    long future_x = this->window_x + x + left_border;
    long future_y = this->window_y + y + top_border;
    long future_w = this->window_w + left_border + right_border;
    long future_h = this->window_h + top_border + bottom_border;

    long horiz_space = future_w - future_x;
    long vert_space = future_h - future_y;

    if (horiz_space < newImage_cols){
        right_border += 50;
    }
    if (vert_space < newImage_rows){
        bottom_border += 50;
    }
    */

    if (top_border != 0 || bottom_border != 0 || left_border != 0 || right_border != 0){
        //std::cout << "[moveWindow] Calling resizeCanvas" << std::endl;
        if (this->resizeCanvas(top_border, bottom_border, left_border, right_border) != 0){
            return -1;
        }
    }

    this->window_x += x;
    this->window_y += y;
    
    return 0;
}



int Canvas::resizeCanvas(long top_border, long bottom_border, long left_border, long right_border) {
    if (top_border < 0 || bottom_border < 0 || left_border < 0 || right_border < 0){
        std::cerr << "[ERROR] New canvas border sizes can't be negative" << std::endl;
        return -1;
    }

    /*
    std::cout <<
        "[DEBUG - resizeCanvas] top_border = "<<top_border <<"\n"<<
        "[DEBUG - resizeCanvas] bottom_border = "<<bottom_border <<"\n"<<
        "[DEBUG - resizeCanvas] left_border = "<<left_border <<"\n"<<
        "[DEBUG - resizeCanvas] right_border = "<<right_border <<"\n"<<
    std::endl;
    */

    cv::copyMakeBorder(
        this->canvas, 
        this->canvas, 
        top_border, 
        bottom_border, 
        left_border, 
        right_border, 
        cv::BORDER_CONSTANT,
        cv::Scalar(0,0,0, 0)
    );

    this->canvas_h += top_border + bottom_border;
    this->canvas_w += left_border + right_border;

    this->window_x += left_border;
    this->window_y += top_border;

    return 0;
}



void resizeWithRatio(cv::Mat& image, const int& height, const int& width){
    int new_x;
    int new_y;

    int top = 0;
    int bottom = 0;
    int left = 0;
    int right = 0;

    
    double ratio = 0;

    //Con la mappa crasha

    if(image.cols>image.rows){
        ratio = (double) image.rows/image.cols;
    } else {
        ratio = (double) image.cols/image.rows;
    }

    if(height<width){
        new_y = height;
        new_x = height/ratio;

        if(new_x>width){
            new_x = width;
            new_y = width*ratio;

            top = (height - new_y)/2;
            bottom = (height - new_y)/2;

        } else {
            left = (width - new_x)/2;
            right = (width - new_x)/2;
        }

    } else {
        new_x = width;
        new_y = width*ratio;

        if(new_y>height){
            new_y = height;
            new_x = height/ratio;

            left = (width - new_x)/2;
            right = (width - new_x)/2;
        } else {
            top = (height - new_y)/2;
            bottom = (height - new_y)/2;
        }   
    }

    cv::resize(image, image, cv::Size(new_x, new_y), cv::INTER_AREA);
    cv::copyMakeBorder(image, image, top, bottom, left, right, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0, 0));
}

void Canvas::updateDisplay() {

    cv::Mat tmp_display = this->removeBlackBorders(this->canvas);
    resizeWithRatio(tmp_display, this->display_h, this->display_w);
    //this->display.setTo(cv::Scalar(0,0,0,0));
    this->display = tmp_display.clone();

    this->mapBuffer->put(this->display);
    
    return;

}








