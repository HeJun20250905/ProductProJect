#include "VisionFrame.h"
#include <QDebug>

// Windows API
#define WIN32_LEAD_AND_MEAN
#include <windows.h>

#include <opencv2/core.hpp> 
#include "opencv2/imgproc.hpp"

#if defined(_MSC_VER) && (_MSC_VER >= 1600)    
# pragma execution_character_set("utf-8")    
#endif

VisionFrame& VisionFrame::instance()
{
    // 全局单例对象
    static VisionFrame instance;

    return instance;
}

int VisionFrame::VisionFrame_Init()
{
    return 0;
}

int VisionFrame::VisionFrame_Uninit()
{
    qDebug() << "[VisionFrame] 开始安全停止连接...";

    return 0;
}

void VisionFrame::UpdateVisionFrame(unsigned char* pData, int nLen)
{
    // qDebug() << "[VisionFrame] 获取图像数据" << nLen;

    QMutexLocker locker(&m_visionFrameMutex);      // 加锁保护

    if (m_visionFrame && pData) 
    {
        memcpy(m_visionFrame->data, pData, nLen);
    }
}

cv::Mat VisionFrame::GetVisionFrame() const
{
    QMutexLocker locker(&m_visionFrameMutex);      // 加锁保护
    if (m_visionFrame) 
    {
        return m_visionFrame->clone();
    }
    return cv::Mat();
}

void VisionFrame::UpdateVisionWorkFlow(const QVector<VisionWorkFlow>& workFlow)
{
    QMutexLocker locker(&m_visionWorkFlowMutex);
    m_visionWorkFlow = workFlow;
}

QVector<VisionWorkFlow> VisionFrame::GetVisionWorkFlow() const
{
    QMutexLocker locker(&m_visionWorkFlowMutex);
    return m_visionWorkFlow;
}

void VisionFrame::UpdateVisionWorkPara(const VisionWorkPara& para)
{
    QMutexLocker locker(&m_visionWorkParaMutex);
    m_visionWorkPara = para;
}

VisionWorkPara VisionFrame::GetVisionWorkPara() const
{
    QMutexLocker locker(&m_visionWorkParaMutex);
    return m_visionWorkPara;
}

VisionFrame::VisionFrame()
{
    qDebug() << "[VisionFrame] 开始初始化";
}

VisionFrame::~VisionFrame()
{
    VisionFrame_Uninit();
}

