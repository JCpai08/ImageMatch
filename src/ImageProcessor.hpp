// ImageProcessor.h
#ifndef IMAGE_PROCESSOR_H
#define IMAGE_PROCESSOR_H

#include <opencv2/opencv.hpp>
#include <opencv2/core/types.hpp>
#include <FL/Fl_Progress.H>
#include <cmath>
#include <string>

class ImageProcessor {
private:
    std::string file1_path;
    std::string file2_path;
    cv::Mat img1, img2;

public:
    std::string result;
    std::string output;
    int windowSize;
    void setFile1(const std::string& path){ file1_path = path; } 
    void setFile2(const std::string& path){ file2_path = path; }
    
    std::string getFile1() const{return file1_path;}
    std::string getFile2() const{return file2_path;}
    
    bool loadImages();
    bool processImages(Fl_Progress* progress = nullptr);
    const cv::Mat& getImage1() const{return img1;}
    const cv::Mat& getImage2() const{return img2;}
};

#endif