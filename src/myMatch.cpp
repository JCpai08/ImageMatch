#include <cmath>
#include "myMatch.hpp"

double NCC_2d (const cv::Mat &srcPatch, const cv::Mat &dstPatch) {
    CV_Assert(srcPatch.size() == dstPatch.size());
    int rows = srcPatch.rows;
    int cols = srcPatch.cols;
    double sumSrc = 0.0, sumDst = 0.0;
    double sumSrcSq = 0.0, sumDstSq = 0.0;
    double sumCross = 0.0;
    int count = 0;

    for (int r = 0; r <= rows; r++) {
        for (int c = 0; c <= cols; c++) {
            double srcVal = static_cast<double>(srcPatch.at<uchar>(r, c));
            double dstVal = static_cast<double>(dstPatch.at<uchar>(r, c));

            sumSrc += srcVal;
            sumDst += dstVal;
            sumSrcSq += srcVal * srcVal;
            sumDstSq += dstVal * dstVal;
            sumCross += srcVal * dstVal;
            count++;
        }
    }

    if (count == 0) return 0.0;
    double meanSrc = sumSrc / count;
    double meanDst = sumDst / count;

    double numerator = sumCross - count * meanSrc * meanDst;
    double denominator = std::sqrt(
        (sumSrcSq - count * meanSrc * meanSrc) *
        (sumDstSq - count * meanDst * meanDst)
    );

    if (denominator == 0) return 0.0;

    return numerator / denominator;
}

// 双线性插值函数
auto bilinearInterpolation = [](const cv::Mat& img, double x, double y) -> double {
    int x1 = static_cast<int>(std::floor(x));
    int y1 = static_cast<int>(std::floor(y));
    int x2 = x1 + 1;
    int y2 = y1 + 1;
    
    if (x1 < 0 || x2 >= img.cols || y1 < 0 || y2 >= img.rows)
        return 0;
        
    double dx = x - x1;
    double dy = y - y1;
    
    double Q11 = img.at<uchar>(y1, x1);
    double Q21 = img.at<uchar>(y1, x2);
    double Q12 = img.at<uchar>(y2, x1);
    double Q22 = img.at<uchar>(y2, x2);
    
    return Q11 * (1 - dx) * (1 - dy) + Q21 * dx * (1 - dy) + 
            Q12 * (1 - dx) * dy + Q22 * dx * dy;
};
void drawMatches(const cv::Mat &srcImg, const cv::Mat &dstImg, cv::Mat &outputImg, std::vector<CMatch> &matches)
{
    outputImg.create(cv::Size(srcImg.cols + dstImg.cols, std::max(srcImg.rows, dstImg.rows)), CV_8UC3);

    srcImg.copyTo(outputImg(cv::Rect(0, 0, srcImg.cols, srcImg.rows)));
    dstImg.copyTo(outputImg(cv::Rect(srcImg.cols, 0, dstImg.cols, dstImg.rows)));

    cv::Point pt1, pt2;
    static std::uniform_int_distribution<int> u(0, 255);

    int cnt = 0;
    for (const auto &match : matches)
    {
        cnt++;
        if (cnt > 20) break; // 最多画100条线
        cv::Scalar color(0,0,255);

        pt1 = match.srcPt;
        pt2 = cv::Point(match.dstPt.x + srcImg.cols, match.dstPt.y);

        cv::circle(outputImg, pt1, 5, color, 2);
        cv::circle(outputImg, pt2, 5, color, 2);
        cv::line(outputImg, pt1, pt2, color, 2);
    }
}

void correlationMatch(const cv::Mat &srcImg, 
    const cv::Mat &dstImg, 
    std::vector<cv::Point2d> &srcPts,
    std::vector<cv::Point2d> &dstPts,
    std::vector<CMatch> &matches, 
    int windowSize,
    Fl_Progress* progress
    ) { 
    int cnt(0),total(srcPts.size());
    int halfWindow = windowSize / 2;
    for (const auto &srcPt : srcPts) {
        cnt++;
        if (progress != nullptr)
            progress->value(cnt*100/total);
            Fl::check();
        cv::Rect srcRect(srcPt.x - halfWindow, srcPt.y - halfWindow, windowSize,windowSize);
        if (srcRect.x < 0 || srcRect.y < 0 || srcRect.x + srcRect.width >= srcImg.cols || srcRect.y + srcRect.height >= srcImg.rows)
            continue;
        cv::Mat srcPatch = srcImg(srcRect);

        double bestNCC = -1.0;
        cv::Point2d bestDstPt;

        for (const auto &dstPt : dstPts) {
            cv::Rect dstRect(dstPt.x - halfWindow, dstPt.y - halfWindow, windowSize, windowSize);
            if (dstRect.x < 0 || dstRect.y < 0 || dstRect.x + dstRect.width >= dstImg.cols || dstRect.y + dstRect.height >= dstImg.rows)
                continue;
            cv::Mat dstPatch = dstImg(dstRect);

            double nccValue = NCC_2d(srcPatch, dstPatch);
            if (nccValue > bestNCC) {
                bestNCC = nccValue;
                bestDstPt = dstPt;
            }
        }

        if (bestNCC > 0.8) { // 阈值可以调整
            CMatch match;
            match.dist = bestNCC;
            match.srcPt = srcPt;
            match.dstPt = bestDstPt;
            matches.push_back(match);
        }
    }
}

void lsqMatch(const cv::Mat &srcImg, 
    const cv::Mat &dstImg, 
    std::vector<CMatch> &matches, 
    std::vector<CMatch> &matchesLsq,
    int windowSize,
    Fl_Progress* progress) {
    
    int total = matches.size();
    int halfWindow = windowSize / 2;

    for (int i = 0; i < total; i++) {
        if (progress != nullptr) {
            progress->value((i + 1) * 100 / total);
            Fl::check();
        }
        
        cv::Point2d srcPt = matches[i].srcPt;
        cv::Point2d dstPt = matches[i].dstPt;
        
        // 初始化参数
        double x1{srcPt.x}, y1{srcPt.y}, x2{dstPt.x}, y2{dstPt.y};
        
        // 仿射变换参数初始化
        double a0 = x2 - x1;
        double a1 = 1.0;
        double a2 = 0.0;
        double b0 = y2 - y1;
        double b1 = 0.0;
        double b2 = 1.0;
        
        // 辐射校正参数初始化
        double h0 = 0.0;
        double h1 = 1.0;
        
        // 相关系数
        double formerCoefficient = 0.0;
        double latterCoefficient = 0.0;
        
        // 最佳匹配点位
        double formerResX = 0.0;
        double formerResY = 0.0;
        double rightResX = 0.0;
        double rightResY = 0.0;
        
        bool firstIteration = true;
        int cnt{0};
        // 迭代优化
        for(;; cnt++) {
            // 结束条件
            if (cnt > 10) break;
            if ((!firstIteration) && (latterCoefficient < formerCoefficient)) break;
            if (!firstIteration) {
                formerResX = rightResX;
                formerResY = rightResY;
                formerCoefficient = latterCoefficient;
            } else {
                firstIteration = false;
            }
            
            // 计算因子
            double factor1 = 0.0;
            double factor1_2 = 0.0;
            double factor2 = 0.0;
            double factor2_2 = 0.0;
            
            // 创建矩阵
            cv::Mat matA = cv::Mat::zeros(windowSize * windowSize, 8, CV_64F);
            cv::Mat matL = cv::Mat::zeros(windowSize * windowSize, 1, CV_64F);
            cv::Mat dstPatch = cv::Mat::zeros(windowSize, windowSize, CV_64F);
            
            int startX = static_cast<int>(x1 - halfWindow);
            int startY = static_cast<int>(y1 - halfWindow);

            cv::Mat srcPatch = srcImg(cv::Rect(startX, startY, windowSize, windowSize));
            
            // 填充矩阵
            for (int m = 0; m < windowSize; m++) {
                for (int n = 0; n < windowSize; n++) {
                    int x_1 = startX + n;
                    int y_1 = startY + m;
                    
                    if (x_1 < 1 || x_1 >= srcImg.cols - 1 || y_1 < 1 || y_1 >= srcImg.rows - 1)
                        continue;
                    
                    double newRX = a0 + a1 * x_1 + a2 * y_1;
                    double newRY = b0 + b1 * x_1 + b2 * y_1;
                    
                    // 双线性插值
                    double newRGray = bilinearInterpolation(dstImg, newRX, newRY);
                    
                    // 辐射校正
                    double radioRGrey = newRGray * h1 + h0;
                    dstPatch.at<double>(m, n) = radioRGrey;
                    
                    // 检查边界
                    if (newRY < 1 || newRY >= dstImg.rows - 1 || newRX < 1 || newRX >= dstImg.cols - 1)
                        continue;
                    
                    // 计算梯度
                    double dgx = (static_cast<double>(dstImg.at<uchar>(static_cast<int>(newRY), static_cast<int>(newRX) + 1)) - 
                                 static_cast<double>(dstImg.at<uchar>(static_cast<int>(newRY), static_cast<int>(newRX) - 1))) / 2.0;
                    double dgy = (static_cast<double>(dstImg.at<uchar>(static_cast<int>(newRY) + 1, static_cast<int>(newRX))) - 
                                 static_cast<double>(dstImg.at<uchar>(static_cast<int>(newRY) - 1, static_cast<int>(newRX)))) / 2.0;
                    
                    double leftDgx = (static_cast<double>(srcImg.at<uchar>(y_1, x_1 + 1)) - 
                                     static_cast<double>(srcImg.at<uchar>(y_1, x_1 - 1))) / 2.0;
                    double leftDgy = (static_cast<double>(srcImg.at<uchar>(y_1 + 1, x_1)) - 
                                     static_cast<double>(srcImg.at<uchar>(y_1 - 1, x_1))) / 2.0;
                    
                    factor1 += x_1 * leftDgx * leftDgx;
                    factor1_2 += leftDgx * leftDgx;
                    factor2 += y_1 * leftDgy * leftDgy;
                    factor2_2 += leftDgy * leftDgy;
                    
                    // 填充系数矩阵
                    int idx = m * windowSize + n;
                    matA.at<double>(idx, 0) = 1.0;
                    matA.at<double>(idx, 1) = newRGray;
                    matA.at<double>(idx, 2) = dgx;
                    matA.at<double>(idx, 3) = newRX * dgx;
                    matA.at<double>(idx, 4) = newRY * dgx;
                    matA.at<double>(idx, 5) = dgy;
                    matA.at<double>(idx, 6) = newRX * dgy;
                    matA.at<double>(idx, 7) = newRY * dgy;
                    
                    // 填充常数项矩阵
                    double lGrey = static_cast<double>(srcImg.at<uchar>(y_1, x_1));
                    matL.at<double>(idx, 0) = lGrey - radioRGrey;
                }
            }
            
            // 求解最小二乘问题
            cv::Mat matAt = matA.t();
            cv::Mat matAtA = matAt * matA;
            cv::Mat matAtL = matAt * matL;
            cv::Mat matX;
            
            // 使用SVD求解避免奇异矩阵问题
            cv::solve(matAtA, matAtL, matX, cv::DECOMP_SVD);
            
            // 提取增量
            double dh0 = matX.at<double>(0, 0);
            double dh1 = matX.at<double>(1, 0);
            double da0 = matX.at<double>(2, 0);
            double da1 = matX.at<double>(3, 0);
            double da2 = matX.at<double>(4, 0);
            double db0 = matX.at<double>(5, 0);
            double db1 = matX.at<double>(6, 0);
            double db2 = matX.at<double>(7, 0);
            
            // 更新参数
            h0 = h0 + dh0 + h0 * dh1;
            h1 = h1 + h1 * dh1;
            a0 = a0 + da0 + a0 * da1 + b0 * da2;
            a1 = a1 + a1 * da1 + b1 * da2;
            a2 = a2 + a2 * da1 + b2 * da2;
            b0 = b0 + db0 + a0 * db1 + b0 * db2;
            b1 = b1 + a1 * db1 + b1 * db2;
            b2 = b2 + a2 * db1 + b2 * db2;
            
            // 计算相关系数
            latterCoefficient = NCC_2d(srcPatch, dstPatch);
            
            // 计算最佳匹配点
            double leftResX = factor1 / factor1_2;
            double leftResY = factor2 / factor2_2;
            rightResX = a0 + a1 * leftResX + a2 * leftResY;
            rightResY = b0 + b1 * leftResX + b2 * leftResY;
            
        }
        
        // 保存结果
        if (latterCoefficient > 0.8) { // 设置阈值
            CMatch matchLsq;
            matchLsq.dist = latterCoefficient;
            matchLsq.srcPt = srcPt;
            matchLsq.dstPt = cv::Point2d(rightResX, rightResY);
            matchesLsq.push_back(matchLsq);
        }
    }
}

