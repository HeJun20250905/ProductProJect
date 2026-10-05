#include "VisionDialog.h"
#include <QDebug>
#include <QTimer>
#include <QCoreApplication>
#include <QDir>
#include <QDateTime>
#include <QMouseEvent>

#include "../../../Include/VisionCore_Export.h"
#include "VisionFrame.h"
#include "VisionChildDialog.h"

#include "../../../Include/MotionCore_Export.h"

#ifdef _DEBUG
#define  LIB_PATH     "..\\..\\..\\Library\\Win32\\Debug"
#else
#define  LIB_PATH     "..\\..\\..\\Library\\Win32\\Release"
#endif

#pragma comment(lib,  LIB_PATH"\\VisionCore.lib")
#pragma comment(lib,  LIB_PATH"\\MotionCore.lib")

#include <opencv2/core.hpp>      // cv::Mat, cv::Point 等基础类型
#include <opencv2/imgproc.hpp>   // cvtColor, resize, cornerSubPix 等
#include <opencv2/imgcodecs.hpp> // imread, imwrite
#include <opencv2/calib3d.hpp>   // findChessboardCorners, calibrateCamera, findHomography

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <algorithm>

#if defined(_MSC_VER) && (_MSC_VER >= 1600)    
# pragma execution_character_set("utf-8")    
#endif

VisionDialog::VisionDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::VisionDialog)
{
    ui->setupUi(this);

    // 设置dialog标题风格
    this->setWindowFlags(
        Qt::Dialog |
        Qt::CustomizeWindowHint |
        Qt::WindowTitleHint |
        Qt::WindowCloseButtonHint);

    this->setWindowTitle("视觉模块");

    m_visionDialog_visionTimer = new QTimer(this);
    connect(m_visionDialog_visionTimer, &QTimer::timeout, this, &VisionDialog::visionDialog_visionTimer_timeout);
    m_visionDialog_visionTimer->start(33);

    // 添加事件过滤器
    ui->label_Video->installEventFilter(this);
}

VisionDialog::~VisionDialog()
{
    delete ui;
}

void VisionDialog::on_pushButton_Cancel_clicked()
{
    this->close();
}

bool VisionDialog::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == ui->label_Video && event->type() == QEvent::MouseButtonDblClick) 
    {
        QMouseEvent* handle = static_cast<QMouseEvent*>(event);

        handleDoubleClick(handle->pos());

        qDebug() << "双击控件";

        return true;
    }

    // 默认处理
    return QDialog::eventFilter(obj, event);
}

void VisionDialog::handleDoubleClick(const QPoint& pos)
{
    // 控件尺寸
    int labelW = ui->label_Video->width();
    int labelH = ui->label_Video->height();

    // 原始图像尺寸
    int imgW = 1280;
    int imgH = 1024;

    // 缩放比（取小值，保持宽高比）
    double scale = std::min((double)labelW / imgW, (double)labelH / imgH);

    // 缩放后图像尺寸
    int dispW = imgW * scale;
    int dispH = imgH * scale;

    // 居中偏移
    int offsetX = (labelW - dispW) / 2;
    int offsetY = (labelH - dispH) / 2;

    // 去掉偏移
    int x = pos.x() - offsetX;
    int y = pos.y() - offsetY;

    // ===== 弹窗 1：控件尺寸和缩放 =====
    {
        QString msg = QString("labelW=%1  labelH=%2\nscale=%3\noffsetX=%4  offsetY=%5\nx=%6  y=%7")
            .arg(labelW).arg(labelH)
            .arg(scale)
            .arg(offsetX).arg(offsetY)
            .arg(x).arg(y);
        MessageBoxW((HWND)this->winId(),
            reinterpret_cast<const wchar_t*>(msg.utf16()),
            L"1. 控件尺寸", MB_OK | MB_ICONINFORMATION);
    }

    // 判断是否在图像区域内
    if (x < 0 || x >= dispW || y < 0 || y >= dispH)
    {
        MessageBoxW((HWND)this->winId(),
            L"点在图像区域外",
            L"提示", MB_OK | MB_ICONWARNING);
        return;
    }

    // 控件坐标 -> 原始图像坐标
    double imgX = x / scale;
    double imgY = y / scale;

    // 原始图像中心
    double cx = imgW / 2.0;   // 640
    double cy = imgH / 2.0;   // 512

    // 像素偏移
    double dx_px = imgX - cx;
    double dy_px = imgY - cy;

    // ===== 弹窗 2：原始图像坐标和像素偏移 =====
    {
        QString msg = QString("imgX=%1  imgY=%2\ncx=%3  cy=%4\ndx_px=%5  dy_px=%6")
            .arg(imgX).arg(imgY)
            .arg(cx).arg(cy)
            .arg(dx_px).arg(dy_px);
        MessageBoxW((HWND)this->winId(),
            reinterpret_cast<const wchar_t*>(msg.utf16()),
            L"2. 图像坐标与像素偏移", MB_OK | MB_ICONINFORMATION);
    }

    // 换算成毫米
    double dx_mm = dx_px / m_mmPerPixel;
    double dy_mm = dy_px / m_mmPerPixel;

    // ===== 弹窗 3：毫米偏移和最终发送值 =====
    {
        QString msg = QString("m_mmPerPixel=%1\ndx_mm=%2  dy_mm=%3\n发送: dx=%4  dy=%5")
            .arg(m_mmPerPixel)
            .arg(dx_mm).arg(dy_mm)
            .arg(dx_mm).arg(-dy_mm);
        MessageBoxW((HWND)this->winId(),
            reinterpret_cast<const wchar_t*>(msg.utf16()),
            L"3. 毫米偏移", MB_OK | MB_ICONINFORMATION);
    }

    // 发给运控
    this->VisionDialogAxisMove(dy_mm, -dx_mm);
}

bool VisionDialog::VisionDialogAxisMove(double xPoint, double yPoint)
{
    // 轴使能，1使能, 0关闭使能
    Motion_Enable(0, 1);
    Motion_Enable(1, 1);
    Motion_Enable(2, 1);
    qDebug() << "[VisionDialog] 使能轴";

    // 组装轴号和目标位置数组
    int   axes[3] = { 0, 1, 2 };
    float targets[3] = { xPoint, yPoint, 0.0 };

    int ret = Motion_MoveRelMulti(3, axes, targets);
    if (ret != 0)
    {
        qDebug() << "[VisionDialog] 运动失败，错误码:" << ret;
        return false;
    }
    
    qDebug() << "[VisionDialog] 运动";
    return true;
}

void VisionDialog::on_pushButton_OK_clicked()
{
    this->accept();
}

void VisionDialog::on_pushButton_Start_clicked()
{
    if (!m_visionDialog_visionTimer->isActive())
    {
        m_visionDialog_visionTimer->start(33);
    }
}

void VisionDialog::on_pushButton_Stop_clicked()
{
    if (m_visionDialog_visionTimer->isActive())
    {
        m_visionDialog_visionTimer->stop();
    }
}

void VisionDialog::on_pushButton_WorkFlow_clicked()
{
    VisionChildDialog visionChildDialog;
    visionChildDialog.exec();
}

// 执行算法工作流
bool DoVisionWorkFlow(VisionWorkFlow step)
{
    return true;
}

void VisionDialog::on_pushButton_Computation_clicked()
{
    //// 读取工作流
    //QVector<VisionWorkFlow> workFlow = VisionFrame::instance().GetVisionWorkFlow();

    //// 非空判定
    //if (workFlow.isEmpty())
    //{
    //    qDebug() << "[MotionDialog] 工作流为空";
    //    return;
    //}

    //// 遍历工作流，依次执行
    //for (int index = 0; index < workFlow.size(); index++)
    //{
    //    DoVisionWorkFlow(workFlow[index]);
    //}

    //// 弹窗检查：打印工作流
    //QString msg = QString("即将应用 %1 个算法步骤：\n\n").arg(workFlow.size());
    //for (int i = 0; i < workFlow.size(); ++i) 
    //{
    //    msg += QString("步骤 %1: %2\n").arg(i + 1).arg(static_cast<int>(workFlow[i]));
    //}

    //// 使用 Win32 API 弹窗
    //::MessageBox(
    //    (HWND)this->winId(),                // 当前窗口的句柄（让弹窗居中显示）
    //    (LPCWSTR)msg.utf16(),               // 将 QString 转为宽字符指针
    //    (LPCWSTR)L"工作流检查",             // 弹窗标题
    //    MB_OK | MB_ICONINFORMATION          // 样式：OK按钮 + 信息图标
    //);

    //// 读取当前参数
    //VisionWorkPara para = VisionFrame::instance().GetVisionWorkPara();

    //// 拼接参数信息字符串
    //// QString msg;
    //msg += "--- 预处理参数 ---\n";
    //msg += QString("高斯核大小: %1 x %2\n").arg(para.m_gaussCoreX).arg(para.m_gaussCoreY);
    //msg += QString("高斯标准差: %1, %2\n").arg(para.m_gaussSigmaX).arg(para.m_gaussSigmaY);
    //msg += QString("二值化阈值: %1\n\n").arg(para.m_thresholdValue);

    //msg += "--- 轮廓筛选参数 ---\n";
    //msg += QString("Mark点最小面积: %1\n").arg(para.m_minMarkArea);
    //msg += QString("Mark点最大面积: %1\n\n").arg(para.m_maxMarkArea);

    //msg += "--- 绘制参数 ---\n";
    //msg += QString("十字线长度: %1 x %2\n").arg(para.m_lineLenX).arg(para.m_lineLenY);
    //msg += QString("线宽: %1 x %2\n").arg(para.m_lineWidX).arg(para.m_lineWidY);
    //msg += QString("线颜色(RGB): (%1, %2, %3)").arg(para.m_lineColorR).arg(para.m_lineColorG).arg(para.m_lineColorB);

    //// 使用 Win32 API 弹窗展示
    //::MessageBox(
    //    (HWND)this->winId(),                // 当前窗口的句柄（让弹窗居中显示）
    //    (LPCWSTR)msg.utf16(),               // 将 QString 转为宽字符指针
    //    (LPCWSTR)L"算法参数检查",           // 弹窗标题
    //    MB_OK | MB_ICONINFORMATION          // 样式：OK按钮 + 信息图标
    //);

    emit vision_computation_signal();
}

void VisionDialog::visionDialog_visionTimer_timeout()
{
    cv::Mat frame = VisionFrame::instance().GetVisionFrame();

    // 无锁显示
    if (!ui->label_Video || frame.empty())
    {
        qDebug() << "[MotionDialog] 显示图像异常";
        return;
    }

    // 用OpenCV进行等比例缩放
    double scale = (std::min)(
        (double)ui->label_Video->width() / frame.cols,
        (double)ui->label_Video->height() / frame.rows
        );
    cv::Size targetSize(frame.cols * scale, frame.rows * scale);

    cv::Mat resizedImg;
    cv::resize(frame, resizedImg, targetSize, 0, 0, cv::INTER_LINEAR);

    // 用缩放后的数据构造QImage
    QImage qImg(resizedImg.data,
        resizedImg.cols,
        resizedImg.rows,
        resizedImg.step,
        QImage::Format_Grayscale8);

    // 显示图片
    ui->label_Video->setPixmap(QPixmap::fromImage(qImg));
}

void VisionDialog::on_pushButton_Picture_clicked()
{
    // 拿数据
    cv::Mat frame = VisionFrame::instance().GetVisionFrame();
    if (frame.empty()) 
    {
        qWarning() << "GetVisionFrame 返回空";
        return;
    }

    // 确保 Image 目录存在
    QString exeDir = QCoreApplication::applicationDirPath();
    QString imageDir = exeDir + "/Image";
    QDir().mkpath(imageDir);

    // 生成文件名（带时间戳，避免覆盖）
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz");
    QString filePath = imageDir + "/frame_" + timestamp + ".png";

    // 保存
    bool ok = cv::imwrite(filePath.toStdString(), frame);
    if (ok) 
    {
        qDebug() << "已保存:" << filePath;
    }
    else 
    {
        qWarning() << "保存失败:" << filePath;
    }
}

void VisionDialog::on_pushButton_CalibBoard_clicked()
{
    const int INNER_COLS = 9;
    const int INNER_ROWS = 6;
    const double SQUARE_SIZE = 4.0;

    QString exeDir = QCoreApplication::applicationDirPath();
    QString testDir = exeDir + "/Test";
    QDir().mkpath(testDir);

    std::vector<cv::Point3f> objp;
    for (int r = 0; r < INNER_ROWS; ++r)
        for (int c = 0; c < INNER_COLS; ++c)
            objp.emplace_back(c * SQUARE_SIZE, r * SQUARE_SIZE, 0.0f);

    std::vector<std::vector<cv::Point3f>> objPoints;
    std::vector<std::vector<cv::Point2f>> imgPoints;
    cv::Size imageSize;

    QString imageDir = exeDir + "/Image";
    QDir dir(imageDir);
    QStringList files = dir.entryList(
        QStringList() << "*.png" << "*.jpg" << "*.bmp",
        QDir::Files, QDir::Name);

    if (files.isEmpty()) {
        qWarning() << "Image 文件夹里没有图片:" << imageDir;
        return;
    }

    int found = 0;
    for (const QString& f : files) {
        QString fullPath = dir.absoluteFilePath(f);
        cv::Mat gray = cv::imread(fullPath.toStdString(), cv::IMREAD_GRAYSCALE);
        if (gray.empty()) continue;

        imageSize = gray.size();

        std::vector<cv::Point2f> corners;
        bool ok = cv::findChessboardCorners(
            gray, cv::Size(INNER_COLS, INNER_ROWS), corners,
            cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_NORMALIZE_IMAGE
        );

        if (!ok) {
            qDebug() << "未找到角点:" << f;
            continue;
        }

        cv::cornerSubPix(
            gray, corners, cv::Size(11, 11), cv::Size(-1, -1),
            cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 30, 0.001)
        );

        // 保存角点可视化图
        cv::Mat vis;
        cv::cvtColor(gray, vis, cv::COLOR_GRAY2BGR);
        cv::drawChessboardCorners(vis, cv::Size(INNER_COLS, INNER_ROWS), corners, ok);
        cv::imwrite((testDir + "/corners_" + f).toStdString(), vis);

        // 记录棋盘格中心
        cv::Point2f ctr(0, 0);
        for (const auto& c : corners) ctr += c;
        ctr *= (1.0f / corners.size());

        qDebug() << "图片:" << f
            << "棋盘格中心:" << ctr.x << ctr.y
            << "图像中心:" << imageSize.width / 2 << imageSize.height / 2;

        objPoints.push_back(objp);
        imgPoints.push_back(corners);
        found++;
    }

    qDebug() << "有效图片:" << found << "/" << files.size();

    if (found < 10) {
        qWarning() << "有效标定图不足 10 张，标定失败";
        return;
    }

    cv::Mat cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
    cv::Mat distCoeffs = cv::Mat::zeros(5, 1, CV_64F);
    std::vector<cv::Mat> rvecs, tvecs;

    double err = cv::calibrateCamera(
        objPoints, imgPoints, imageSize,
        cameraMatrix, distCoeffs, rvecs, tvecs
    );

    qDebug() << "重投影误差:" << err;
    std::stringstream ss;
    ss << cameraMatrix;
    qDebug() << "内参矩阵:\n" << QString::fromStdString(ss.str());

    std::stringstream ss2;
    ss2 << distCoeffs;
    qDebug() << "畸变系数:\n" << QString::fromStdString(ss2.str());

    // 保存每张图的重投影误差
    for (size_t i = 0; i < objPoints.size(); ++i) {
        std::vector<cv::Point2f> projected;
        cv::projectPoints(objPoints[i], rvecs[i], tvecs[i],
            cameraMatrix, distCoeffs, projected);

        double e = cv::norm(imgPoints[i], projected, cv::NORM_L2)
            / imgPoints[i].size();
        qDebug() << "图片" << i << "重投影误差:" << e;
    }

    // 保存去畸变前后对比
    if (!files.isEmpty()) {
        cv::Mat sample = cv::imread(
            dir.absoluteFilePath(files[0]).toStdString(),
            cv::IMREAD_GRAYSCALE);
        if (!sample.empty()) {
            cv::Mat undistorted;
            cv::undistort(sample, undistorted, cameraMatrix, distCoeffs);
            cv::imwrite((testDir + "/undistort_before.png").toStdString(), sample);
            cv::imwrite((testDir + "/undistort_after.png").toStdString(), undistorted);
        }
    }

    QString outPath = exeDir + "/calib.yml";
    cv::FileStorage fs(outPath.toStdString(), cv::FileStorage::WRITE);
    fs << "camera_matrix" << cameraMatrix;
    fs << "dist_coeffs" << distCoeffs;
    fs << "reproj_error" << err;
    fs << "image_width" << imageSize.width;
    fs << "image_height" << imageSize.height;
    fs.release();

    qDebug() << "标定结果已保存:" << outPath;
    qDebug() << "调试文件已保存到:" << testDir;
}

void VisionDialog::on_pushButton_CalibScale_clicked()
{
    // 1. 拿原图
    cv::Mat frame = VisionFrame::instance().GetVisionFrame();
    if (frame.empty()) {
        qWarning() << "没有拿到图像";
        return;
    }

    // 2. 转灰度（原图已经是灰度，但保险起见）
    cv::Mat gray;
    if (frame.channels() == 3)
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    else
        gray = frame.clone();

    // 3. 二值化（正方形亮就用 BINARY，暗就用 BINARY_INV）
    cv::Mat binary;
    cv::threshold(gray, binary, 0, 255, cv::THRESH_BINARY_INV + cv::THRESH_OTSU); 

    // 4. 去噪
    cv::morphologyEx(binary, binary, cv::MORPH_OPEN,
        cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5)));

    // 5. 找轮廓
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // 6. 找最大的正方形
    double bestArea = 0;
    double pixelSide = 0;
    cv::Point2f center;
    for (const auto& c : contours) {
        double area = cv::contourArea(c);
        if (area < 500) continue;

        cv::RotatedRect rect = cv::minAreaRect(c);
        double w = rect.size.width;
        double h = rect.size.height;
        double ratio = (std::max)(w, h) / (std::min)(w, h);
        if (ratio < 0.9 || ratio > 1.1) continue;   // 不是正方形

        if (area > bestArea) {
            bestArea = area;
            pixelSide = (w + h) / 2.0;
            center = rect.center;
        }
    }

    if (pixelSide <= 0) {
        qWarning() << "未找到正方形";
        return;
    }

    // 7. 算比例
    double squareSizeMm = 10.0;   // 正方形实际边长
    double mmPerPixel = squareSizeMm / pixelSide;
    double pixelPerMm = pixelSide / squareSizeMm;

    qDebug() << "正方形像素边长:" << pixelSide;
    qDebug() << "mm/pixel:" << mmPerPixel;
    qDebug() << "pixel/mm:" << pixelPerMm;
    qDebug() << "正方形中心:" << center.x << center.y;

    m_mmPerPixel = pixelPerMm;
}

void VisionDialog::on_pushButton_DetectAngle_clicked()
{
    // 1. 拿原图
    cv::Mat frame = VisionFrame::instance().GetVisionFrame();
    if (frame.empty()) {
        qWarning() << "没有拿到图像";
        return;
    }

    // 2. 转灰度
    cv::Mat gray;
    if (frame.channels() == 3)
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    else
        gray = frame.clone();

    // 3. 二值化
    cv::Mat binary;
    cv::threshold(gray, binary, 0, 255,
        cv::THRESH_BINARY_INV + cv::THRESH_OTSU);

    // 4. 去噪
    cv::morphologyEx(binary, binary, cv::MORPH_OPEN,
        cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5)));

    // 5. 找轮廓
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // 6. 找最大的正方形
    double bestArea = 0;
    std::vector<cv::Point> bestContour;

    for (const auto& c : contours) {
        double area = cv::contourArea(c);
        if (area < 500) continue;

        cv::RotatedRect rect = cv::minAreaRect(c);
        double w = rect.size.width;
        double h = rect.size.height;
        double ratio = (std::max)(w, h) / (std::min)(w, h);
        if (ratio < 0.9 || ratio > 1.1) continue;

        if (area > bestArea) {
            bestArea = area;
            bestContour = c;
        }
    }

    if (bestContour.empty()) {
        qWarning() << "未找到正方形";
        return;
    }

    // 7. 多边形逼近，得到四个角点
    std::vector<cv::Point> approx;
    double peri = cv::arcLength(bestContour, true);
    cv::approxPolyDP(bestContour, approx, peri * 0.02, true);

    if (approx.size() != 4) {
        qWarning() << "不是四边形";
        return;
    }

    // 8. 找缺口边：对每条边，算轮廓偏离直线的最大距离
    int notchEdgeIdx = -1;
    double maxDeviation = 0;

    for (int i = 0; i < 4; ++i) {
        cv::Point2f p1 = approx[i];
        cv::Point2f p2 = approx[(i + 1) % 4];

        double maxDev = 0;
        for (const auto& pt : bestContour) {
            cv::Point2f v = cv::Point2f(pt) - p1;
            cv::Point2f dir = p2 - p1;
            double t = v.dot(dir) / dir.dot(dir);
            if (t < 0.2 || t > 0.8) continue;

            cv::Point2f proj = p1 + t * dir;
            double dist = cv::norm(cv::Point2f(pt) - proj);
            if (dist > maxDev) maxDev = dist;
        }

        if (maxDev > maxDeviation) {
            maxDeviation = maxDev;
            notchEdgeIdx = i;
        }
    }

    if (notchEdgeIdx < 0) {
        qWarning() << "未找到缺口";
        return;
    }

    // 9. 算角度
    cv::Point2f p1 = approx[notchEdgeIdx];
    cv::Point2f p2 = approx[(notchEdgeIdx + 1) % 4];
    cv::Point2f dir = p2 - p1;

    double angle = std::atan2(dir.y, dir.x) * 180.0 / CV_PI;

    // 归一化到 -90 ~ 90
    while (angle > 90) angle -= 180;
    while (angle < -90) angle += 180;

    qDebug() << "缺口边索引:" << notchEdgeIdx;
    qDebug() << "缺口边角度:" << angle << "度";

    m_notchAngle = angle;

    QString msg = QString("缺口边索引: %1\n缺口边角度: %2°")
        .arg(notchEdgeIdx).arg(angle);
    MessageBoxW((HWND)this->winId(),
        reinterpret_cast<const wchar_t*>(msg.utf16()),
        L"缺口角度", MB_OK | MB_ICONINFORMATION);
}

void VisionDialog::displayCalculateResult(const cv::Mat& resultImage)
{
    m_visionDialog_visionTimer->stop();

    // cv::Mat 转换为 QImage
    QImage qImg(resultImage.data,
        resultImage.cols,
        resultImage.rows,
        resultImage.step,
        QImage::Format_Grayscale8);

    qImg = qImg.copy();

    // QImage 阶段等比例缩放
    QImage scaledImage = qImg.scaled(ui->label_Video->size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation);

    // 将已经缩放好的 QImage 转换为 QPixmap
    QPixmap pixmap = QPixmap::fromImage(scaledImage);

    // 显示到界面上
    ui->label_Video->setPixmap(pixmap);
}

