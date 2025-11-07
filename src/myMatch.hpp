#pragma once
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <FL/Fl_Progress.H>

struct CMatch {
    double dist;
    cv::Point2d dstPt;
    cv::Point2d srcPt;
};


double NCC_2d (const cv::Mat &srcPatch, const cv::Mat &dstPatch);

void correlationMatch(const cv::Mat &srcImg, 
    const cv::Mat &dstImg, 
    std::vector<cv::Point2d> &srcPts,
    std::vector<cv::Point2d> &dstPts,
    std::vector<CMatch> &matches, 
    int windowSize, 
    Fl_Progress* progress
);

void lsqMatch(const cv::Mat &srcImg, 
    const cv::Mat &dstImg, 
    std::vector<CMatch> &matches, 
    std::vector<CMatch> &matchesLsq,
    int windowSize, 
    Fl_Progress* progress
);

void drawMatches(const cv::Mat &srcImg, const cv::Mat &dstImg, cv::Mat &outputImg, std::vector<CMatch> &matches);