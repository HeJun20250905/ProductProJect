#include "VisionThread.h"
#include <QDebug>

#include "VisionFrame.h"

#include "opencv2/imgproc.hpp"              // 图像处理模块

#if defined(_MSC_VER) && (_MSC_VER >= 1600)    
# pragma execution_character_set("utf-8")    
#endif

VisionThread::VisionThread(QObject* parent)
    : QObject(parent)
{
    
}

VisionThread::~VisionThread()
{
    
}

void VisionThread::doWork(const cv::Mat& image)
{
    qDebug() << "[VisionThread] doWork 收到图像，开始 SMT Mark点识别流程...";

    // 安全检查
    if (image.empty()) 
    {
        qDebug() << "[VisionThread] 识别失败：输入图像为空！请检查相机连接。";
        emit visionFinished(cv::Mat());
        return;
    }

    // 获取配置工作流
    QVector<VisionWorkFlow> currentWorkFlow = VisionFrame::instance().GetVisionWorkFlow();

    // 非空判定
    if (currentWorkFlow.isEmpty())
    {
        qDebug() << "[VisionThread] 识别失败：未指定识别工作流！";
        return;
    }

    // 深拷贝图像作为信号返回的源图
    cv::Mat resultImg = image.clone();

    // 将信号槽的图像作为工作流程图
    cv::Mat workImg = image;

    // 指向信号返回结果的指针
    cv::Mat* finalSignalImg = &workImg;

    // 重置数据状态
    m_bestIndex = -1;
    m_maxArea = 0;
    m_maxFoundArea = 0;
    m_maxFoundIndex = -1;
    m_contours.clear();

    // 获取视觉算法参数
    VisionWorkPara visionWorkPara = VisionFrame::instance().GetVisionWorkPara();

    m_gaussCoreX = visionWorkPara.m_gaussCoreX;
    m_gaussCoreY = visionWorkPara.m_gaussCoreY;
    m_gaussSigmaX = visionWorkPara.m_gaussSigmaX;
    m_gaussSigmaY = visionWorkPara.m_gaussSigmaY;
    m_thresholdValue = visionWorkPara.m_thresholdValue;

    m_minMarkArea = visionWorkPara.m_minMarkArea;
    m_maxMarkArea = visionWorkPara.m_maxMarkArea;

    m_lineWidX = visionWorkPara.m_lineWidX;
    m_lineWidY = visionWorkPara.m_lineWidY;
    m_lineLenX = visionWorkPara.m_lineLenX;
    m_lineLenY = visionWorkPara.m_lineLenY;
    m_lineColorR = visionWorkPara.m_lineColorR;
    m_lineColorG = visionWorkPara.m_lineColorG;
    m_lineColorB = visionWorkPara.m_lineColorB;

    // 遍历容器，依次调用算法
    for (int i = 0; i < currentWorkFlow.size(); i++)
    {
        switch (currentWorkFlow[i])
        {
        case VisionWorkFlow::GaussBlur:
            // 预处理，高斯滤波去反光或噪点，Size(5,5) 兼顾速度与去噪
            this->Step_GaussianBlur(workImg);
            finalSignalImg = &workImg;
            break;

        case VisionWorkFlow::Threshold:
            // 二值化（OTSU）阈值，适应氧化或反光
            this->Step_threshold(workImg);
            finalSignalImg = &workImg;
            break;

        case VisionWorkFlow::FindContour:
            // 特征提取与筛选
            this->Step_FindContours(workImg);
            finalSignalImg = &workImg;
            qDebug() << "[VisionThread] 图像预处理完成，共提取到轮廓数量:" << m_contours.size();
            break;

        case VisionWorkFlow::FilterContour:
            // 核心：面积过滤
            this->Step_FilterContours(workImg);
            finalSignalImg = &workImg;
            // 如果没找到合格的 Mark 点，直接返回
            if (m_bestIndex == -1)
            {
                qDebug() << "[VisionThread] 识别失败：未找到合格的 Mark 点！";
                qDebug() << "   -> 当前设定阈值: [" << m_minMarkArea << ", " << m_maxMarkArea << "]";
                qDebug() << "   -> 画面中最大的轮廓面积是:" << m_maxFoundArea << " (索引:" << m_maxFoundIndex << ")";

                // 精准失败原因
                if (m_contours.empty())
                {
                    qDebug() << "   -> 诊断: 画面中完全没有轮廓，请检查 光源 是否过暗，或 二值化阈值 是否过高！";
                }
                else if (m_maxFoundArea < m_minMarkArea)
                {
                    qDebug() << "   -> 诊断: 画面中最大的轮廓面积(" << m_maxFoundArea << ")小于最小阈值(" << m_minMarkArea << ")。";
                    qDebug() << "   -> 建议: 目标可能太小、反光严重导致断裂，或者 最小面积阈值 设置过高！";
                }
                else if (m_maxFoundArea > m_maxMarkArea)
                {
                    qDebug() << "   -> 诊断: 画面中最大的轮廓面积(" << m_maxFoundArea << ")大于最大阈值(" << m_maxMarkArea << ")。";
                    qDebug() << "   -> 建议: 可能把 PCB 板边缘、大面积锡膏或反光当成了目标，请调大 最大面积阈值 或增加 ROI 区域限制！";
                }
                emit visionFinished(*finalSignalImg);
                return;
            }
            break;

        case VisionWorkFlow::Locate:
            // 高精度定位，计算最小外接矩形
            this->Step_Locate(workImg);
            finalSignalImg = &workImg;
            // 打印调试信息
            qDebug() << "[VisionThread] SMT Mark点定位成功!";
            qDebug() << "  -> 中心坐标 (X, Y):" << m_center.x << "," << m_center.y;
            qDebug() << "  -> 偏转角度:" << m_angle << "度";
            qDebug() << "  -> 轮廓面积:" << m_maxArea;
            break;

        case VisionWorkFlow::Draw:
            // 在 resultImg 上画一个醒目的绿色十字
            this->Step_Draw(resultImg);
            finalSignalImg = &resultImg;
            break;

        default:
            // 遇到 None 或无效值，跳过
            break;
        }
    }

    emit visionFinished(*finalSignalImg);
}

void VisionThread::Step_GaussianBlur(cv::Mat& img)
{
    // 预处理，高斯滤波去反光或噪点，Size(5,5) 兼顾速度与去噪
    cv::GaussianBlur(img, img, cv::Size(m_gaussCoreX, m_gaussCoreY), m_gaussSigmaX, m_gaussSigmaX);
}

void VisionThread::Step_threshold(cv::Mat& img)
{
    // 二值化（OTSU）阈值，适应氧化或反光
    cv::threshold(img, img, m_thresholdValue, 255, cv::THRESH_BINARY_INV);
}

void VisionThread::Step_FindContours(cv::Mat& img)
{
    // 特征提取与筛选
    cv::findContours(img, m_contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
}

void VisionThread::Step_FilterContours(cv::Mat& img)
{
    // 筛选符合条件的轮廓
    for (size_t i = 0; i < m_contours.size(); ++i)
    {
        double area = cv::contourArea(m_contours[i]);

        // 记录全场最大的轮廓（无论是否符合条件）
        if (area > m_maxFoundArea)
        {
            m_maxFoundArea = area;
            m_maxFoundIndex = static_cast<int>(i);
        }

        if (area > m_maxArea && area >= m_minMarkArea && area <= m_maxMarkArea)
        {
            m_maxArea = area;
            m_bestIndex = static_cast<int>(i);
        }
    }
}

void VisionThread::Step_Locate(cv::Mat& img)
{
    // 坐标定位，计算最小外接矩形
    m_rect = cv::minAreaRect(m_contours[m_bestIndex]);
    m_center = m_rect.center;
    m_angle = m_rect.angle;
}

void VisionThread::Step_Draw(cv::Mat& img)
{
    // 画十字
    cv::Scalar color(m_lineColorR, m_lineColorG, m_lineColorB);
    cv::line(img, cv::Point(m_center.x - m_lineLenX, m_center.y), cv::Point(m_center.x + m_lineLenX, m_center.y), color, m_lineWidX);
    cv::line(img, cv::Point(m_center.x, m_center.y - m_lineLenY), cv::Point(m_center.x, m_center.y + m_lineLenY), color, m_lineWidY);
}

