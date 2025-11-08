// ImageProcessor.cpp
#include "ImageProcessor.hpp"
#include "myMatch.hpp"
#include <sstream>
#include <chrono>
#include <iostream>

std::vector<cv::Point2d> myHarrisDetector(cv::Mat img);
bool ImageProcessor::loadImages() {
    img1 = cv::imread(file1_path);
    img2 = cv::imread(file2_path);
    return !img1.empty() && !img2.empty();
}

bool ImageProcessor::processImages(Fl_Progress* progress) {
    if (file1_path.empty() || file2_path.empty()) {
        output = "请先选择两个文件";
        return false;
    }
    if (!loadImages()) {
        output = "无法读取图像文件";
        return false;
    }
    
    try {
        progress->label("Harris角点检测中...");
        Fl::check();
        cv::Mat img_gray1, img_gray2;
        cv::cvtColor(img1, img_gray1, cv::COLOR_BGR2GRAY);
        cv::cvtColor(img2, img_gray2, cv::COLOR_BGR2GRAY);

        auto t = std::chrono::high_resolution_clock::now();
        std::vector<cv::Point2d> corners1 = myHarrisDetector(img_gray1);
        std::vector<cv::Point2d> corners2 = myHarrisDetector(img_gray2);
        // cv::cornerHarris(img_gray1, img_gray1,5, 3, 0.04);
        // cv::cornerHarris(img_gray2, img_gray2,5, 3, 0.04);
        auto end_t = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration<double>(end_t - t);
        double harris_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

        progress->label("匹配中...");
        Fl::check();
        t = std::chrono::high_resolution_clock::now();
        std::vector<CMatch> matches;
        correlationMatch(img_gray1, img_gray2, corners1, corners2, matches, windowSize, progress);
        end_t = std::chrono::high_resolution_clock::now();
        elapsed = std::chrono::duration<double>(end_t - t);
        double match_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
        std::sort(matches.begin(), matches.end(), 
          [](const CMatch& a, const CMatch& b) {
              return a.dist > b.dist;
          });
        
        progress->label("单点最小二乘匹配优化中...");
        Fl::check();
        t = std::chrono::high_resolution_clock::now();
        std::vector<CMatch> matchesLsq;
        lsqMatch(img_gray1, img_gray2, matches,matchesLsq, windowSize, progress);
        end_t = std::chrono::high_resolution_clock::now();
        elapsed = std::chrono::duration<double>(end_t - t);
        double lsq_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

        progress->label("匹配计算完成!");
        Fl::check();
        
        std::ostringstream oss;
        oss << "Harris角点检测耗时: " << harris_time_ms << "ms" << std::endl;
        oss << "匹配耗时: " << match_time_ms << "ms" << std::endl;
        oss << "匹配结果: " << matches.size() << "对特征点" << std::endl;
        oss << "单点最小二乘匹配优化耗时: " << lsq_time_ms << "ms" << std::endl;
        oss << "单点最小二乘匹配结果: " << matchesLsq.size() << "对特征点" << std::endl;
        output = oss.str();
        std::ostringstream oss_result;
        int n = 0;
        for (auto& match : matchesLsq) {
            // n++;
            // if (n > 10) break; // 只显示前10个匹配点
            oss_result << match.srcPt.x << ", " << match.srcPt.y <<", "<< match.dstPt.x << ", " << match.dstPt.y << std::endl;
            oss_result << "dist: " << match.dist << std::endl;
        }
        result = oss_result.str();
        cv::Mat img_match;
        drawMatches(img1, img2, img_match, matchesLsq);
        cv::imwrite("match.png", img_match);
        return true;
    } catch (const std::exception& e) {
        output = "计算出错: " + std::string(e.what());
        return false;
    }
}
