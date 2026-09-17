#pragma once
#include <QObject>

#include <vector>

#include <opencv2/core.hpp> 

// 状态返回码
enum class WorkFlowStatus 
{
    Success = 0,           // 全部成功
    EmptyWorkFlow,         // 工作流为空
    StepFailed_GaussBlur,  // 高斯滤波失败
    StepFailed_Threshold,  // 二值化失败
    StepFailed_FindContour,// 找轮廓失败
    StepFailed_Filter,     // 轮廓过滤失败
    StepFailed_Locate,     // 定位失败
    StepFailed_Draw        // 绘制失败
};

class VisionThread : public QObject
{
    Q_OBJECT
public:
    VisionThread(QObject* parent = nullptr);
    ~VisionThread();

    void doWork(const cv::Mat& image);

    // cv::Mat图像处理模块
    void Step_GaussianBlur(cv::Mat& img);               // 预处理滤波
    void Step_threshold(cv::Mat& img);                  // 预处理二值化
    void Step_FindContours(cv::Mat& img);               // 特征提取（找轮廓）
    void Step_FilterContours(cv::Mat& img);             // 核心筛选（面积过滤）
    void Step_Locate(cv::Mat& img);                     // 坐标定位
    void Step_Draw(cv::Mat& img);                       // 绘制

private:
    // 高斯模糊参数
    int m_gaussCoreX = 5;                               // 高斯核X
    int m_gaussCoreY = 5;                               // 高斯核Y
    double m_gaussSigmaX = 1.5;                         // 高斯标准差X
    double m_gaussSigmaY = 1.5;                         // 高斯标准差Y

    // 二值化参数
    int m_thresholdValue = 127;                         // 二值化的阈值

    // 轮廓集合
    std::vector<std::vector<cv::Point>> m_contours;     // 保存所有找到的轮廓的容器

    // 筛选轮廓参数
    double m_maxFoundArea = 0;                          // 记录全场最大轮廓的面积
    int m_maxFoundIndex = -1;                           // 记录全场最大轮廓的索引
    double m_minMarkArea = 1500.0;                      // 设置最小面积阈值
    double m_maxMarkArea = 5000.0;                      // 设置最大面积阈值
    double m_maxArea = 0;                               // 找到的面积阈值内最合适的轮廓 
    int m_bestIndex = -1;                               // 找到的面积阈值内最合适的轮廓的index

    // 定位最佳矩形参数
    cv::RotatedRect m_rect;                             // 最佳矩形
    cv::Point2d     m_center;                           // 最佳矩形的中心点
    double m_angle = 0.0;                               // 最佳矩形的角度

    // 画图参数
    int m_lineWidX = 2;                                 // 线宽X
    int m_lineWidY = 2;                                 // 线宽Y
    int m_lineLenX = 50;                                // 线距X
    int m_lineLenY = 50;                                // 线距Y
    int m_lineColorR = 0;                               // 线色R
    int m_lineColorG = 0;                               // 线色G
    int m_lineColorB = 0;                               // 线色B

signals:
    void visionFinished(const cv::Mat& resultImage);
};

