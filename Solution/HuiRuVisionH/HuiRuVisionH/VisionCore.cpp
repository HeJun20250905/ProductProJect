#include "VisionCore.h"
#include <QDebug>

#include <process.h>        // 线程

#ifdef _DEBUG
#define  LIB_PATH     "..\\..\\..\\Library\\Win32\\Debug"
#else
#define  LIB_PATH     "..\\..\\..\\Library\\Win32\\Release"
#endif

#pragma comment(lib,  LIB_PATH   "\\MvCameraControl.lib")

#if defined(_MSC_VER) && (_MSC_VER >= 1600)    
# pragma execution_character_set("utf-8")    
#endif

VisionCore& VisionCore::instance()
{
    // 全局单例对象
    static VisionCore instance;

    return instance;
}

int VisionCore::VisionCore_Init(HWND hWndDisplay)
{
    // 显示窗口句柄
    m_display = hWndDisplay;

    // 初始化相机SDK库
    int result = MV_CC_Initialize();
    if (result != MV_OK)
    {
        qDebug() << "[VisionCore] 初始化相机SDK库失败，错误码: 0x" << result << Qt::hex << result;
        return 1;
    }

    // 初始化枚举列表
    memset(&m_deviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
    // 枚举设备
    result = MV_CC_EnumDevices(
        MV_GIGE_DEVICE | 
        MV_USB_DEVICE | 
        MV_GENTL_GIGE_DEVICE | 
        MV_GENTL_CAMERALINK_DEVICE | 
        MV_GENTL_CXP_DEVICE | 
        MV_GENTL_XOF_DEVICE, 
        &m_deviceList
    );
    // 枚举结果判定
    if (result != MV_OK)
    {
        qDebug() << "[VisionCore] 枚举设备失败，错误码: 0x" << result << Qt::hex << result;
        return 1;
    }
    // 枚举结果设备数量判定
    if (m_deviceList.nDeviceNum == 0)
    {
        qDebug() << "[VisionCore] 未检测到设备";
        return 1;
    }

    // 初始化相机句柄
    m_handle = nullptr;
    m_deviceInfo = m_deviceList.pDeviceInfo[0];

    // 绑定相机句柄
    result = MV_CC_CreateHandle(&m_handle, m_deviceInfo);
    if (result != MV_OK)
    {
        qDebug() << "[VisionCore] 创建句柄失败，错误码: 0x" << result << Qt::hex << result;
        m_handle = nullptr;
        return 1;
    }

    // 打开设备
    result = MV_CC_OpenDevice(m_handle, MV_ACCESS_Exclusive, 0);
    if (result != MV_OK)
    {
        qDebug() << "[VisionCore] 打开设备失败，错误码: 0x" << result << Qt::hex << result;
        MV_CC_DestroyHandle(m_handle);
        m_handle = nullptr;
        return 1;
    }

    // 获取最佳包大小
    m_packetSize = MV_CC_GetOptimalPacketSize(m_handle);
    if (m_packetSize > 0)
    {
        // 设置最佳包大小
        MV_CC_SetIntValueEx(m_handle, "GevSCPSPacketSize", m_packetSize);
    }

    // 设置触发模式
    result = MV_CC_SetEnumValue(m_handle, "TriggerMode", 0);
    if (result != MV_OK)
    {
        qDebug() << "[VisionCore] 设置触发模式失败，错误码: 0x" << result << Qt::hex << result;
    }

    return 0;
}

int VisionCore::VisionCore_Uninit()
{
    qDebug() << "[VisionCore] 开始安全停止连接...";

    // 停止取流线程
    m_isGrabbing = false;           // 线程退出

    if (m_grabThread)
    {
        WaitForSingleObject(m_grabThread, 3000);
        CloseHandle(m_grabThread);
        m_grabThread = nullptr;
    }

    // 释放图像内存，防止下次 Start 时内存泄漏
    {
        QMutexLocker locker(&m_mutex);
        if (m_frameData != nullptr)
        {
            delete[] m_frameData;
            m_frameData = nullptr;
            m_nFrameDataSize = 0;
        }
    }

    // 清理SDK资源
    if (m_handle)
    {
        MV_CC_StopGrabbing(m_handle);
        MV_CC_CloseDevice(m_handle);
        MV_CC_DestroyHandle(m_handle);
        m_handle = nullptr;
    }

    MV_CC_Finalize();

    return 0;
}

// 线程函数
unsigned int __stdcall GrabThreadProc(void* pUser)
{
    VisionCore* pThis = (VisionCore*)pUser;

    // 初始化图像数据结构体
    MV_FRAME_OUT stFrame = { 0 };

    //// ★ 加这里：记录上一帧的时间
    //static LARGE_INTEGER s_prevTime = { 0 };
    //LARGE_INTEGER freq = { 0 };
    //QueryPerformanceFrequency(&freq);  // 获取CPU高频计时器频率

    while (pThis->m_isGrabbing)
    {
        int result = MV_CC_GetImageBuffer(pThis->m_handle, &stFrame, 1000);
        if (result == MV_OK)
        {
            //// ★ 加这里：计算与上一帧的时间差
            //LARGE_INTEGER now;
            //QueryPerformanceCounter(&now);
            //if (s_prevTime.QuadPart != 0)
            //{
            //    double elapsedMs = (double)(now.QuadPart - s_prevTime.QuadPart) * 1000.0 / freq.QuadPart;
            //    qDebug() << "帧间隔:" << elapsedMs << "ms";
            //}
            //s_prevTime = now;

            // ★ 第一次拿到帧时分配内存
            if (pThis->m_frameData == nullptr)
            {
                pThis->m_nFrameDataSize = stFrame.stFrameInfo.nFrameLenEx;
                pThis->m_frameData = new unsigned char[pThis->m_nFrameDataSize];
            }

            // ★ 加在这里：把数据拷出来，供机器视觉处理用
            {
                QMutexLocker locker(&pThis->m_mutex);
                memcpy(pThis->m_frameData, stFrame.pBufAddr, stFrame.stFrameInfo.nFrameLenEx);
                memcpy(&pThis->m_frameInfo, &stFrame.stFrameInfo, sizeof(MV_FRAME_OUT_INFO_EX));
            }
            
            qDebug() << "分辨率:" << pThis->m_frameInfo.nWidth << "x" << pThis->m_frameInfo.nHeight;
            
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
            MV_CC_DisplayOneFrameEx2(pThis->m_handle, pThis->m_display, &stImage, 0);

            MV_CC_FreeImageBuffer(pThis->m_handle, &stFrame);
        }
    }
    return 0;
}

int VisionCore::VisionCore_Start()
{
    // 安全判定
    if (m_handle == nullptr)
    {
        qDebug() << "[VisionCore] Please First Init";
        return 1;
    }

    // 先开启图像采集
    MV_CC_StartGrabbing(m_handle);

    // 再开启取流线程
    m_isGrabbing = true;
    unsigned int nThreadID = 0;
    m_grabThread = (void*)_beginthreadex(nullptr, 0, GrabThreadProc, this, 0, &nThreadID);

    return 0;
}

int VisionCore::VisionCore_Stop()
{
    this->VisionCore_Uninit();

    return 0;
}


// --- 曝光时间 ---
int VisionCore::VisionCore_SetExposureTime(float exposureUs)
{
    if (m_handle == nullptr)
    {
        qDebug() << "[VisionCore] SetExposureTime Please First Init";
        return 1;
    }
    return MV_CC_SetFloatValue(m_handle, "ExposureTime", exposureUs);
}

int VisionCore::VisionCore_GetExposureTime(float& exposureUs)
{
    if (m_handle == nullptr)
    {
        qDebug() << "[VisionCore] GetExposureTime Please First Init";
        return 1;
    }
    MVCC_FLOATVALUE floatValue = { 0 };
    int ret = MV_CC_GetFloatValue(m_handle, "ExposureTime", &floatValue);
    if (ret == MV_OK) 
    {
        exposureUs = floatValue.fCurValue;
        qDebug() << "[VisionCore] 曝光时间:" << exposureUs << "us";
    }
    return ret;
}

// --- 自动曝光 ---
int VisionCore::VisionCore_SetExposureAuto(int mode)
{
    if (m_handle == nullptr)
    {
        qDebug() << "[VisionCore] SetExposureAut Please First Init";
        return 1;
    }
    return MV_CC_SetEnumValue(m_handle, "ExposureAuto", mode);
}

int VisionCore::VisionCore_GetExposureAuto(int& mode)
{
    if (m_handle == nullptr)
    {
        qDebug() << "[VisionCore] GetExposureAuto Please First Init";
        return 1;
    }
    MVCC_ENUMVALUE enumValue = { 0 };
    int ret = MV_CC_GetEnumValue(m_handle, "ExposureAuto", &enumValue);
    if (ret == MV_OK) 
    {
        mode = enumValue.nCurValue;
        qDebug() << "[VisionCore] 自动曝光:" << mode;
    }
    return ret;
}

// --- 增益 ---
int VisionCore::VisionCore_SetGain(float gainDb)
{
    if (m_handle == nullptr)
    {
        qDebug() << "[VisionCore] SetGain Please First Init";
        return 1;
    }
    
    return MV_CC_SetFloatValue(m_handle, "Gain", gainDb);
}

int VisionCore::VisionCore_GetGain(float& gainDb)
{
    if (m_handle == nullptr)
    {
        qDebug() << "[VisionCore] GetGain Please First Init";
        return 1;
    }
    
    MVCC_FLOATVALUE floatValue = { 0 };
    int ret = MV_CC_GetFloatValue(m_handle, "Gain", &floatValue);
    if (ret == MV_OK) 
    {
        gainDb = floatValue.fCurValue;
        qDebug() << "[VisionCore] 增益:" << gainDb << "dB";
    }
    return ret;
}

// --- 自动增益 ---
int VisionCore::VisionCore_SetGainAuto(int mode)
{
    if (m_handle == nullptr)
    {
        qDebug() << "[VisionCore] SetGainAuto Please First Init";
        return 1;
    }
        
    return MV_CC_SetEnumValue(m_handle, "GainAuto", mode);
}

int VisionCore::VisionCore_GetGainAuto(int& mode)
{
    if (m_handle == nullptr)
    {
        qDebug() << "[VisionCore] GetGainAuto Please First Init";
        return 1;
    }
    MVCC_ENUMVALUE enumValue = { 0 };
    int ret = MV_CC_GetEnumValue(m_handle, "GainAuto", &enumValue);
    if (ret == MV_OK) 
    {
        mode = enumValue.nCurValue;
        qDebug() << "[VisionCore] 自动增益:" << mode;
    }
    return ret;
}

// --- 亮度 ---
int VisionCore::VisionCore_SetBrightness(int brightness)
{
    if (m_handle == nullptr)
    {
        qDebug() << "[VisionCore] SetBrightness Please First Init";
        return 1;
    }
    return MV_CC_SetIntValueEx(m_handle, "Brightness", brightness);
}

int VisionCore::VisionCore_GetBrightness(int& brightness)
{
    if (m_handle == nullptr)
    {
        qDebug() << "[VisionCore] GetBrightness Please First Init";
        return 1;
    }
    MVCC_INTVALUE_EX intValue = { 0 };
    int ret = MV_CC_GetIntValueEx(m_handle, "Brightness", &intValue);
    if (ret == MV_OK) 
    {
        brightness = (int)intValue.nCurValue;
        qDebug() << "[VisionCore] 亮度:" << brightness;
    }
    return ret;
}

// --- 对比度 ---
int VisionCore::VisionCore_SetContrast(int contrast)
{
    if (m_handle == nullptr)
    {
        qDebug() << "[VisionCore] SetContrast Please First Init";
        return 1;
    }
    return MV_CC_SetIntValueEx(m_handle, "Contrast", contrast);
}

int VisionCore::VisionCore_GetContrast(int& contrast)
{
    if (m_handle == nullptr)
    {
        qDebug() << "[VisionCore] GetContrast Please First Init";
        return 1;
    }
    MVCC_INTVALUE_EX intValue = { 0 };
    int ret = MV_CC_GetIntValueEx(m_handle, "Contrast", &intValue);
    if (ret == MV_OK) 
    {
        contrast = (int)intValue.nCurValue;
        qDebug() << "[VisionCore] 对比度:" << contrast;
    }
    return ret;
}

// --- 伽马使能 ---
int VisionCore::VisionCore_SetGammaEnable(bool enable)
{
    if (m_handle == nullptr)
    {
        qDebug() << "[VisionCore] SetGammaEnable Please First Init";
        return 1;
    }
    return MV_CC_SetBoolValue(m_handle, "GammaEnable", enable ? 1 : 0);
}

int VisionCore::VisionCore_GetGammaEnable(bool& enable)
{
    if (m_handle == nullptr)
    {
        qDebug() << "[VisionCore] GetGammaEnable Please First Init";
        return 1;
    }
    bool bValue = false;
    int ret = MV_CC_GetBoolValue(m_handle, "GammaEnable", &bValue);
    if (ret == MV_OK) 
    {
        enable = bValue;
        qDebug() << "[VisionCore] 伽马使能:" << enable;
    }
    return ret;
}

// --- 伽马值 ---
int VisionCore::VisionCore_SetGamma(float gamma)
{
    if (m_handle == nullptr)
    {
        qDebug() << "[VisionCore] SetGamma Please First Init";
        return 1;
    }
    return MV_CC_SetFloatValue(m_handle, "Gamma", gamma);
}

int VisionCore::VisionCore_GetGamma(float& gamma)
{
    if (m_handle == nullptr)
    {
        qDebug() << "[VisionCore] GetGamma Please First Init";
        return 1;
    }
    MVCC_FLOATVALUE floatValue = { 0 };
    int ret = MV_CC_GetFloatValue(m_handle, "Gamma", &floatValue);
    if (ret == MV_OK) 
    {
        gamma = floatValue.fCurValue;
        qDebug() << "[VisionCore] 伽马值:" << gamma;
    }
    return ret;
}

// --- 自动白平衡 ---
int VisionCore::VisionCore_SetWhiteBalanceAuto(int mode)
{
    if (m_handle == nullptr)
    {
        qDebug() << "[VisionCore] SetWhiteBalanceAuto Please First Init";
        return 1;
    }
    return MV_CC_SetEnumValue(m_handle, "BalanceWhiteAuto", mode);
}

int VisionCore::VisionCore_GetWhiteBalanceAuto(int& mode)
{
    if (m_handle == nullptr)
    {
        qDebug() << "[VisionCore] GetWhiteBalanceAuto Please First Init";
        return 1;
    }
    MVCC_ENUMVALUE enumValue = { 0 };
    int ret = MV_CC_GetEnumValue(m_handle, "BalanceWhiteAuto", &enumValue);
    if (ret == MV_OK) 
    {
        mode = enumValue.nCurValue;
        qDebug() << "[VisionCore] 自动白平衡:" << mode;
    }
    return ret;
}

// --- 采集帧率 ---
int VisionCore::VisionCore_SetAcquisitionFrameRate(float frameRate)
{
    if (m_handle == nullptr)
    {
        qDebug() << "[VisionCore] SetAcquisitionFrameRate Please First Init";
        return 1;
    }
    return MV_CC_SetFloatValue(m_handle, "AcquisitionFrameRate", frameRate);
}

int VisionCore::VisionCore_GetAcquisitionFrameRate(float& frameRate)
{
    if (m_handle == nullptr)
    {
        qDebug() << "[VisionCore] GetAcquisitionFrameRate Please First Init";
        return 1;
    }
    MVCC_FLOATVALUE floatValue = { 0 };
    int ret = MV_CC_GetFloatValue(m_handle, "AcquisitionFrameRate", &floatValue);
    if (ret == MV_OK) 
    {
        frameRate = floatValue.fCurValue;
        qDebug() << "[VisionCore] 采集帧率:" << frameRate;
    }
    return ret;
}

// --- 像素格式 ---
int VisionCore::VisionCore_SetPixelFormat(unsigned int pixelFormat)
{
    if (m_handle == nullptr)
    {
        qDebug() << "[VisionCore] SetPixelFormat Please First Init";
        return 1;
    }
    return MV_CC_SetEnumValue(m_handle, "PixelFormat", pixelFormat);
}

int VisionCore::VisionCore_GetPixelFormat(unsigned int& pixelFormat)
{
    if (m_handle == nullptr)
    {
        qDebug() << "[VisionCore] GetPixelFormat Please First Init";
        return 1;
    }
    MVCC_ENUMVALUE enumValue = { 0 };
    int ret = MV_CC_GetEnumValue(m_handle, "PixelFormat", &enumValue);
    if (ret == MV_OK) 
    {
        pixelFormat = enumValue.nCurValue;
        qDebug() << "[VisionCore] 像素格式:" << pixelFormat;
    }
    return ret;
}

// --- 触发模式 ---
int VisionCore::VisionCore_SetTriggerMode(int mode)
{
    if (m_handle == nullptr)
    {
        qDebug() << "[VisionCore] SetTriggerMode Please First Init";
        return 1;
    }
    return MV_CC_SetEnumValue(m_handle, "TriggerMode", mode);
}

int VisionCore::VisionCore_GetTriggerMode(int& mode)
{
    if (m_handle == nullptr)
    {
        qDebug() << "[VisionCore] GetTriggerMode Please First Init";
        return 1;
    }
    MVCC_ENUMVALUE enumValue = { 0 };
    int ret = MV_CC_GetEnumValue(m_handle, "TriggerMode", &enumValue);
    if (ret == MV_OK) 
    {
        mode = enumValue.nCurValue;
        qDebug() << "[VisionCore] 触发模式:" << mode;
    }
    return ret;
}

// --- 触发源 ---
int VisionCore::VisionCore_SetTriggerSource(int& source)
{
    if (m_handle == nullptr)
    {
        qDebug() << "[VisionCore] SetTriggerSource Please First Init";
        return 1;
    }
    return MV_CC_SetEnumValue(m_handle, "TriggerSource", source);
}

int VisionCore::VisionCore_GetTriggerSource(int& sourceValue)
{
    if (m_handle == nullptr)
    {
        qDebug() << "[VisionCore] GetTriggerSource Please First Init";
        return 1;
    }

    MVCC_ENUMVALUE enumValue = { 0 };
    int ret = MV_CC_GetEnumValue(m_handle, "TriggerSource", &enumValue);
    if (ret == MV_OK)
    {
        sourceValue = enumValue.nCurValue;
        qDebug() << "[VisionCore] 触发源枚举值:" << sourceValue;
    }
    return ret;
}

VisionCore::VisionCore()
{
    qDebug() << "[VisionCore] 开始初始化";

}

VisionCore::~VisionCore()
{
    m_isGrabbing = false;           // 仅做最基础的标志位清理
    if (m_frameData != nullptr) 
    {
        delete[] m_frameData;
        m_frameData = nullptr;
    }
}

