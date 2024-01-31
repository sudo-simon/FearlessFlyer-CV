
#include "StitcherThread.hpp"
#include "modules/BlockingQueue.hpp"
#include "modules/StateBoard.hpp"
#include <buffers.hpp>
#include <cstddef>
#include <exception>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/mat.hpp>

#include <opencv2/core/matx.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <ostream>
#include <vector>


void StitcherThread::Start(){

    cv::Mat frame;
    std::cout << "---- STITCHER THREAD STARTED ----" << std::endl;

    bool isTerminated = false;
    while(!isTerminated){
        this->fromCap_buffer_ptr->take(frame);
        StitchingRoutine(frame);
        if(!frame.empty()){
            MapBufferUpdate();
        }

        this->termSig_ptr->read(isTerminated);
    }

    std::cout << "---- STITCHER THREAD STOPPED ----" << std::endl;
}

void StitcherThread::Terminate(){
    std::terminate();
}

void StitcherThread::InitializeStitcher(BlockingQueue<cv::Mat>* fifo_ptr, BlockingQueue<cv::Mat>* buffer_ptr, StateBoard* termSig){
    this->fromCap_buffer_ptr = fifo_ptr;
    this->mapBuffer_ptr = buffer_ptr;
    this->termSig_ptr = termSig;
}

void StitcherThread::StitchingRoutine(cv::Mat& newFrame){

    std::cout << "STITCH" << std::endl;

    cv::Mat img1 = cv::imread("/home/lor3n/Dev/RTMP/scripts/1.png");
    cv::Mat img2 = cv::imread("/home/lor3n/Dev/RTMP/scripts/2.png");

    if(img1.empty() || img2.empty()){
        std::cout << "immagini non trovate";
    }

    cv::Ptr<cv::ORB> detector = cv::ORB::create();
    std::vector<cv::KeyPoint> keypoints1, keypoints2;
    cv::Mat descriptor1, descriptor2;

    detector->detectAndCompute(img1, cv::noArray(), keypoints1, descriptor1);
    detector->detectAndCompute(img2, cv::noArray(), keypoints2, descriptor2);

    cv::BFMatcher matcher(cv::NORM_HAMMING);
    std::vector<cv::DMatch> matches;
    matcher.match(descriptor1, descriptor2, matches);

    std::sort(matches.begin(), matches.end(), [](const cv::DMatch& a, const cv::DMatch& b) {
        return a.distance < b.distance;
    });

    int numGoodMatches = static_cast<int>(matches.size() * 0.15);
    matches.resize(numGoodMatches);

    cv::Mat points1, points2;
    for (const auto& match : matches) {
        points1.push_back(keypoints1[match.queryIdx].pt);
        points2.push_back(keypoints2[match.trainIdx].pt);
    }

    /* Bisogna capire perchè l' homografia viene diversa da python, verificare che i points siano uguali e capire da cosa deriva la differenza*/

    cv::Mat H = cv::findHomography(points1, points2, cv::RANSAC);

    cv::Mat imgWarped = warpPerspectiveNoCut(img2, H);

    double dx = -H.at<double>(0, 2);
    double dy = -H.at<double>(1, 2);

    cv::imwrite("res.png", imgWarped);

    /* Bisogna capire come copiare, seguendo una regola generale*/

    int bottom = 0, left = 0, right = 0, top = 0;

    if (dy > 0) {
        top = std::ceil(dy);
        bottom = 0;
    } else {
        top = 0;
        bottom = std::ceil(std::abs(dy));
    }

    if (dx > 0) {
        left = 0;
        right = std::ceil(dx);
    } else {
        left = std::ceil(std::abs(dx));
        right = 0;
    }

    cv::Mat imageWithBorder;
    cv::copyMakeBorder(img1, imageWithBorder, top, bottom, left, right, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));

    int x_offset = static_cast<int>(std::copysign(std::ceil(std::abs(dx)), dx));
    int y_offset = static_cast<int>(std::copysign(std::ceil(std::abs(dy)), dy));

    int yError = imgWarped.rows - img1.rows;
    int xError = imgWarped.cols - img1.cols;

    for (int i = y_offset; i < imgWarped.rows + y_offset - yError; ++i) {
        std::cout << i << std::endl;
        for (int j = x_offset; j < imgWarped.cols + x_offset - xError; ++j) {
            if (imgWarped.at<cv::Vec3f>(i - y_offset, j - x_offset) == cv::Vec3f(0,0,0)) {
                continue;
            }
            imageWithBorder.at<cv::Vec3f>(i,j) = imageWithBorder.at<cv::Vec3f>(i,j) = imgWarped.at<cv::Vec3f>(i-y_offset, j-x_offset);
        }
    }

    cv::imwrite("res.png", imageWithBorder);
}


inline void StitcherThread::MapBufferUpdate(){
    this->mapBuffer_ptr->put(this->map);
}

cv::Mat StitcherThread::warpPerspectiveNoCut(const cv::Mat& srcImage, cv::Mat transformationMatrix) {
    const float height = srcImage.rows;
    const float width = srcImage.cols;

    cv::Vec2f data[4] = {cv::Vec2f(0,0),cv::Vec2f(width,0),cv::Vec2f(width,height), cv::Vec2f(0,height)};
    cv::Mat srcCorners = cv::Mat(4, 1, CV_32FC2, data); 

    // Create a new 2x3 matrix with the defined values
    cv::Mat dstCorners = cv::Mat(4,2, CV_32FC2);
    
    cv::perspectiveTransform(srcCorners, dstCorners, transformationMatrix);

    //trova il minimo ed il massimo su l'asse delle x e delle y
    float maxX = 0, minX = 0;
    float maxY = 0, minY = 0;

    for (int i = 0; i < dstCorners.rows; ++i) {
        for (int j = 0; j < dstCorners.cols; ++j) {
            cv::Vec2f point = dstCorners.at<cv::Vec2f>(i, j);
            float x = point[0];
            float y = point[1];

            // Update min and max values
            minX = std::min(minX, x);
            minY = std::min(minY, y);
            maxX = std::max(maxX, x);
            maxY = std::max(maxY, y);
        }
    }


    std::cout << maxX << std::endl;

    int dstWidth = static_cast<int>(maxX - minX);
    int dstHeight = static_cast<int>(maxY - minY);

    
    float dataShift[9] = {1, 0, -minX, 0, 1, -minY, 0, 0, 1};
    cv::Mat shiftMatrix = cv::Mat(3, 3, CV_32F, dataShift); 

    transformationMatrix.convertTo(transformationMatrix, CV_32F);

    cv::Mat adjustedMatrix;
    std::cout << shiftMatrix.type() <<std::endl;
    std::cout << transformationMatrix.type() <<std::endl;
    cv::gemm(shiftMatrix, transformationMatrix, 1.0, cv::Mat(), 0.0, adjustedMatrix);

    std::cout << adjustedMatrix <<std::endl;

    cv::Mat dstImage;
    cv::warpPerspective(srcImage, dstImage, adjustedMatrix, cv::Size(dstWidth, dstHeight));


    return dstImage;
}