#include <opencv2/opencv.hpp>

std::vector<cv::Point2d> myHarrisDetector(cv::Mat img)
{
    // 读取图像并转换为灰度图
    cv::Mat gray;
    if (img.channels() > 1) {
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = img.clone();
    }
    
    // 转换为浮点型 避免溢出
    cv::Mat floatImg;
    gray.convertTo(floatImg, CV_32F);
    
    // 定义梯度算子
    cv::Mat filterX = (cv::Mat_<float>(3, 3) << 1, 0, -1,
                                               2, 0, -2,
                                               1, 0, -1);
    
    cv::Mat filterY = (cv::Mat_<float>(3, 3) << 1, 2, 1,
                                               0, 0, 0,
                                              -1, -2, -1);
    
    // 计算图像梯度
    cv::Mat Ix, Iy;
    cv::filter2D(floatImg, Ix, -1, filterX);
    cv::filter2D(floatImg, Iy, -1, filterY);
    
    // 计算梯度乘积
    cv::Mat Ixy, Ixx, Iyy;
    cv::multiply(Ix, Iy, Ixy);
    cv::multiply(Ix, Ix, Ixx);
    cv::multiply(Iy, Iy, Iyy);
    
    // 高斯模糊
    // 这个似乎比较重要 也可以换其他核尝试 均值
    cv::GaussianBlur(Ixy, Ixy, cv::Size(3, 3), 0.5);
    cv::GaussianBlur(Ixx, Ixx, cv::Size(3, 3), 0.5);
    cv::GaussianBlur(Iyy, Iyy, cv::Size(3, 3), 0.5);
    
    // 计算行列式和迹
    cv::Mat det_M, trace_M;
    cv::Mat Ixx_Iyy;
    cv::multiply(Ixx, Iyy, Ixx_Iyy);
    cv::multiply(Ixy, Ixy, Ixy);
    cv::subtract(Ixx_Iyy, Ixy, det_M);
    
    cv::add(Ixx, Iyy, trace_M);
    
    // Harris响应计算
    float k = 0.04;
    cv::Mat trace_sq, k_trace_sq;
    cv::multiply(trace_M, trace_M, trace_sq);
    cv::multiply(trace_sq, cv::Scalar::all(k), k_trace_sq);
    
    cv::Mat R;
    cv::subtract(det_M, k_trace_sq, R);
    
    int window_size = 5;
    cv::Mat kernel = cv::Mat::ones(window_size, window_size, CV_8UC1);

    // 2. 使用膨胀操作进行最大值滤波
    cv::Mat R_dilated;
    cv::dilate(R, R_dilated, kernel);

    // 3. 找出局部最大值点
    // 只有当原始响应值等于膨胀后的值时，才是局部最大值
    cv::Mat local_max_mask;
    cv::compare(R, R_dilated, local_max_mask, cv::CMP_EQ); // 比较结果是一个CV_8UC1的掩码

    // 4. 应用响应值阈值，过滤弱角点
    // 找到响应图中的最大值，用于计算动态阈值
    double max_val;
    cv::minMaxLoc(R, nullptr, &max_val);
    
    // 创建一个阈值掩码
    cv::Mat threshold_mask;
    cv::threshold(R, threshold_mask, 0.01f * max_val, 1, cv::THRESH_BINARY);
    threshold_mask.convertTo(threshold_mask, CV_8UC1);

    // 5. 结合局部最大值和阈值，得到最终的角点掩码
    cv::Mat corners_nms;
    cv::bitwise_and(local_max_mask, threshold_mask, corners_nms);

    // img = img * 0.5 + corners_nms * 255;
    // cv::imwrite("result.jpg", img);
    
    // 提取角点坐标
    std::vector<cv::Point2d> cornerPoints;
    for (int i = 0; i < corners_nms.rows; i++) {
        for (int j = 0; j < corners_nms.cols; j++) {
            if (corners_nms.at<uchar>(i, j) == 1) {
                cornerPoints.push_back(cv::Point2d(j, i));
            }
        }
    }
    
    return cornerPoints;
}