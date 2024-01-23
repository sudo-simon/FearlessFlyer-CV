
#include "StitcherThread.hpp"
#include "modules/BlockingQueue.hpp"
#include "modules/StateBoard.hpp"
#include <buffers.hpp>
#include <cstddef>
#include <exception>
#include <iostream>
#include <opencv2/core/mat.hpp>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <ostream>


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

    // Detect keypoints using FAST
    cv::Ptr<cv::Feature2D> fast = cv::FastFeatureDetector::create();
    std::vector<cv::KeyPoint> keypoints;
    fast->detect(newFrame, keypoints);

    // Extract descriptors
    cv::Ptr<cv::DescriptorExtractor> extractor = cv::ORB::create();
    cv::Mat newDescriptor;
    extractor->compute(newFrame, keypoints, newDescriptor);

    if(lastDescriptor.empty()){
        this->lastFrame = newFrame;
        this->lastDescriptor = newDescriptor;
        this->lastKeypoints = keypoints;
        this->map = newFrame;
        std::cout << "FIRST FRAME" << std::endl;
        return;
    }

    std::cout << "NEW FRAME" << std::endl;

    // Match keypoints using a descriptor matcher
    cv::Ptr<cv::DescriptorMatcher> matcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::BRUTEFORCE_HAMMING);
    std::vector<cv::DMatch> matches;
    matcher->match(newDescriptor, lastDescriptor, matches);

    // Filter matches using RANSAC
    std::vector<cv::Point2f> points1, points2;
    for (const cv::DMatch& match : matches) {
        points1.push_back(keypoints[match.queryIdx].pt);
        points2.push_back(lastKeypoints[match.trainIdx].pt);
    }

    cv::Mat homography = cv::findHomography(points1, points2, cv::RANSAC);
    

    //Calcolata la matrice di omografia bisogna estrarre la matrice di traslazione(?) e rotazione
    //Applicare l'omografia a newframe e appiccare newframe su map

    double dx = homography.at<double>(0, 2);
    double dy = homography.at<double>(1, 2);

    // Create the translation matrix
    cv::Mat translationMatrix = (cv::Mat_<double>(2, 3) << 1, 0, dx, 0, 1, dy);

    if(!this->lastMatrix.empty())
        translationMatrix = this->lastMatrix + translationMatrix;

    cv::Mat result;
    cv::warpAffine(newFrame, result, translationMatrix, cv::Size(0,0));

    this->map.copyTo(result(cv::Rect(0,0,this->map.cols, this->map.rows)));

    this->map = result;

    this->lastMatrix = translationMatrix;

    std::cout << "STITCHED" << std::endl;
}


inline void StitcherThread::MapBufferUpdate(){
    //cv::imwrite("test.png", this->map);
    this->mapBuffer_ptr->put(this->map);
}