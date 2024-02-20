
#include "StitcherThread.hpp"
#include "modules/BlockingQueue.hpp"
#include "modules/StateBoard.hpp"
#include <buffers.hpp>
#include <cstddef>
#include <exception>
#include <iostream>
#include <iterator>
#include <opencv2/core.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/mat.hpp>
#include "modules/Canvas/Canvas.hpp" 
#include <omp.h>

#include <opencv2/core/matx.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <ostream>
#include <string>
#include <vector>


void StitcherThread::Start(){

    cv::Mat frame;
    std::cout << "---- STITCHER THREAD STARTED ----" << std::endl;

    this->fromCap_buffer_ptr->take(frame);

    Canvas canvas(1920,1080, frame.cols, frame.rows, this->mapBuffer_ptr);

    bool isTerminated = false;
    while(!isTerminated){

        canvas.stitchFrame(frame);

        this->fromCap_buffer_ptr->take(frame);

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



// OLD STITCHING ROUTINE

/*
void StitcherThread::StitchingRoutine(cv::Mat& newFrame){

    std::chrono::milliseconds startTime = duration_cast<std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch());

    if(lastFrame.empty()){
        lastFrame = newFrame;
        this->map = newFrame;
        return;
    }

    cv::Ptr<cv::ORB> detector = cv::ORB::create();
    std::vector<cv::KeyPoint> keypoints1, keypoints2;
    cv::Mat descriptor1, descriptor2;
    

    detector->detectAndCompute(lastFrame, cv::noArray(), keypoints1, descriptor1);
    detector->detectAndCompute(newFrame, cv::noArray(), keypoints2, descriptor2);

    cv::BFMatcher matcher(cv::NORM_HAMMING);
    std::vector<cv::DMatch> matches;
    matcher.match(descriptor1, descriptor2, matches);

    //Il problema è nel sorting, bisogna selezionare i match che condividono la stessa distanza non quelli con la distanza più lunga

    std::sort(matches.begin(), matches.end(), [](const cv::DMatch& a, const cv::DMatch& b) {
        return a.distance < b.distance;
    });

    // Matcher treshold 0.1 and RANSAC treshold 3.0 seems the best choice
    int numGoodMatches = (int) (matches.size()) * 0.75;
    matches.resize(numGoodMatches);

    std::vector<cv::Point2f> points1, points2;
    for (const auto& match : matches) {
        points1.push_back(keypoints1[match.queryIdx].pt);
        points2.push_back(keypoints2[match.trainIdx].pt);
    }

    cv::Mat H = cv::findHomography(points1, points2,cv::RANSAC, 3.0);

    cv::Mat imgWarped = warpPerspectiveNoCut(newFrame, H);

    double dx = -H.at<double>(0, 2);
    double dy = H.at<double>(1, 2);

    double bordDx = dx; 
    double bordDy = dy;

    if(global_dx+dx<0){
        global_dx = 0;
    }else{
        global_dx += dx;
    }

    if(global_dy+dy>0){
        global_dy = 0;
    }else{
        global_dy += dy;
    }

    int bottom = 0, left = 0, right = 0, top = 0;

    if (bordDy > 0) {
        top = std::round(bordDy);
        bottom = 0;
    } else {
        top = 0;
        bottom = std::round(std::abs(bordDy));
    }

    if (bordDx > 0) {
        left = 0;
        right = std::round(bordDx);
    } else {
        left = std::round(std::abs(bordDx));
        right = 0;
    }

    cv::copyMakeBorder(this->map, this->map, top, bottom, left, right, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));

    int x_off = abs(round(global_dx));
    int y_off = abs(round(global_dy));


    //Parallelizae

    
    #pragma omp parallel for
    for (int i = y_off; i < imgWarped.rows + y_off; i++) {
        #pragma omp parallel for
        for (int j = x_off; j < imgWarped.cols + x_off; j++) {

            if(j<this->map.cols && i <this->map.rows){
                if (imgWarped.at<cv::Vec4b>(i - y_off, j - x_off)[3] < 255) {
                    continue;
                }

                this->map.at<cv::Vec4b>(i,j) = imgWarped.at<cv::Vec4b>(i-y_off, j-x_off);   
            }
        }
    }

    this->lastFrame = newFrame;

    //cv::imwrite("stitchres/"+"result.jpeg",this->map);

    size_t mapSize = this->map.total() * this->map.elemSize();
    double mapMegabytes = mapSize / (1024.0 * 1024.0);
    std::chrono::milliseconds endTime = duration_cast<std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch());
    double time = endTime.count()-startTime.count();
    std::cout << "Stitching done. Map MB: "<< mapMegabytes << " Time: "<< time << "ms" << std::endl;
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

    int dstWidth = static_cast<int>(maxX - minX);
    int dstHeight = static_cast<int>(maxY - minY);

    float dataShift[9] = {1, 0, -minX, 0, 1, -minY, 0, 0, 1};
    cv::Mat shiftMatrix = cv::Mat(3, 3, CV_32F, dataShift); 

    transformationMatrix.convertTo(transformationMatrix, CV_32F);

    cv::Mat adjustedMatrix;
    cv::gemm(shiftMatrix, transformationMatrix, 1.0, cv::Mat(), 0.0, adjustedMatrix);

    cv::Mat dstImage;
    cv::warpPerspective(srcImage, dstImage, adjustedMatrix, cv::Size(dstWidth, dstHeight));

    return dstImage;
}
*/