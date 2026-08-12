#include "VisionCore.h"

#include "../../../Include/MVS/MvCameraControl.h"

#include <QDebug>
#include <QMessageBox>

#include <process.h>

// 句柄
void* handle = nullptr;

// 枚举到的设备列表
MV_CC_DEVICE_INFO_LIST stDeviceList;

// 设备信息
MV_CC_DEVICE_INFO* pDeviceInfo = nullptr;

// 设备参数
int nPktSize = 0;

// 存图像数据的内存
unsigned char* g_frameData = nullptr;

// 图像的宽高、像素格式等信息
MV_FRAME_OUT_INFO_EX g_frameInfo = { 0 };

#ifdef _DEBUG
#define  LIB_PATH     "..\\..\\..\\Library\\Win32\\Debug"
#else
#define  LIB_PATH     "..\\..\\..\\Library\\Win32\\Release"
#endif

#pragma comment(lib,  LIB_PATH   "\\MvCameraControl.lib")

VisionCore::VisionCore(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::VisionCoreClass())
{
    ui->setupUi(this);

    m_hWndDisplay = (HWND)ui->label_Image->winId();
}

VisionCore::~VisionCore()
{
    // 停止取流线程
    m_bGrabbing = false;
    if (m_hGrabThread)
    {
        WaitForSingleObject(m_hGrabThread, 3000);
        CloseHandle(m_hGrabThread);
        m_hGrabThread = nullptr;
    }

    // 清理SDK资源
    if (handle)
    {
        MV_CC_StopGrabbing(handle);
        MV_CC_CloseDevice(handle);
        MV_CC_DestroyHandle(handle);
        handle = nullptr;
    }
    MV_CC_Finalize();

    delete ui;
}

void VisionCore::on_pushButton_Init_clicked()
{
    // 初始化
    int nRet = MV_CC_Initialize();
    if (nRet != MV_OK)
    {
        QMessageBox::critical(this, "错误", QString("SDK初始化失败，错误码: 0x%1").arg(nRet, 8, 16, QChar('0')));
        return;
    }

    // 枚举设备
    memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
    nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE | MV_GENTL_GIGE_DEVICE
        | MV_GENTL_CAMERALINK_DEVICE | MV_GENTL_CXP_DEVICE | MV_GENTL_XOF_DEVICE, &stDeviceList);
    if (nRet != MV_OK)
    {
        QMessageBox::critical(this, "错误", QString("枚举设备失败，错误码: 0x%1").arg(nRet, 8, 16, QChar('0')));
        return;
    }
    if (stDeviceList.nDeviceNum == 0)
    {
        QMessageBox::information(this, "提示", "未检测到设备");
        return;  // ★ 少了这个
    }

    // 创建句柄
    handle = nullptr;
    pDeviceInfo = stDeviceList.pDeviceInfo[0];
    nRet = MV_CC_CreateHandle(&handle, pDeviceInfo);
    if (nRet != MV_OK)
    {
        QMessageBox::critical(this, "错误", QString("创建句柄失败，错误码: 0x%1").arg(nRet, 8, 16, QChar('0')));
        handle = nullptr;
        return;
    }
    // 打开设备
    nRet = MV_CC_OpenDevice(handle, MV_ACCESS_Exclusive, 0);
    if (nRet != MV_OK)
    {
        QMessageBox::critical(this, "错误", QString("打开设备失败，错误码: 0x%1").arg(nRet, 8, 16, QChar('0')));
        MV_CC_DestroyHandle(handle);
        handle = nullptr;
        return;
    }
    // 获取最佳包大小
    nPktSize = MV_CC_GetOptimalPacketSize(handle);
    if (nPktSize > 0)
    {
        // 设置最佳包大小
        MV_CC_SetIntValueEx(handle, "GevSCPSPacketSize", nPktSize);
    }

    // 设置触发模式
    nRet = MV_CC_SetEnumValue(handle, "TriggerMode", 0);
    if (nRet != MV_OK)
    {
        QMessageBox::warning(this, "警告", QString("设置触发模式失败，错误码: 0x%1").arg(nRet, 8, 16, QChar('0')));
    }
}

// 线程函数（静态或全局）
unsigned int __stdcall GrabThreadProc(void* pUser)
{
    VisionCore* pThis = (VisionCore*)pUser;
    MV_FRAME_OUT stFrame = { 0 };

    // ★ 加这里：记录上一帧的时间
    static LARGE_INTEGER s_prevTime = { 0 };
    LARGE_INTEGER freq = { 0 };
    QueryPerformanceFrequency(&freq);  // 获取CPU高频计时器频率

    while (pThis->m_bGrabbing)
    {
        int nRet = MV_CC_GetImageBuffer(handle, &stFrame, 1000);
        if (nRet == MV_OK)
        {
            // ★ 加这里：计算与上一帧的时间差
            LARGE_INTEGER now;
            QueryPerformanceCounter(&now);
            if (s_prevTime.QuadPart != 0)
            {
                double elapsedMs = (double)(now.QuadPart - s_prevTime.QuadPart) * 1000.0 / freq.QuadPart;
                qDebug() << "帧间隔:" << elapsedMs << "ms";
            }
            s_prevTime = now;

            // ★ 第一次拿到帧时分配内存
            if (g_frameData == nullptr)
            {
                g_frameData = new unsigned char[stFrame.stFrameInfo.nFrameLenEx];
            }

            // ★ 加在这里：把数据拷出来，供机器视觉处理用
            memcpy(g_frameData, stFrame.pBufAddr, stFrame.stFrameInfo.nFrameLenEx);
            memcpy(&g_frameInfo, &stFrame.stFrameInfo, sizeof(MV_FRAME_OUT_INFO_EX));

            qDebug() << "分辨率:" << g_frameInfo.nWidth << "x" << g_frameInfo.nHeight;
            
            // ★ 在这里把数据转成 cv::Mat，供机器视觉处理用
            //cv::Mat srcImage;
            //if (g_frameInfo.enPixelType == PixelType_Gvsp_Mono8)
            //{
            //    srcImage = cv::Mat(g_frameInfo.nHeight, g_frameInfo.nWidth, CV_8UC1, g_frameData);
            //}
            //else if (g_frameInfo.enPixelType == PixelType_Gvsp_RGB8_Packed)
            //{
            //    srcImage = cv::Mat(g_frameInfo.nHeight, g_frameInfo.nWidth, CV_8UC3, g_frameData);
            //}

            // SDK 直接渲染到窗口，不拷贝、不转换
            MV_CC_IMAGE stImage = { 0 };
            stImage.nWidth = stFrame.stFrameInfo.nExtendWidth;
            stImage.nHeight = stFrame.stFrameInfo.nExtendHeight;
            stImage.enPixelType = stFrame.stFrameInfo.enPixelType;
            stImage.nImageLen = stFrame.stFrameInfo.nFrameLenEx;
            stImage.pImageBuf = stFrame.pBufAddr;
            MV_CC_DisplayOneFrameEx2(handle, pThis->m_hWndDisplay, &stImage, 0);

            MV_CC_FreeImageBuffer(handle, &stFrame);
        }
    }
    return 0;
}

void VisionCore::on_pushButton_Start_clicked()
{
    if (handle == nullptr)
    {
        QMessageBox::warning(this, "", "Please First Init");
        return;
    }

    // 改成先 StartGrabbing，再开线程
    MV_CC_StartGrabbing(handle);
    // 开取流线程（_beginthreadex 方式，和 BasicDemo 一样）
    m_bGrabbing = true;
    unsigned int nThreadID = 0;
    m_hGrabThread = (void*)_beginthreadex(nullptr, 0, GrabThreadProc, this, 0, &nThreadID);
}

void VisionCore::on_pushButton_Stop_clicked()
{
    m_bGrabbing = false;           // 线程退出
    if (m_hGrabThread)
    {
        WaitForSingleObject(m_hGrabThread, 3000);
        CloseHandle(m_hGrabThread);
        m_hGrabThread = nullptr;
    }

    if (handle == nullptr)
    {
        QMessageBox::warning(this, "", "No Device Connect");
        return;
    }

    // 停止抓图
    MV_CC_StopGrabbing(handle);

    // 关闭设备
    MV_CC_CloseDevice(handle);

    // 销毁句柄
    MV_CC_DestroyHandle(handle);

    // 释放资源
    MV_CC_Finalize();
}